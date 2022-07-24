#include <jerry/builtin/object/applications/xmlconfig/Context.h>
#include <jerry/builtin/object/applications/xmlconfig/EntryImpl.h>
#include <jerry/config/Config.h>
#include <jerry/config/FilePosition.h>
#include <jerry/Logger.h>

//#include <esl/Module.h>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {
namespace xmlconfig {

namespace {
Logger logger("jerry::builtin::object::applications::xmlconfig::Context");

std::string getConfigFile(const boost::filesystem::path& path) {
	boost::filesystem::path configFile = path / "jerry.xml";
	return configFile.generic_string();
}
} /* anonymous namespace */

Context::Context(const boost::filesystem::path& aPath)
: config::Config(getConfigFile(aPath)),
  appName(aPath.filename().generic_string()),
  path(aPath)
{
	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(getFileName().c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw jerry::config::FilePosition::add(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw jerry::config::FilePosition::add(*this, "No root element");
	}

	setXMLFile(getFileName(), *element);
	loadXML(*element);
}

void Context::save(std::ostream& oStream) const {
	oStream << "\n<jerry-app>\n";

	for(const auto& entry : entries) {
		entry->save(oStream, 2);
	}

	oStream << "</jerry-app>\n";
}

void Context::install(Application& application) {
	loadLibraries();

	for(const auto& entry : entries) {
		entry->install(application);
	}
}

void Context::loadXML(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw jerry::config::FilePosition::add(*this, "Name of XML root element is empty");
	}
	if(std::string(element.Name()) != "jerry-app") {
		throw jerry::config::FilePosition::add(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"jerry-app\"");
	}
	if(element.GetUserData() != nullptr) {
		throw jerry::config::FilePosition::add(*this, "Node has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw jerry::config::FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		auto oldXmlFile = setXMLFile(getFileName(), *innerElement);
		parseInnerElement(*innerElement);
		setXMLFile(oldXmlFile);
	}
}

void Context::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw jerry::config::FilePosition::add(*this, "Element name is empty");
	}

	const std::string elementName(element.Name());
	if(elementName == "library") {
		parseLibrary(element);
	}
	else {
		entries.emplace_back(new EntryImpl(getFileName(), element));
	}
}

void Context::parseLibrary(const tinyxml2::XMLElement& element) {
	std::string fileName;

	if(element.GetUserData() != nullptr) {
		throw jerry::config::FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			fileName = attribute->Value();
			if(fileName == "") {
				throw jerry::config::FilePosition::add(*this, "Value \"\" of attribute 'file' is invalid.");
			}
		}
		else {
			throw jerry::config::FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(fileName == "") {
		throw jerry::config::FilePosition::add(*this, "Missing attribute 'file'");
	}

	boost::filesystem::path libraryFileInside = path / fileName;
	boost::filesystem::path libraryFileOutside = fileName;

	if(boost::filesystem::is_regular_file(libraryFileInside)) {
		libraries.push_back(std::make_pair(libraryFileInside.generic_string(), nullptr));
	}
	else if(boost::filesystem::is_regular_file(libraryFileOutside)) {
		libraries.push_back(std::make_pair(libraryFileOutside.generic_string(), nullptr));
	}
	else {
		throw jerry::config::FilePosition::add(*this, "Cannot find library-file '" + fileName + "'");
	}
}

void Context::loadLibraries() {
	/* ************************
	 * load and add libraries *
	 * ********************** */
	for(auto& library : libraries) {
		/*
		if(library.second) {
			throw esl::stacktrace::Stacktrace::add(std::runtime_error(std::string("Library \"") + library.first + "\" loaded already."));
		}
		*/
		library.second = &esl::plugin::Library::load(library.first);
		library.second->install(esl::plugin::Registry::get(), "");
	}
}

} /* namespace xmlconfig */
} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
