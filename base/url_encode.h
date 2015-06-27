#ifndef _UTIL_URL_ENCODE_H_
#define _UTIL_URL_ENCODE_H_

#include <string>

namespace base {

std::string UrlEncode(const std::string & src);
std::string UrlDecode(const std::string & src);

}

#endif // _UTIL_URL_ENCODE_H_

