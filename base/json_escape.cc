#include <sstream>
#include <iomanip>
#include "json_escape.h"

namespace base {

std::string JsonEscape(const std::string & src) {
  unsigned char c;
  std::string dest;

  for (size_t i = 0; i < src.size(); ++i)  { 
    c = src[i]; 
    if ((c == '\\') || (c == '"')) { 
      dest += '\\'; 
      dest += c; 
    } else if (c == '\b') {
      dest += "\\b"; 
    } else if (c == '\t') {
      dest += "\\t"; 
    } else if (c == '\n') {
      dest += "\\n"; 
    } else if (c == '\f') {
      dest += "\\f"; 
    } else if (c == '\r') {
      dest += "\\r"; 
    } else { 
      if (c < ' ') { 
        std::ostringstream oss;
        oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(c);
        dest += oss.str();
      } else { 
        dest += c; 
      } 
    }
  } 
  return dest;
}

}

