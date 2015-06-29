#include "zk_node_shard.h"

#include "zk_node_endpoint.h"
#include "base/logging.h"

namespace melon {
namespace client {

void ZkNodeShard::HandleNodeEvent(const char * path, const std::string & value) {
  std::string spath(path);
  size_t pos = spath.find_last_of('/');
  std::string addr = spath.substr(pos + 1);

  ZkNodeEndpointPtr ep(new ZkNodeEndpoint(path, zk_client_));
  ep->Load();

  LOG_INFO("ZkNodeShard " << path_ << " HandleNodeEvent : " << addr 
    << " weight=" << ep->weight());

  // 为减少lock时间，增加两个辅助的vector
  std::vector<ZkNodeEndpointPtr> active_eps, inactive_eps;
  {
    boost::shared_lock<boost::shared_mutex> rlock(endpoints_mutex_);
    active_eps = active_endpoints_;
    inactive_eps = inactive_endpoints_;
  }

  for(size_t i = 0; i < active_eps.size(); ++i) {
    if (active_eps[i]->addr() == addr) {
      active_eps.erase(active_eps.begin() + i);
      break;
    }
  }

  for(size_t i = 0; i < inactive_eps.size(); ++i) {
    if (inactive_eps[i]->addr() == addr) {
      inactive_eps.erase(inactive_eps.begin() + i);
      break;
    }
  }

  if (ep->weight() > 0) {
    active_eps.push_back(ep);
  } else {
    inactive_eps.push_back(ep);
  }

  if (active_eps.empty()) {
    // 没有可用的节点时，启用不可用节点
    boost::unique_lock<boost::shared_mutex> wlock(endpoints_mutex_);
    active_endpoints_.swap(inactive_eps);
    inactive_endpoints_.clear();
  } else {
    boost::unique_lock<boost::shared_mutex> wlock(endpoints_mutex_);
    active_endpoints_.swap(active_eps);
    inactive_endpoints_.swap(inactive_eps);
  }

  Dump();
}

void ZkNodeShard::HandleChildEvent(const char * path, const std::vector<std::string> & children) {
  LOG_INFO("ZkNodeShard " << path_ << " HandleChildEvent " << path);
  LoadEndpoints();
  Dump();
}

void ZkNodeShard::Dump() {
  boost::shared_lock<boost::shared_mutex> rlock(endpoints_mutex_);
  LOG_INFO("ZkNodeShard " << path_ << " active_endpoints_:");
  for(size_t i = 0; i < active_endpoints_.size(); ++i) {
    LOG_INFO("\t" << active_endpoints_[i]->addr());
  }

  LOG_INFO("ZkNodeShard " << path_ << " inactive_endpoints_:");
  for(size_t i = 0; i < inactive_endpoints_.size(); ++i) {
    LOG_INFO("\t" << inactive_endpoints_[i]->addr());
  }
}

static int GetShardId(const char * shard_path, size_t maxlen) {
  size_t len = maxlen;
  while(*(shard_path + len - 1) != '_') {
    --len;
  }

  size_t index = 0;
  for(size_t i = len; i < maxlen; i++) {
    index = index * 10 + *(shard_path + i) - '0';
  }
  return index % 10000;
}

ZkNodeShard::ZkNodeShard(const std::string & path, base::ZookeeperClient * zk_client) : path_(path), zk_client_(zk_client) {
  srand(time(NULL));
  id_ = GetShardId(path_.c_str(), path_.size());
}

std::string ZkNodeShard::SelectEndpoint() {
//if (active_endpoints_.empty()) {
//  LOG_WARN("ZkNodeShard " << path_ << " SelectEndpoint err, no active endpoint");
//  return std::string();

//int idx = rand() % active_endpoints_.size();
//LOG_INFO("ZkNodeShard " << path_ << " selected_idx=" << idx << "/" << active_endpoints_.size()
//         << " endpoint=" << active_endpoints_.at(idx)->addr());
//return active_endpoints_.at(idx)->addr();
//}
  static int err_counter = 0;
  static time_t last_retry_time = 0;
  int32_t total_weight = 0;
  std::map<int32_t, ZkNodeEndpointPtr> weighted_endpoints;

  {
    boost::shared_lock<boost::shared_mutex> rlock(endpoints_mutex_);
    for(size_t i = 0; i < active_endpoints_.size(); ++i) {
      int32_t current_weight = active_endpoints_[i]->weight();
      if (current_weight > 0) {
        total_weight += current_weight;
        weighted_endpoints.insert(std::make_pair(total_weight, active_endpoints_[i]));
      }
    }
  }

  if (weighted_endpoints.empty()) {
    // 超过2000次，或20秒，则进行reload
    if (err_counter > 2000 || (time(NULL) - last_retry_time > 20)) {
      err_counter = 0;
    }

    if (++err_counter <= 2) {
      last_retry_time = time(NULL); 
      LoadEndpoints();
    }

    LOG_WARN("ZkNodeShard " << path_ << " SelectEndpoint err, no active endpoint, err_counter=" << err_counter);
  
    return std::string();
  } else {
    err_counter = 0;
  }

  ZkNodeEndpointPtr endpoint = weighted_endpoints.upper_bound(rand() % total_weight)->second;
  LOG_DEBUG("ZkNodeShard " << path_ << " selected_endpoint=" << endpoint->addr());
  return endpoint->addr();
}

bool ZkNodeShard::LoadEndpoints() {
  LOG_INFO("ZkNodeShard " << path_ << " start load endpoint.");
  std::vector<std::string> endpoints; 
  if (zk_client_->GetChildren(path_.c_str(), &endpoints) != 0) {
    LOG_WARN("ZkNodeShard no endpoint " << path_);
    return false;
  }
  LOG_INFO("ZkNodeShard " << path_ << " endpoint size=" << endpoints.size());
  zk_client_->AddChildListener(path_.c_str(), shared_from_this());
  
  std::vector<ZkNodeEndpointPtr> active_endpoints, inactive_endpoints;

  for(size_t i = 0; i < endpoints.size(); ++i) {
    LOG_INFO("ZkNodeShard " << path_ << " load endpoint : " << endpoints[i]);
    // ip:port 节点的值有变化时，通知shard
    zk_client_->AddNodeListener((path_ + '/' + endpoints[i]).c_str(), shared_from_this());
    
    ZkNodeEndpointPtr ep(new ZkNodeEndpoint(path_ + '/' + endpoints[i], zk_client_)); 
    ep->Load();
    if (ep->weight() > 0) {
      active_endpoints.push_back(ep);
    } else {
      inactive_endpoints.push_back(ep);
    }
  }

  if (active_endpoints.empty()) {
    // 没有可用的节点时，启用不可用节点
    boost::unique_lock<boost::shared_mutex> wlock(endpoints_mutex_);
    active_endpoints_.swap(inactive_endpoints);
    inactive_endpoints_.clear();
  } else {
    boost::unique_lock<boost::shared_mutex> wlock(endpoints_mutex_);
    active_endpoints_.swap(active_endpoints);
    inactive_endpoints_.swap(inactive_endpoints);
  }
  return true;
}

}
}
