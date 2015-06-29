#ifndef _MELON_CLIENT_GROUP_H_
#define _MELON_CLIENT_GROUP_H_

#include "zk_node_shard.h"
#include "base/zk_client.h"

namespace melon {
namespace client {

class ZkNodeVersion : public base::ZkEventListener {
 public:
  ZkNodeVersion(const std::string & path, base::ZookeeperClient * zk_client);

  virtual void HandleNodeEvent(const char * path, const std::string & value);
  virtual void HandleChildEvent(const char * path, const std::vector<std::string> & children);

  int priority() const {
    return priority_;
  }
  bool Load();
  std::string ShardServerAddr(size_t shard) const;

 private:
  std::string path_;
  std::string version_;
  int priority_;
  int shard_factor_;

  base::ZookeeperClient * zk_client_;

  mutable boost::shared_mutex shards_mutex_;
  std::map<int, ZkNodeShardPtr> shards_;
};
typedef boost::shared_ptr<ZkNodeVersion> ZkNodeVersionPtr;

}
}

#endif // _MELON_CLIENT_GROUP_H_


