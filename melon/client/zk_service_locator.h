#ifndef _MELON_CLIENT_SERVICE_LOCATOR_H_
#define _MELON_CLIENT_SERVICE_LOCATOR_H_

#include "service_locator.h"

#include <vector>
#include <string>
#include <boost/thread.hpp>

#include "base/zk_client.h"
#include "zk_node_service.h"

namespace melon {
namespace client {

class ZkServiceLocator : public ServiceLocator {
 public:
  ZkServiceLocator(const char * service_registry) 
      : zk_client_(service_registry) {
    zk_client_.Init();
  }

  virtual ~ZkServiceLocator(){}

  virtual std::string Locate(const char * service_id);
 private:
  base::ZookeeperClient zk_client_;

  mutable boost::shared_mutex service_mutex_;
  typedef std::map<std::string, ZkNodeServicePtr> ServiceMap;
  std::map<std::string, ZkNodeServicePtr> service_map_;
};

}
}

#endif // _MELON_CLIENT_SERVICE_LOCATOR_H_

