#ifndef _BASE_PROPERTY_H_
#define _BASE_PROPERTY_H_

#include <string>
#include <map>

#include <boost/lexical_cast.hpp>

#include "base/logging.h"

namespace base {
class Property {
 public:
  //解析 config.property 格式的数据
  int ParseString(const std::string & prop_str);
  int ParseFile(const std::string & file_name);

  template <class T>
  T Get(const std::string & key, const T & default_prop) const {
    std::map<std::string, std::string>::const_iterator it = prop_map_.find(key);
    
    if (it != prop_map_.end()) {
      try {
        T v = boost::lexical_cast<T>(it->second);
        LOG_INFO("Property::Get " << key << " value=" << v);
        return v;
      } catch (...) {
        LOG_WARN("Property::Get " << key << " bad value=" << it->second);
      }
    } else {
      LOG_INFO("Property::Get " << key << " not found, default=" << default_prop);
    }
    return default_prop;
  }

 private:
  std::map<std::string, std::string> prop_map_;


};


}

#endif // _BASE_PROPERTY_H_
