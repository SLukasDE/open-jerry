/*
 * Client.h
 *
 *  Created on: 17.12.2021
 *      Author: lukas
 */

#ifndef JERRY_CONFIG_HTTP_CLIENT_H_
#define JERRY_CONFIG_HTTP_CLIENT_H_

namespace jerry {
namespace config {
namespace http {

class Client {
public:
	Client();
	virtual ~Client();
};

} /* namespace http */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_HTTP_CLIENT_H_ */
