#ifndef _MELON_CLIENT_SHARD_H_
#define _MELON_CLIENT_SHARD_H_

#include "zk_node_endpoint.h"
#include "base/zk_client.h"

namespace melon {
namespace client {

struct ZkNodeShard : public base::ZkEventListener {
 public:
  ZkNodeShard(const std::string & path, base::ZookeeperClient * zk_client);
  ~ZkNodeShard() {}

  virtual void HandleNodeEvent(const char * path, const std::string & value);
  virtual void HandleChildEvent(const char * path, const std::vector<std::string> & children);
 
  int id() const {
    return id_;
  }

  std::string SelectEndpoint();

  bool LoadEndpoints();
  void Dump();

 private:
  int id_;
  std::string path_;

  base::ZookeeperClient * zk_client_;

  mutable boost::shared_mutex endpoints_mutex_;

  std::vector<ZkNodeEndpointPtr> active_endpoints_;
  std::vector<ZkNodeEndpointPtr> inactive_endpoints_;
};
typedef boost::shared_ptr<ZkNodeShard> ZkNodeShardPtr;

}
}

#endif // _MELON_CLIENT_SHARD_H_

