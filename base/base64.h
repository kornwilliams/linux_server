#ifndef _UTIL_BASE64_H_
#define _UTIL_BASE64_H_

#include <string>

namespace base{

class Base64 {

public:
	/**
	 * Base64-encodes the input according to RFC 3548.
	 * @param input The data to encode.
	 * @return The encoded string.
	 */
	static const std::string encode64(const std::string& input);

	/**
	 * Base64-decodes the input according to RFC 3548.
	 * @param input The encoded data.
	 * @return The decoded data.
	 */
	static const std::string decode64(const std::string& input);

private:
	static const std::string alphabet64;
	static const std::string::size_type table64[];
	static const char pad;
	static const std::string::size_type np;
};

}
#endif // _UTIL_BASE64_H_
