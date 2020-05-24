#include "base64.h"

std::string base64(const std::string& str) {
	std::string base64Chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
	std::string result;
	
	for (unsigned int i = 0; i < str.size(); i += 3) {
		int packed64;
		int num64Chars;

		packed64 = (str.at(i) & 255) << 16;
		num64Chars = 2;

		if (i + 1 < str.size()) {
			packed64 = packed64 + ((str.at(i+1) & 255) << 8);
			num64Chars = 3;
		}

		if (i + 2 < str.size()) {
			packed64 = packed64 + (str.at(i+2) & 255);
			num64Chars = 4;
		}

		for (int j = 0; j < 4; ++j) {
			if(j<num64Chars) {
				result += base64Chars[ (packed64 >> (6 * (3 - j))) & 63];
			}
			else {
				result += "=";
			}
		}
	}
	return result;
}
