#ifndef OPENJERRY_PLUGIN_H_
#define OPENJERRY_PLUGIN_H_

#include <esl/plugin/Registry.h>

namespace openjerry {

class Plugin final {
public:
	Plugin() = delete;
	static void install(esl::plugin::Registry& registry, const char* data);
};

} /* namespace openjerry */

#endif /* OPENJERRY_PLUGIN_H_ */
