#include "dump_request.h"

#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include "base/logging.h"
#include "base/url_encode.h"

namespace lht {
namespace fcgi {

bool DumpRequest::Response() {
  static int count = 0;
  if (!fcgi_out_) {
    LOG_WARN("null fcgi_out!");
    return false;
  }

  std::stringstream rsp_header;
  rsp_header << "Content-type: text/html\r\n" << "Set-Cookie: count="
             << ++count << "\r\n\r\n";
  FCGX_PutS(rsp_header.str().c_str(), fcgi_out_);

  std::string rsp_body;
  rsp_body += "</br><span style=\"color:red\">query:</span></br>";
  for(std::map<std::string, std::string>::const_iterator it = query_map_.begin(); it != query_map_.end(); ++it) {
    rsp_body += it->first;
    rsp_body += ":";
    rsp_body += it->second;
    rsp_body += "</br>";
  }

  rsp_body += "</br></br><span style=\"color:red\">cookies:</span></br>";
  for(std::map<std::string, std::string>::const_iterator it = cookie_map_.begin(); it != cookie_map_.end(); ++it) {
    rsp_body += it->first;
    rsp_body += ":";
    rsp_body += it->second;
    rsp_body += "</br>";
    rsp_body += it->first;
    rsp_body += ":";
    rsp_body += base::UrlEncode(it->second);
    rsp_body += "</br>";
  }


  std::string ua = GetParam("HTTP_USER_AGENT");
  rsp_body += "</br></br><span style=\"color:red\">HTTP_USER_AGENT:</span></br>";
  rsp_body += ":";
  rsp_body += ua;
  rsp_body += "</br>";

  FCGX_PutS(rsp_body.c_str(), fcgi_out_);
  return true;
}

} // namesapce fcig
} // namespace lht


