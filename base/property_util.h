#ifndef _BASE_PROPERTY_UTIL_H_
#define _BASE_PROPERTY_UTIL_H_

#include <string>
#include <map>

#include <boost/lexical_cast.hpp>

#include "base/logging.h"

namespace base {
  
//解析.propert格式的数据
void ParseProperty(const std::string & prop_str, std::map<std::string, std::string> * res);

template <class T>
T GetProperty(const std::map<std::string, std::string> & props, const std::string & key, const T & default_prop) {
  std::map<std::string, std::string>::const_iterator it = props.find(key);
  
  if (it != props.end()) {
    try {
      T v = boost::lexical_cast<T>(it->second);
      LOG_INFO("Zk endpoint prop " << key << " value=" << v);
      return v;
    } catch (...) {
      LOG_WARN("Zk endpoint prop " << key << " bad value=" << it->second);
    }
  } else {
    LOG_INFO("Zk endpoint prop " << key << " not found, default=" << default_prop);
  }
  return default_prop;
}


}

#endif // _BASE_PROPERTY_UTIL_H_
