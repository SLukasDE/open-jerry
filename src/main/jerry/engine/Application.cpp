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

#include <jerry/engine/Application.h>

namespace jerry {
namespace engine {

Application::Application(const std::string& aName, ObjectContext& parentObjectContext)
: name(aName)
{
	setParent(&parentObjectContext);
}

const std::string& Application::getName() const noexcept {
	return name;
}

basic::Context& Application::addBasicListener() {
	if(basicListener) {
		throw std::runtime_error("Cannot add multiple basic-listeners");
	}
	basicListener.reset(new engine::basic::Context);
	return *basicListener;
}

basic::Context* Application::getBasicListener() {
	return basicListener.get();
}

http::Context& Application::addHttpListener() {
	if(httpListener) {
		throw std::runtime_error("Cannot add multiple http-listeners");
	}
	httpListener.reset(new engine::http::Context);
	return *httpListener;
}

http::Context* Application::getHttpListener() {
	return httpListener.get();
}

void Application::initializeContext() {
	ObjectContext::initializeContext();

	if(basicListener) {
		basicListener->initializeContext();
	}

	if(httpListener) {
		httpListener->initializeContext();
	}
}

} /* namespace engine */
} /* namespace jerry */
