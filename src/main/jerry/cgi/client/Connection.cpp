/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#include <jerry/cgi/client/Connection.h>
#if 0
#include <jerry/cgi/client/consumer/ResponseHandler.h>
#include <jerry/cgi/client/producer/RequestStatic.h>
#include <jerry/cgi/client/producer/RequestDynamic.h>
#include <jerry/cgi/client/producer/RequestFile.h>
#include <jerry/Logger.h>

#include <esl/http/client/NetworkException.h>
#include <esl/system/Process.h>
#include <esl/utility/Protocol.h>
#include <esl/utility/String.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace cgi {
namespace client {

namespace {
Logger logger("jerry::cgi::client::Connection");
}

std::unique_ptr<esl::http::client::Interface::Connection> Connection::create(const esl::utility::URL& hostUrl, const esl::object::Values<std::string>& values) {
	if(hostUrl.getScheme() != esl::utility::Protocol::protocolHttp && hostUrl.getScheme() != esl::utility::Protocol::protocolHttps) {
        throw esl::addStacktrace(std::runtime_error("jerry::cgi::client: Unknown scheme \"" + hostUrl.getScheme().toString() + "\" in URL."));
	}

	if(values.hasValue("executable") == false) {
        throw esl::addStacktrace(std::runtime_error("jerry::cgi::client: No executable specified."));
	}

	esl::system::process::Arguments arguments(values.getValue("executable"));
	if(arguments.getArgc() < 1) {
        throw esl::addStacktrace(std::runtime_error("jerry::cgi::client: Specified executable is empty."));
	}

	return std::unique_ptr<esl::http::client::Interface::Connection>(new Connection(hostUrl, std::move(arguments), values));
}

Connection::Connection(const esl::utility::URL& aHostUrl, esl::system::process::Arguments aArguments, const esl::object::Values<std::string>& values)
: hostUrl(aHostUrl),
  arguments(std::move(aArguments))
{
	path = esl::utility::String::toLower(hostUrl.getScheme().toString());
	path += "://" + hostUrl.getHostname();

	if(! hostUrl.getPort().empty()) {
		path += ":" + hostUrl.getPort() + "/";
	}

	std::string path = esl::utility::String::trim(hostUrl.getPath(), '/');
	if(! path.empty()) {
		path += path + "/";
	}

	if(values.hasValue("workingDir")) {
		workingDir = values.getValue("workingDir");
	}

	if(values.hasValue("SERVER_SOFTWARE")) {
		serverSoftware = values.getValue("SERVER_SOFTWARE");
	}

	if(values.hasValue("SERVER_ADMIN")) {
		serverAdmin = values.getValue("SERVER_ADMIN");
	}

	if(values.hasValue("SERVER_PROTOCOL")) {
		serverProtocol = values.getValue("SERVER_PROTOCOL");
	}

	if(values.hasValue("REMOTE_HOST")) {
		remoteHost = values.getValue("REMOTE_HOST");
	}

	if(values.hasValue("REMOTE_ADDR")) {
		remoteAddr = values.getValue("REMOTE_ADDR");
	}

	if(values.hasValue("REMOTE_USER")) {
		remoteUser = values.getValue("REMOTE_USER");
	}
	else if(values.hasValue("username")) {
		remoteUser = values.getValue("username");
	}

}

esl::http::client::Response Connection::send(const esl::http::client::Request& request) const {
	esl::system::process::ProducerDynamic producerDynamic([&request](char* buffer, std::size_t count) { return request.getRequestHandler()->producer(buffer, count); });
	//producer::RequestDynamic producerDynamic(request);
	return send(request, responseHandler, producerDynamic);
}

esl::http::client::Response Connection::send(const esl::http::client::RequestDynamic& request) const {
	esl::system::process::ProducerDynamic producerDynamic([&request](char* buffer, std::size_t count) { return request.getRequestHandler()->producer(buffer, count); });
	//producer::RequestDynamic producerDynamic(request);
	return send(request, responseHandler, producerDynamic);
}

esl::http::client::Response Connection::send(const esl::http::client::RequestStatic& request, esl::http::client::ResponseHandler* responseHandler) const {
	producer::RequestStatic producerRequest(request);
	return send(request, responseHandler, producerRequest.getRequestInfo(), producerRequest);
}

esl::http::client::Response Connection::send(const esl::http::client::RequestFile& request, esl::http::client::ResponseHandler* responseHandler) const {
	producer::RequestFile producerRequest(request);
	return send(request, responseHandler, producerRequest.getRequestInfo(), producerRequest);
}

esl::http::client::Response Connection::send(const esl::http::client::Request& request, esl::http::client::ResponseHandler* responseHandler, esl::system::Interface::Producer& processProducer) const {
	std::vector<std::pair<std::string, std::string>> environment;

	environment.push_back(std::make_pair("SERVER_SOFTWARE", serverSoftware));
	environment.push_back(std::make_pair("SERVER_NAME", hostUrl.getHostname()));
	environment.push_back(std::make_pair("SERVER_PROTOCOL", serverProtocol));
	environment.push_back(std::make_pair("SERVER_PORT", hostUrl.getPort()));
	if(hostUrl.getScheme() == esl::utility::Protocol::protocolHttps) {
		environment.push_back(std::make_pair("SERVER_PORT_SECURE", "1"));
		environment.push_back(std::make_pair("HTTPS", "on"));
	}
	else {
		environment.push_back(std::make_pair("SERVER_PORT_SECURE", "0"));
		environment.push_back(std::make_pair("HTTPS", "off"));
		if(hostUrl.getScheme() != esl::utility::Protocol::protocolHttp) {
			logger.warn << "Unknown scheme \"" << hostUrl.getScheme().toString() << "\" in URL.";
	        //throw esl::addStacktrace(std::runtime_error("cgi4esl: Unknown scheme \"" + hostUrl.getScheme().toString() + "\" in URL."));
		}
	}
	environment.push_back(std::make_pair("HTTP_ACCEPT", "*/*"));
	//environment.push_back(std::make_pair("HTTP_USER_AGENT", ""));
	environment.push_back(std::make_pair("GATEWAY_INTERFACE", "CGI/1.1"));

	environment.push_back(std::make_pair("REQUEST_METHOD", request.getMethod().toString()));
	environment.push_back(std::make_pair("REMOTE_HOST", remoteHost));
	environment.push_back(std::make_pair("REMOTE_ADDR", remoteAddr));
	environment.push_back(std::make_pair("REMOTE_USER", remoteUser));

	if(request.getRequestHandler() && request.getRequestHandler()->isEmpty() == false) {
		environment.push_back(std::make_pair("CONTENT_TYPE", request.getRequestHandler()->getContentType().toString()));
		if(request.getRequestHandler()->hasSize()) {
			environment.push_back(std::make_pair("CONTENT_LENGTH", std::to_string(request.getRequestHandler()->getSize())));
		}
	}

	environment.push_back(std::make_pair("SCRIPT_NAME", ""));
	environment.push_back(std::make_pair("PATH", ""));
	environment.push_back(std::make_pair("PATH_INFO", ""));
	environment.push_back(std::make_pair("PATH_TRANSLATED", ""));
	environment.push_back(std::make_pair("QUERY_STRING", ""));
	environment.push_back(std::make_pair("", ""));





	esl::system::Interface::Process::ParameterStreams parameterStreams;
	esl::system::Interface::Process::ParameterFeatures parameterFeatures;

	esl::http::client::RequestHandler* requestHandler = request.getRequestHandler();
	std::unique_ptr<esl::system::process::ProducerDynamic> processProducer2;
	if(requestHandler) {
		processProducer2.reset(new esl::system::process::ProducerDynamic([requestHandler](char* buffer, std::size_t count) { return requestHandler->producer(buffer, count); }));

		esl::system::Interface::Process::ParameterStream& parameterStream = parameterStreams[esl::system::Interface::FileDescriptor::stdInHandle];
		parameterStream.consumer = nullptr;
		parameterStream.producer = processProducer2;
	}

	consumer::ResponseHandler processConsumer(responseHandler);
	{
		esl::system::Interface::Process::ParameterStream& parameterStream = parameterStreams[esl::system::Interface::FileDescriptor::stdOutHandle];
		parameterStream.consumer = &processConsumer;
		parameterStream.producer = nullptr;
	}

	esl::system::Process process(arguments, esl::system::process::Environment(std::move(environment)), workingDir);
	process.execute(parameterStreams, parameterFeatures);
	//process.execute(processProducer, esl::system::Interface::FileDescriptor::stdInHandle, processConsumer, esl::system::Interface::FileDescriptor::stdOutHandle);

	return esl::http::client::Response(processConsumer.getStatusCode(), std::move(processConsumer.getHeaders()));
}

} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
#endif
