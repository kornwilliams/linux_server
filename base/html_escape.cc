#include <sstream>
#include <iomanip>
#include "html_escape.h"

namespace base {

std::string HtmlEscape(const std::string & unescaped) {
  std::string dest;

  for (size_t i = 0; i < unescaped.size(); ++i)  { 
    switch(unescaped[i]) {
      case '&':  dest += "&amp;";break;
      case '"':  dest += "&quot;"; break;
      case '\'': dest += "&#39;";  break;
      case '<':  dest += "&lt;";   break;
      case '>':  dest += "&gt;";   break;
      case '\r': case '\n': case '\v': case '\f': case '\t':
        dest += " ";
        break;
      default:
        dest += unescaped[i];
    }
  }
  return dest;
}

}

