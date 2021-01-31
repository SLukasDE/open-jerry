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

#include <jerry/script/AST.h>

namespace jerry {
namespace script {

AST::AST(std::string script)
: type(list)
{
	std::string::const_iterator begin = script.cbegin();
	std::string::const_iterator end = script.cend();
	parse(begin, end);
}

AST::AST(Type aType, std::string aText)
: type(aType),
  value(std::move(aText))
{ }

AST::Type AST::getType() const noexcept {
	return type;
}

const std::string& AST::getValue() const noexcept {
	return value;
}

std::vector<std::reference_wrapper<AST>> AST::getChildren() const {
	std::vector<std::reference_wrapper<AST>> rv;

	for(auto& ast : children) {
		rv.push_back(std::ref(*ast));
	}

	return rv;
}

AST& AST::addList() {
	AST* element = new AST;
	children.push_back(std::unique_ptr<AST>(element));
	return *element;
}

AST& AST::addText(std::string textValue) {
	AST* element = new AST(AST::text, std::move(textValue));
	children.push_back(std::unique_ptr<AST>(element));
	return *element;
}

AST& AST::addFunction() {
	AST* element = new AST(Type::function, "");
	children.push_back(std::unique_ptr<AST>(element));
	return *element;
}

void AST::parse(std::string::const_iterator& iterator, const std::string::const_iterator& end) {
	AST* listElement = nullptr;
	std::string textString;

	for(; iterator != end; ++iterator) {
		/* make some special checks if parent-AST-type is function */
		if(getType() == function) {

			/* skip spaces */
			if(*iterator == ' ') {
				/* store previous text if it is not empty */
				if(!textString.empty()) {
					if(listElement) {
						listElement->addText(std::move(textString));
					}
					else {
						addText(std::move(textString));
					}
					textString.clear();
				}

				/* reset listElement to add next characters as new argument for function parent */
				listElement = nullptr;

				continue;
			}

			/* stop if '}' occurred */
			if(*iterator == '}') {
				/* store previous text if it is not empty */
				if(!textString.empty()) {
					if(listElement) {
						listElement->addText(std::move(textString));
					}
					else {
						addText(std::move(textString));
					}
					textString.clear();
				}

				/* we still persist on '}' character because       *
				 * caller will increment the iterator if we return */
				return;
			}
		}

		if(*iterator == '\\') {
			++iterator;

			if(iterator == end) {
				textString += '\\';
				break;
			}

			textString += *iterator;
		}

		/* special check if character is '$' */
		else if(*iterator == '$') {
			auto previousIterator = iterator;
			++iterator;

			if(iterator == end) {
				textString += '$';
				break;
			}

			/* check if it is a function */
			if(*iterator == '{') {
				/* it is a function */
				++iterator;

				if(getType() == function && listElement == nullptr) {
					listElement = &addList();
				}

				/* store previous text if it is not empty */
				if(!textString.empty()) {
					if(listElement) {
						listElement->addText(std::move(textString));
					}
					else {
						addText(std::move(textString));
					}
					textString.clear();
				}

				if(getType() == function) {
					listElement->addFunction().parse(iterator, end);
				}
				else {
					addFunction().parse(iterator, end);
				}

				/* iterator is now on functions closing '}' character  *
				 * and will be incremented at continuing the for-loop. */
			}
			else {
				/* it is not a function */

				textString += '$';
				iterator = previousIterator;
			}
		}
		else {
			/* push everything else into textString */
			textString += *iterator;
		}
	}

	if(!textString.empty()) {
		if(listElement) {
			listElement->addText(std::move(textString));
		}
		else {
			addText(std::move(textString));
		}
	}
}

} /* namespace script */
} /* namespace jerry */
