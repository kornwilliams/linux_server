#ifndef _MELON_CLIENT_ENDPOINT_H_
#define _MELON_CLIENT_ENDPOINT_H_

#include "base/zk_client.h"

namespace melon {
namespace client {

class ZkNodeEndpoint {
 public:
  ZkNodeEndpoint(const std::string & path, base::ZookeeperClient * zk_client);
  ~ZkNodeEndpoint();
  bool Load();

  std::string addr() const {
    return addr_;
  }

  int32_t weight() const {
    return weight_;
  }

 private:
  std::string path_;

  std::string addr_;
  std::string host_;
  short port_;

  int32_t weight_;

  base::ZookeeperClient * zk_client_;
};
typedef boost::shared_ptr<ZkNodeEndpoint> ZkNodeEndpointPtr;

}
}

#endif // _MELON_CLIENT_ENDPOINT_H_
