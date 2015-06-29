#ifndef _REDIS_SHARD_REPLICA_LOCATOR_H_
#define _REDIS_SHARD_REPLICA_LOCATOR_H_

#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <hiredis/hiredis.h>

#include "redis_locator.h"

namespace redis {

struct ShardConfig {
  std::string master_;
  std::map<std::string, int32_t> slaves_;
};

class ShardReplicaLocator : public RedisLocator {
 public:
  ShardReplicaLocator(const char * config_file, const char * config_section);

  virtual ~ShardReplicaLocator(){}

  virtual std::string Locate(const char * locate_key);
 private:
  int ParseServerKey(const char * server_key, bool * use_master, int * shard) const;
  std::vector<ShardConfig> shards_;
};

}

#endif // _REDIS_SHARD_REPLICA_LOCATOR_H_

