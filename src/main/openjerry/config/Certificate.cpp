/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2022 Sven Lukas
 *
 * Jerry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Jerry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Jerry.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <openjerry/config/Certificate.h>
#include <openjerry/config/FilePosition.h>

#include <esl/crypto/KeyStore.h>
#include <esl/plugin/Registry.h>

#include <fstream>
#include <stdexcept>
#include <vector>

namespace openjerry {
namespace config {

Certificate::Certificate(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	std::string keyFile;
	std::string certFile;
	std::string domain;

	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "domain") {
			domain = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "key") {
			keyFile = attribute->Value();
			if(keyFile == "") {
				throw FilePosition::add(*this, "Value \"\" of attribute 'key' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "cert") {
			certFile = attribute->Value();
			if(certFile == "") {
				throw FilePosition::add(*this, "Value \"\" of attribute 'cert' is invalid.");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(keyFile == "") {
		throw FilePosition::add(*this, "Missing attribute 'key'");
	}
	if(certFile == "") {
		throw FilePosition::add(*this, "Missing attribute 'cert'");
	}




	std::vector<unsigned char> key;
	std::vector<unsigned char> certificate;

	if(!keyFile.empty()) {
		std::ifstream ifStream(keyFile, std::ios::binary );
		if(!ifStream.good()) {
			throw FilePosition::add(*this, "Cannot open key file \"" + keyFile + "\"");
		}
	    key = std::vector<unsigned char>(std::istreambuf_iterator<char>(ifStream), {});
	}

	if(!certFile.empty()) {
		std::ifstream ifStream(certFile, std::ios::binary );
		if(!ifStream.good()) {
			throw FilePosition::add(*this, "Cannot open certificate file \"" + certFile + "\"");
		}
		certificate = std::vector<unsigned char>(std::istreambuf_iterator<char>(ifStream), {});
	}



	esl::crypto::KeyStore* keyStore = esl::plugin::Registry::get().findObject<esl::crypto::KeyStore>();
	if(!keyStore) {
		throw FilePosition::add(*this, "Cannot add key and certificate, because there is no crypto engine installed.");
	}

	keyStore->addCertificate(domain, certificate);
	keyStore->addPrivateKey(domain, key, "");
}

void Certificate::save(std::ostream& oStream, std::size_t spaces) const {
}

} /* namespace config */
} /* namespace openjerry */
