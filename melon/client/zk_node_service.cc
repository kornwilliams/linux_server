#include "zk_node_service.h"

#include <iostream>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include "base/logging.h"

namespace melon {
namespace client {

ZkNodeService::ZkNodeService(const std::string & id, base::ZookeeperClient * zk_client) : id_(id), zk_client_(zk_client) {
  path_ = GetPathById(id);
}

std::string ZkNodeService::LocateService(const std::string & version, int shard) const {
  if (!version.empty()) {
    std::map<std::string, ZkNodeVersionPtr>::const_iterator it = all_versions_.find("v" + version);
    if (it == all_versions_.end()) {
      LOG_WARN("ZkNodeService " << path_ << " version  " << version << " not found");
      return std::string();
    }
    LOG_INFO("ZkNodeService " << path_ << " version  " << version << " found");
    return it->second->ShardServerAddr(shard);
  }

  if (active_version_) {
    return active_version_->ShardServerAddr(shard);
  }
  LOG_WARN("ZkNodeService " << path_ << " no active version");
  return std::string();
}

void ZkNodeService::UpdateServiceState(const std::string & endpoint) {
}

bool ZkNodeService::Load() {
  std::vector<std::string> versions;
  if (zk_client_->GetChildren(path_.c_str(), &versions) != 0) {
    LOG_WARN("ZkNodeService " << path_ << " load versions err.");
    return false;
  }
  // version兄弟节点有增删时，通知
  zk_client_->AddChildListener(path_.c_str(), shared_from_this());

  ZkNodeVersionPtr active_version;

  LOG_INFO("ZkNodeService " << path_ << " versions.size=" << versions.size());
  std::map<std::string, ZkNodeVersionPtr> all_versions;

  // 需要监听所有 vresion 的数据, 但只加载active version 子节点即可 
  for(size_t i = 0; i < versions.size(); ++i) {
    std::string version_path = path_ + '/' + versions[i];

    // 关注所有子节点的数据变化
    zk_client_->AddNodeListener(version_path.c_str(), shared_from_this());

    ZkNodeVersionPtr version_ptr(new ZkNodeVersion(version_path, zk_client_));
    if (version_ptr->Load()) {
      if (!active_version) {
        active_version = version_ptr;
      } else {
        if (active_version->priority() < version_ptr->priority()) {
          active_version = version_ptr;
        }
      }
      all_versions.insert(std::make_pair(versions[i], version_ptr));
      LOG_INFO("ZkNodeService " << path_ << " load version " << version_path << " ok.");
    } else {
      LOG_WARN("ZkNodeService " << path_ << " load version " << version_path << " err.");
    }
  }

  if (active_version) {
    active_version_ = active_version;
    boost::mutex::scoped_lock(mutex_);
    all_versions.swap(all_versions_);
  }
  return true;
}

void ZkNodeService::HandleNodeEvent(const char * path, const std::string & value) {
  // child version data change event
  // TODO: 重新加载是简单粗暴的方法
  Load();
}

void ZkNodeService::HandleChildEvent(const char * path, const std::vector<std::string> & children) {
  Load();
}

std::string ZkNodeService::GetPathById(const std::string & service_id) {
  std::stringstream ss;
  std::vector<std::string> strs;
  boost::split(strs, service_id, boost::is_any_of("/"), boost::token_compress_on);
  for (size_t i = 0; i < strs.size(); ++i) {
    ss << '/' << strs[i];
  }

  return service_id;
//std::stringstream ss;
//std::vector<std::string> strs;
//boost::split(strs, service_id, boost::is_any_of("."), boost::token_compress_on);
//for (int i = strs.size() - 1; i >= 0; --i) {
//  ss << '/' << strs[i];
//}

//return ss.str();
}

}
}

