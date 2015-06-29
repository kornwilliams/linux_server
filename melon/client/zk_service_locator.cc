#include "zk_service_locator.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "zk_node_service.h"
#include "base/logging.h"

namespace melon {
namespace client {

struct SerivceLoc {
  SerivceLoc() : shard(0) {}
  std::string service_id; // /im/session_service
  std::string version;    // 1.0, 2.1
  int shard;              // 0,1,2,3
  std::string endpoint;   // 127.0.0.1:9090
};

int ParseServiceKey(const std::string & service_key, SerivceLoc * loc) {
  if (service_key.empty()) {
    LOG_WARN("empty service_key");
    return -1;
  }

  if (service_key[0] != '/') {
    LOG_WARN("service_key bad service_id " << service_key);
    return -2;
  }
  std::vector<std::string> strs;
  boost::split(strs, service_key, boost::is_any_of("/"), boost::token_compress_on);
  if (strs.size() < 3 || strs.size() > 6) {
    LOG_WARN("service_key bad param " << service_key);
    return -5;
  }

  loc->service_id = "/" + strs[1] + "/" + strs[2];
  if (strs.size() > 3) {
    static boost::regex ver_pattern("v\\d{1,9}\\.\\d{1,3}");
    if (!boost::regex_match(strs[3], ver_pattern)) {
      LOG_WARN("bad version " << service_key);
      return -8;
    }
    // TODO: regex match
    loc->version = strs[3].substr(1);
  }

  if (strs.size() > 4) {
    static const std::string shard_prefix("shard_");
    if (!boost::algorithm::starts_with(strs[4], shard_prefix)) {
      LOG_WARN("bad shard prefix " << service_key);
      return -6;
    }
    try {
      loc->shard = boost::lexical_cast<int>(strs[4].substr(shard_prefix.size()));
    } catch (boost::bad_lexical_cast &) {
      LOG_WARN("bad shard id " << service_key);
      return -4;
    }
  }

  if (strs.size() > 5) {
    static boost::regex ep_pattern("\\d{1,3}(\\.\\d{1,3}){3}:\\d{2,5}");

    if (!boost::regex_match(strs[5], ep_pattern)) {
      LOG_WARN("bad endpoint " << service_key);
      return -7;
    }
    loc->endpoint = strs[5];
  }

//LOG_DEBUG("parse service_key service_id=" << loc->service_id << " ver=" << loc->version
//          << " shard=" << loc->shard << " endpoint=" << loc->endpoint);
  return 0;
}

std::string ZkServiceLocator::Locate(const char * service_key) {
  SerivceLoc loc;
  if (ParseServiceKey(service_key, &loc) != 0) {
    LOG_WARN("bad zookeeper service path " << service_key);
    return std::string();
  }

  if (!loc.endpoint.empty()) {
    LOG_DEBUG("zk hard coded path " << service_key << " endpoint=" << loc.endpoint);
    return loc.endpoint;
  }

  ZkNodeServicePtr service;
  {
    boost::shared_lock<boost::shared_mutex> rlock(service_mutex_);
    ServiceMap::iterator it = service_map_.find(loc.service_id);
    if (it != service_map_.end()) {
      service = it->second;
    }
  }
  if (!service) {
    service.reset(new ZkNodeService(loc.service_id, &zk_client_));
    if (!service->Load()) {
      LOG_ERROR("service " << service->id() << " load error.");
      return std::string();
    }
    boost::unique_lock<boost::shared_mutex> wlock(service_mutex_);
    service_map_.insert(std::make_pair(loc.service_id, service));
  }

  return service->LocateService(loc.version, loc.shard);
}


}
}

