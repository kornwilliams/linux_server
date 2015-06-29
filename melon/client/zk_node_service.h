#ifndef _MELON_CLIENT_SERVICE_H_
#define _MELON_CLIENT_SERVICE_H_

#include <map>

#include "zk_node_version.h"
#include "base/zk_client.h"

namespace melon {
namespace client {

class ZkNodeService : public base::ZkEventListener {
 public:
  ZkNodeService(const std::string & id, base::ZookeeperClient * zk_client);

  std::string LocateService(const std::string & version, int shard) const;

  void UpdateServiceState(const std::string & endpoint);

  bool Load();

  virtual void HandleNodeEvent(const char * path, const std::string & value);
  virtual void HandleChildEvent(const char * path, const std::vector<std::string> & children);

  std::string id() const {
    return id_;
  }

 private:
  std::string id_;
  std::string path_;
 
  base::ZookeeperClient * zk_client_;

  mutable boost::mutex mutex_;
  ZkNodeVersionPtr active_version_;
  std::map<std::string, ZkNodeVersionPtr> all_versions_;

  std::string GetPathById(const std::string & service_id);
};
typedef boost::shared_ptr<ZkNodeService> ZkNodeServicePtr;

}
}

#endif // _MELON_CLIENT_SERVICE_H_

