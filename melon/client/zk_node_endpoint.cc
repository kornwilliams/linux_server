#include "zk_node_endpoint.h"

#include "base/property_util.h"
#include "base/logging.h"

namespace melon {
namespace client {

ZkNodeEndpoint::ZkNodeEndpoint(const std::string & path, base::ZookeeperClient * zk_client) 
    : path_(path)
    , weight_(1000)
    , zk_client_(zk_client) {
  size_t pos = path.find_last_of('/');
  addr_ = path.substr(pos + 1);
  pos = addr_.find(':');
  host_ = addr_.substr(0, pos);
  try {
    port_ = boost::lexical_cast<int>(addr_.substr(pos + 1));
  } catch (...) {
    LOG_WARN("ZkNodeEndpoint " << path_ << " bad path"); 
  }
}

ZkNodeEndpoint::~ZkNodeEndpoint() {
}

/*
template <class T>
T GetProperty(const std::string & key, const std::map<std::string, std::string> & props, const T & default_prop) {
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
*/

bool ZkNodeEndpoint::Load() {
  std::string data;
  if (zk_client_->GetValue(path_.c_str(), &data) != 0) {
    LOG_WARN("ZkNodeEndpoint " << path_ << " load err"); 
    // 加载失败时，保持现有状态不变
    return false;
  }

  std::map<std::string, std::string> props;
  base::ParseProperty(data, &props);
  weight_ = base::GetProperty<int32_t>(props, std::string("weight"), 1000);

  LOG_INFO("ZkNodeEndpoint " << path_ << " data='" << data << "' weight=" << weight_);
  // zk_client_->AddNodeListener(path_.c_str(), shared_from_this());
  return true;
}

}
}

