#ifndef _MELON_CLIENT_CLIENT_POOL_
#define _MELON_CLIENT_CLIENT_POOL_

#include <iostream>

#include <boost/lexical_cast.hpp>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include "base/object_pool.h"
#include "base/config_reader.h"
#include "service_locator.h"
#include "direct_service_locator.h"
#include "zk_service_locator.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace melon {
namespace client {

template <typename ClientType>
class ClientPool : public base::ObjectPool<ClientType> {
 private:
  ServiceLocator * locator_;
  std::string default_location_;
  int timeout_;
  bool ParseAddr(const std::string & addr, std::string * host, int * port) const {
    size_t pos = addr.find_first_of(':');
    if (pos == std::string::npos) {
      return false;
    }
    *host = addr.substr(0, pos);

    try {
      *port = boost::lexical_cast<int>(addr.substr(pos + 1));
    } catch (boost::bad_lexical_cast &) {
      return false;
    }
    return true;
  }
 public:
  void set_default_location(const std::string & default_location) {
    default_location_ = default_location;
  }

  explicit ClientPool(int timeout = 260) : base::ObjectPool<ClientType>(0, 20, 0, 100), timeout_(timeout) {
    ConfigReader zk_cfg("../conf/zookeeper.conf");
    if (!zk_cfg) {
      LOG_WARN("Config file ../conf/zookeeper.conf read error!");
      return;
    }
    LOG_INFO("zk_locator_ created.");
    locator_ = new ZkServiceLocator(zk_cfg.Get("zookeeper", "service_registry").c_str());
  } 

  explicit ClientPool(ServiceLocator * locator, int timeout = 260) : base::ObjectPool<ClientType>(0, 20, 0, 100), locator_(locator), timeout_(timeout) {
  } 

  void set_locator(ServiceLocator * locator) {
    locator_ = locator;
  }

  std::string SelectServer(const char * key) const {
    if (locator_ == NULL) {
      return std::string();
    }

    std::string location = locator_->Locate(key);
    if (location.empty()) {
      LOG_WARN("key=" << key << " locate err, use default=" << default_location_);
      location = default_location_;
    }

    return location;
  }

  std::string SelectServer(const std::string & key) const {
    return SelectServer(key.c_str());
  }

  virtual ClientType * Create(const std::string & endpoint) {
    std::string host;
    int port;

    if (!ParseAddr(endpoint, &host, &port)) {
      LOG_WARN("bad thrift service endpoint:" << endpoint << ".");
      return NULL;
    }

    boost::shared_ptr<TSocket> socket(new TSocket(host.c_str(), port));

    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    // boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    try {
      transport->open();
      LOG_DEBUG("Thrift connect ok. endpoint=" << endpoint);
    } catch (TTransportException & e) {
      LOG_WARN("Thrift connect error endpoint=" << endpoint << " e=" << e.what());
      return NULL;
    } catch (...) {
      LOG_WARN("Thrift connect unknown error. endpoint=" << endpoint);
      return NULL;
    }

    // TODO : 细化tcp选项
    socket->setConnTimeout(timeout_);
    socket->setSendTimeout(timeout_);
    socket->setRecvTimeout(timeout_);
    socket->setNoDelay(true);
    socket->setLinger(false, 0);

    return new ClientType(protocol);
  }

  virtual void Destroy(ClientType * p) {
    // transport 的析够会调用close()
    delete p;
  }
};

}
}

#endif // _MELON_CLIENT_CLIENT_POOL_

