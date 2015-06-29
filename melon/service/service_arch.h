#include <boost/thread/thread.hpp>

#include <thrift/Thrift.h>

#include <thrift/transport/TServerTransport.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
// #include <thrift/server/TNonblockingServer.h>

#include <signal.h>
#include <sys/resource.h>

#include "base/zk_client.h"
#include "base/logging.h"
#include "base/config_reader.h"

namespace melon {
namespace service {

using namespace apache::thrift;

class AbstractProcessorFactory {
 public:
  virtual boost::shared_ptr<TProcessor> Create() = 0;
  virtual ~AbstractProcessorFactory() {}
};

template <class HandlerType, class ProcessorType>
class DefaultProcessorFactory : public AbstractProcessorFactory {
 public:
  static DefaultProcessorFactory & Instance() {
    static DefaultProcessorFactory instance; 
    return instance;
  }

  boost::shared_ptr<TProcessor> Create() {
    boost::shared_ptr<HandlerType> handler(new HandlerType());
    boost::shared_ptr<TProcessor> processor(new ProcessorType(handler));
    return processor;
  }
};

template <class HandlerType, class ProcessorType>
class ThriftService : public base::ZkEventListener {
 public:
  enum {
    STOPED,
    STARTING,
    READY,
  };

  ThriftService()
      : processor_factory_(NULL), server_(NULL), register_delay_(2), weight_(1000)
      , zk_client_(ConfigReader("../conf/zookeeper.conf").Get("zookeeper", "service_registry").c_str()) {
  }
  virtual ~ThriftService() {
    if (processor_factory_ != NULL) {
      delete processor_factory_;
    }
    if (server_ != NULL) {
      delete server_;
    }
  }

  void set_processor_factory(AbstractProcessorFactory * factory) {
    processor_factory_ = factory;
  }

  // 使用定制的thread类型替代默认thread
  void SetThreadFactory(boost::shared_ptr<concurrency::PlatformThreadFactory> factory) {
      thread_factory_ = factory;
  }


  void CheckServiceRegistry() {
    std::string v;
    int rc = zk_client_.GetValue(service_path_.c_str(), &v);
    if (rc != 0) {
      LOG_WARN("Zk CheckServiceRegistry " << service_path_ << " err, try to re-register.");
      DoRegister(false);
    } else {
      LOG_INFO("Zk CheckServiceRegistry " << service_path_ << " ok.");
    }
  }

  virtual void HandleNodeEvent(const char * path, const std::string & value) {
    LOG_INFO("ZkEvent ThriftSevice HandleNodeEvent path=" << path << " value=" << value);
    CheckServiceRegistry();
  }
  virtual void HandleChildEvent(const char * path, const std::vector<std::string> & children) {
    LOG_INFO("ZkEvent ThriftSevice HandleChildEvent path=" << path);
    CheckServiceRegistry();
  }
  virtual void HandleSessionEvent(int state) {
    LOG_INFO("ZkEvent ThriftSevice session state " << state);
    CheckServiceRegistry();
  }

 private:

  virtual boost::shared_ptr<concurrency::PlatformThreadFactory> GetThreadFactory() {
    if (thread_factory_ == NULL) {
      boost::shared_ptr<concurrency::PlatformThreadFactory> factory = boost::shared_ptr<concurrency::PlatformThreadFactory>(new concurrency::PlatformThreadFactory());
#ifndef USE_BOOST_THREAD
      factory->setPriority(concurrency::PosixThreadFactory::HIGHEST);
#endif
      thread_factory_ = factory;
    }
    return thread_factory_;
  }

  static void UseMaxFdLimit() {
    struct rlimit limit;
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
      LOG_WARN("get open file limit eror.");
    } else {
      LOG_INFO("get rlim_cur=" << limit.rlim_cur << " rlim_max=" << limit.rlim_max);
    }
   
    limit.rlim_cur = limit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
      LOG_WARN("set rlim_cur=" << limit.rlim_cur << " rlim_max=" << limit.rlim_max << " err");
    } else {
      LOG_INFO("set rlim_cur=" << limit.rlim_cur << " rlim_max=" << limit.rlim_max << " ok");
    }
  }

  static void SignalHandler(int sig, siginfo_t*, void*) {
    if (sig == SIGHUP) {
      LOG_WARN("SIGHUP received.");
      typename std::set<ThriftService*>::iterator it = service_list_.begin();
      for(; it != service_list_.end(); ++it) {
        (*it)->Unregister();
      }
    } else if (sig == SIGUSR1) {
      LOG_WARN("SIGUSR1 received.");
    }
  }

  void InstallSiganl() {
    struct sigaction sa;
    sa.sa_sigaction = SignalHandler;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    this_holder_.reset(this);
    service_list_.insert(this);
  }
 
 public:
  static void * RegisterThread(void * data) {
    ThriftService * p = reinterpret_cast<ThriftService *>(data);

    boost::shared_ptr<ThriftService> obj_holder(p);

    p->DoRegister(true);
    while(true) {
      p->CheckServiceRegistry();
      sleep(rand() % 20 + 50);
    }
    return NULL;
  }

  // 注册服务
  int StartRegister(const std::string & service_id, const std::string & version,
                    int shard, const std::string & host, int port, int delay, int weight = 1000) {
    // zk_client_.set_root_path();
    zk_client_.Init(2000);

    std::stringstream service_path;
    service_path << service_id << "/v" << version << "/shard_" << shard << '/' << host << ':' << port;
    service_path_ = service_path.str();

    register_delay_ = delay > 0 ? delay : 2;
    weight_ = weight;

    pthread_t thread;
    pthread_create(&thread, NULL, &RegisterThread, (void*)this);
    pthread_detach(thread);
    return 0;
  }

  int Unregister() {
    int rc = zk_client_.SetValue(service_path_.c_str(), "weight=0");

    if (rc != 0) {
      LOG_WARN("Zk service unregister err, path=" << service_path_);
    }
    LOG_INFO("Zk service unregister ok, path=" << service_path_);
    return rc;
  }

  void Start(int port) {
    // UseMaxFdLimit(); // TODO : 支持fd limit配置

    if (server_ == NULL) {
      boost::shared_ptr<TProcessor> processor;
      if (processor_factory_ == NULL) {
        processor = DefaultProcessorFactory<HandlerType, ProcessorType>::Instance().Create();
      } else {
        processor = processor_factory_->Create();
      }

      boost::shared_ptr<protocol::TProtocolFactory> protocolFactory(new protocol::TBinaryProtocolFactory());
      boost::shared_ptr<transport::TTransportFactory> transportFactory(new transport::TFramedTransportFactory());

      transport::TServerSocket * server_sock = new transport::TServerSocket(port);
      // TODO : 细化socket选项
      // server_sock->setRecvTimeout(500); // 不能这样配置, 连接500ms idle, 会No more data to read
      // server_sock->setSendTimeout(200);
      //server_sock->setNoDelay(true);
      //server_sock->setLinger(false, 0);
      boost::shared_ptr<transport::TServerTransport> serverTransport(server_sock);

      boost::shared_ptr<concurrency::ThreadManager> threadManager = concurrency::ThreadManager::newSimpleThreadManager(128, 5);

      threadManager->threadFactory(GetThreadFactory());
      threadManager->start();

      server_ = new server::TThreadPoolServer(processor, serverTransport, transportFactory, protocolFactory, threadManager);
    //server_ = new server::TNonblockingServer(processor, transportFactory, transportFactory, protocolFactory, 
    //    protocolFactory, port, threadManager);
    }

    InstallSiganl();

    server_->serve();
    LOG_FATAL("Service stoped. path=" << service_path_);
  }

  bool Stop() {
    Unregister();
    server_->stop();
    return true;
  }
 private:
  int DoRegister(bool exit_on_error) {
    LOG_INFO("Sleep " << register_delay_ << " seconds before register, path=" << service_path_);
    sleep(register_delay_);
    const static int kMaxRetry = 5;
    int rc = 0;
    std::stringstream value;
    value << "weight=" << weight_;
    for(int i = 0; i < kMaxRetry; ++i) {
      zk_client_.Delete(service_path_.c_str(), false);
      rc = zk_client_.Create(service_path_.c_str(), value.str(), true);
      if (rc == 0) {
        LOG_INFO("Zk register ok, path=" << service_path_);
        zk_client_.AddNodeListener(service_path_.c_str(), shared_from_this());
        size_t pos = service_path_.rfind("/");
        zk_client_.AddChildListener(service_path_.substr(0, pos).c_str(), shared_from_this());
        zk_client_.AddSessionEventListener(shared_from_this());
        break;
      } else {
        LOG_WARN("Zk service register err, path=" << service_path_ << " err=" << zerror(rc));
      }
    }

    if (rc != 0) {
      LOG_ERROR("Zk service register fail after " << kMaxRetry << " retries, err=" << zerror(rc));
      if (exit_on_error) {
        exit(1);
      }
    }
    return rc;
  }
 private:
  AbstractProcessorFactory * processor_factory_;
  server::TThreadPoolServer * server_;
  // server::TNonblockingServer * server_;
  boost::shared_ptr<concurrency::PlatformThreadFactory> thread_factory_;
  std::string service_path_;
  int register_delay_;
  int weight_;

  static std::set<ThriftService*> service_list_;
  base::ZookeeperClient zk_client_;
  boost::shared_ptr<ThriftService> this_holder_;
};

template <class HandlerType, class ProcessorType>
std::set<ThriftService<HandlerType, ProcessorType> *> ThriftService<HandlerType, ProcessorType>::service_list_;

}
}

