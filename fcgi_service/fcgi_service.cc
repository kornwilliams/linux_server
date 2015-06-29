#include "fcgi_service.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>

#include "base/logging.h"

namespace fcgi {

FcgiThreadFunc::FcgiThreadFunc(RequestFactoryPtr factory, int sock_fd) 
  : factory_(factory)
  , sock_fd_(sock_fd) {
}

void FcgiThreadFunc::operator()() {
  if (FCGX_InitRequest(&request_, sock_fd_, 0) != 0) {
    LOG_WARN("FCGX_InitRequest error. fd=" << sock_fd_);
    return;
  }

  while(FCGX_Accept_r(&request_) >= 0) {
    RequestPtr req = factory_->Create(&request_);
    if (req) {
      try {
        req->Response();
      } catch(std::exception& e) {
        LOG_WARN("FcgiThread::run err:" << e.what() << ",File:"<<__FILE__ << ",Line:" << __LINE__);
      } catch(...) {
        LOG_WARN("FcgiThread::run err File:"<<__FILE__ << ",Line:" << __LINE__);
      }
    } else {
      LOG_WARN("unhandled request.");
    }
    FCGX_Finish_r(&request_);
  }
} 

FcgiServer::FcgiServer(const std::string & sock, int thread_count) 
  : socket_(sock)
  , thread_count_(thread_count) {
}

FcgiServer::~FcgiServer() {
}

bool FcgiServer::Start(bool no_exit) {
  if (FCGX_Init() != 0) {
    LOG_WARN("FCGX_Init err");
    return false;
  }

  int sock_fd = FCGX_OpenSocket(socket_.c_str(), 100);
  if (sock_fd < 0){
    LOG_WARN("FCGX_OpenSocket err");
    return false;
  }
  threads_funcs_.reserve(thread_count_);

  for(int i = 0; i < thread_count_; ++i) {
    FcgiThreadFunc * func = new FcgiThreadFunc(RequestFactoryPtr(this), sock_fd);
    boost::thread thread(*func);
    threads_funcs_.push_back(func);
    // IceUtil::ThreadControl tc = t->start();
    thread.detach();
  }
  if (no_exit) {
    FcgiThreadFunc * func = new FcgiThreadFunc(RequestFactoryPtr(this), sock_fd);
    (*func)();
  }
  return true;
}

bool FcgiServer::Stop() {
  return false;
}

RequestPtr FcgiServer::Create(FCGX_Request* r) {
  RequestPtr p;
  for(size_t i = 0; i < request_factories_.size(); ++i) {
    p = request_factories_[i]->Create(r);
    if (p) {
      return p;
    }
  }
  return p;
}

int FcgiServer::RegisterRequestFactory(RequestFactoryPtr factory) {
  if (factory) {
    request_factories_.push_back(factory);
    return 0;
  }
  return -1;
}

}
