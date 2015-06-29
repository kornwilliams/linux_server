#ifndef _FCGI_SERVICE_H_
#define _FCGI_SERVICE_H_

#include <fcgi_stdio.h>
#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include "fcgi_request.h"

namespace fcgi {

class FcgiThreadFunc {
public:
  FcgiThreadFunc(RequestFactoryPtr factory, int sock_fd); 
  void operator()();
private:
  RequestFactoryPtr factory_;
  int sock_fd_;
  FCGX_Request request_;
};

class FcgiServer : public RequestFactory, public boost::enable_shared_from_this<FcgiServer> {
public:
  FcgiServer(const std::string & sock, int thread_count);
  ~FcgiServer();

  int RegisterRequestFactory(RequestFactoryPtr factory);
  virtual RequestPtr Create(FCGX_Request* r);

  bool Start(bool no_exit);
  bool Stop();
private:
  std::vector<RequestFactoryPtr> request_factories_;
  std::string socket_;

  int thread_count_;
  std::vector<FcgiThreadFunc*> threads_funcs_;
private:
  FcgiServer(const FcgiServer &); 
  FcgiServer& operator= (const FcgiServer &); 
};

}

#endif // _FCGI_SERVICE_H_
