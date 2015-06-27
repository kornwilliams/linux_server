#include "property.h"

#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace base {
  
int Property::ParseString(const std::string & prop_str) {
  std::vector<std::string> lines;
  boost::split(lines, prop_str, boost::is_any_of(";\r\n"), boost::token_compress_on);
  for(size_t i = 0; i < lines.size(); ++i) {
    size_t pos = lines[i].find_first_of('=');
    std::string key = lines[i].substr(0, pos);
    boost::trim(key);
    std::string value = lines[i].substr(pos + 1);
    boost::trim(value);
    if (pos != std::string::npos) {
      prop_map_.insert(std::make_pair(key, value));
    }
  }

  return 0;
}

//int ParseFile(const std::string & file_name);

}

