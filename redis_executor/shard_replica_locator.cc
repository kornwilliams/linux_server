#include "shard_replica_locator.h"

#include <set>
#include <boost/lexical_cast.hpp>

#include "base/config_reader.h"
#include "base/property_util.h"

namespace redis {

void ParseSlaveEndpoints(const std::string & read_endpoints, std::map<std::string, int32_t> * res);
std::string SelectSlaveEndpoint(const std::map<std::string, int32_t> & slave_endpoints);

ShardReplicaLocator::ShardReplicaLocator(const char * config_file, const char * config_section) {
  ConfigReader config(config_file);

  int shard_count = config.GetWithType<int>(std::string(config_section), std::string("shard_count"), 0);
  if (shard_count <= 0) {
    LOG_WARN("ShardReplicaLocator bad shard_count");
  }
  for(int i = 0; i < shard_count; ++i) {
    shards_.push_back(ShardConfig());
    std::ostringstream master_key, slaves_key;
    master_key << "master." << i;
    slaves_key << "slaves." << i;

    shards_.back().master_ = config.Get(config_section, master_key.str());

    std::string slaves = config.Get(config_section, slaves_key.str());
    ParseSlaveEndpoints(slaves, &(shards_.back().slaves_));
  }
}

int ShardReplicaLocator::ParseServerKey(const char * server_key, bool * use_master, int * shard) const {
  // server_key 的格式形如 "/w/30254"
  if (strlen(server_key) < 4 || server_key[0] != '/'
      || server_key[2] != '/') {
    return -1;
  }
  if (server_key[1] == 'w') {
    *use_master = true;
  } else if (server_key[1] == 'r') {
    *use_master = false;
  } else {
    return -2;
  }
  
  try {
    *shard = boost::lexical_cast<int>(server_key + 3);
  } catch (...) {
    return -3;
  }

  return 0;
} 

std::string ShardReplicaLocator::Locate(const char * server_key) {
  if (shards_.empty()) {
    LOG_WARN("ShardReplicaLocator::Locate err, shards_ empty.");
    return std::string();
  }

  bool use_master;
  int shard;
  int ret = ParseServerKey(server_key, &use_master, &shard);
  if (ret != 0) {
    LOG_WARN("ShardReplicaLocator::Locate err, server_key=" << server_key << ",use shard 0 master " << shards_[0].master_);
    return shards_[0].master_;
  }

  ShardConfig & shard_cfg = shards_[shard % shards_.size()];

  if (use_master) {
    return shard_cfg.master_;
  }

  // 选取读server
  std::string selected = SelectSlaveEndpoint(shard_cfg.slaves_);
  if (selected.empty()) {
    LOG_WARN("ShardReplicaLocator locate read key=" << server_key << " err, use master_endpoint=" << shard_cfg.master_);
    return shard_cfg.master_;
  }

  LOG_DEBUG("ShardReplicaLocator locate key=" << server_key << ", use slave_endpoint=" << selected);
  return selected;
}

}

