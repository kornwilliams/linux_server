#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>

#include "base/logging.h"
#include "base/json_escape.h"
#include "fcgi_service/fcgi_service.h"

using namespace std;
using namespace fcgi;

class DemoRequest : public Request {
public:
  DemoRequest(FCGX_Request* r) : Request(r) {}
  virtual bool Response();
};

bool DemoRequest::Response() {
  static int count = 0;
  if(!fcgi_out_) {
    cout << "remove notify fcgi_out null";
    return false;
  }

  stringstream rsp_header;
  rsp_header << "Content-type: text/html\r\n" << "Set-Cookie: count="
             << ++count << "\r\n\r\n";
  FCGX_PutS(rsp_header.str().c_str(), fcgi_out_);

  string rsp_body;
  rsp_body += "<span style=\"color:red\">query:</span>:</br>";
  for(map<string, string>::const_iterator it = query_map_.begin(); it != query_map_.end(); ++it) {
    rsp_body += it->first;
    rsp_body += ":";
    rsp_body += it->second;
    rsp_body += "</br>";
  }

  rsp_body += "<span style=\"color:red\">cookies:</span>:</br>";
  for(map<string, string>::const_iterator it = cookie_map_.begin(); it != cookie_map_.end(); ++it) {
    rsp_body += it->first;
    rsp_body += ":";
    rsp_body += it->second;
    rsp_body += "</br>";
  }

  map<string, string>::const_iterator it = param_map_.find("REQUEST_URI");
  if (it != param_map_.end()) {
    rsp_body += "</p><h2>";
    rsp_body += it->second;
    rsp_body += "</h2>";
  }

  it = param_map_.find("SERVER_PROTOCOL");
  if (it != param_map_.end()) {
    rsp_body += "</p><h2>";
    rsp_body += it->second;
    rsp_body += "</h2>";
  }

  it = param_map_.find("REMOTE_ADDR");
  if (it != param_map_.end()) {
    rsp_body += "</p><h2>";
    rsp_body += it->second;
    rsp_body += "</h2>";
  }

  it = param_map_.find("REMOTE_PORT");
  if (it != param_map_.end()) {
    rsp_body += "</p><h2>";
    rsp_body += it->second;
    rsp_body += "</h2>";
  }
  FCGX_PutS(rsp_body.c_str(), fcgi_out_);
  return true;
}

class DemoRequestFactory : public RequestFactory {
public:
  virtual RequestPtr Create(FCGX_Request * r) {
    char * path = FCGX_GetParam("SCRIPT_NAME", r->envp);
    RequestPtr req;
    if (path) {
      if (strcmp(path, "/cgi/demo.html") == 0) {
        req = RequestPtr(new DemoRequest(r));
      }
    }
    return req;
  }
};


int main() {
  LOG_INIT("./fcgi_demo.log", "DEBUG");
  FcgiServer * fcgi_server = new FcgiServer("127.0.0.1:19000", 128);
  // FcgiServer * fcgi_server = new FcgiServer("10.22.206.157:19000", 128);
  fcgi_server->RegisterRequestFactory(RequestFactoryPtr(new DemoRequestFactory()));
  fcgi_server->Start(true); // 本线程也处理 fcgi 请求，不退出
  return 0;
}

