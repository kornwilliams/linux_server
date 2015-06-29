#include "zk_node_version.h"

#include "base/property_util.h"
#include "base/logging.h"

namespace melon {
namespace client {

static std::string ParseVersion(const std::string & version_path) {
  if (version_path.size() < 2) {
    return std::string();
  }
  return version_path.substr(1);
}

ZkNodeVersion::ZkNodeVersion(const std::string & path, base::ZookeeperClient * zk_client) : path_(path), priority_(0), shard_factor_(1), zk_client_(zk_client) {
  version_ = ParseVersion(path);
}

void ZkNodeVersion::HandleNodeEvent(const char * path, const std::string & value) {
  // 自身数据的变化，由service节点处理
}

void ZkNodeVersion::HandleChildEvent(const char * path, const std::vector<std::string> & children) {
  Load();
}


bool ZkNodeVersion::Load() {
  std::vector<std::string> shards;
  if (zk_client_->GetChildren(path_.c_str(), &shards) != 0) {
    LOG_WARN("ZkNodeVersion " << path_ << " get children error");
    return false;
  }

  std::string version_data;
  if (zk_client_->GetValue(path_.c_str(), &version_data) != 0) {
    LOG_WARN("ZkNodeVersion "  << path_ << " get data error");
    return false;
  }

  std::map<std::string, std::string> version_props;
  base::ParseProperty(version_data, &version_props);
  priority_ = base::GetProperty<int>(version_props, "priority", 0);
  shard_factor_ = base::GetProperty<int>(version_props, "shard_factor", 1);

  // zk_client_->AddNodeListener(path_.c_str(), shared_from_this());
  zk_client_->AddChildListener(path_.c_str(), shared_from_this());

  LOG_INFO("ZkNodeVersion "  << path_ << " shards.size=" << shards.size());
  std::map<int, ZkNodeShardPtr> shard_ptrs;
  for(size_t i = 0; i < shards.size(); ++i) {
    // 只关注child, 不关注node本身
    std::string shard_path = path_ + '/' + shards[i];
    LOG_INFO("ZkNodeVersion "  << path_ << " load shard : " << shard_path);
    // zk_client_->AddChildListener(shard_path.c_str(), shared_from_this());
    ZkNodeShardPtr shard(new ZkNodeShard(shard_path, zk_client_));
    shard->LoadEndpoints();
    shard_ptrs.insert(std::make_pair(shard->id(), shard));
  }

  {
    boost::unique_lock<boost::shared_mutex> wlock(shards_mutex_);
    shard_ptrs.swap(shards_);
  }
  return true; 
}

std::string ZkNodeVersion::ShardServerAddr(size_t shard) const {
  ZkNodeShardPtr shard_ptr;
  {
    boost::shared_lock<boost::shared_mutex> rlock(shards_mutex_);
    if (shards_.empty()) {
      LOG_WARN("ZkNodeVersion "  << path_ << " no shard");
      return std::string();
    }

    std::map<int, ZkNodeShardPtr>::const_iterator it = shards_.find(shard);
    if (it != shards_.end()) {
      // LOG_DEBUG("ZkNodeVersion "  << path_ << " shard " << shard << " found");
      shard_ptr = it->second;
    } else {
      LOG_WARN("ZkNodeVersion "  << path_ << " shard " << shard << " not found");
      shard_ptr = shards_.begin()->second;
    }
  }

  if (shard_ptr) {
    return shard_ptr->SelectEndpoint();
  }
  return std::string();
}

}
}

