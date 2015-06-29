#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

#include "fcgi_request.h"
#include "base/url_encode.h"
#include "base/logging.h"

namespace fcgi {

bool Request::ParseKeyValue(const char * data, const char * key_seps,
    char kv_sep, std::map<std::string, std::string> * pairs) const {
  if (!data) {
    return false;
  }

  std::vector<std::string> strs;
  boost::split(strs, data, boost::is_any_of(key_seps),
      boost::token_compress_on);
  for (size_t i = 0; i < strs.size(); ++i) {
    size_t pos = strs[i].find_first_of(kv_sep);
    if (pos != std::string::npos) {
      (*pairs)[strs[i].substr(0, pos)] = base::UrlDecode(strs[i].substr(pos + 1));
    } else {
      (*pairs)[strs[i]] = "";
    }
  }
  return true;
}

const std::string & Request::GetQuery(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it = query_map_.find(key);
  if (it != query_map_.end()) {
    return it->second;
  }
  static std::string empty;
  return empty;
}

const std::string & Request::GetCookie(const std::string& key) const { //返回 cookie 中 key 对应的value
  std::map<std::string, std::string>::const_iterator it = cookie_map_.find(key);
  if (it != cookie_map_.end()) {
    return it->second;
  }
  static std::string empty;
  return empty;
}

const std::string & Request::GetPostArg(const std::string& key) const {
  if (!body_.empty() && body_map_.empty()) {
    ParseKeyValue(body_.c_str(), "&", '=', &body_map_);
  }

  std::map<std::string, std::string>::const_iterator it = body_map_.find(key);
  if (it != body_map_.end()) {
    return it->second;
  }
  static std::string empty;
  return empty;
}

int64_t Request::GetPostArgInt(const std::string& key, int64_t dft) const {
  std::string value = GetPostArg(key);
  int64_t ret = dft;
  try {
    ret = boost::lexical_cast<int64_t>(value);
  } catch (boost::bad_lexical_cast & e) {
    LOG_WARN("bad_lexical_cast : " << e.what());
  }
  return ret;
}

int32_t Request::GetCookieInt(const std::string& key, int32_t dft) const {
  int32_t ret = dft;
  std::map<std::string, std::string>::const_iterator it = cookie_map_.find(key);
  if (it != cookie_map_.end()) {
    try {
      ret = boost::lexical_cast<int32_t>(it->second);
    } catch (boost::bad_lexical_cast & e) {
      LOG_WARN("bad_lexical_cast : " << e.what());
    }
  }
  return ret;
}

int32_t Request::GetQueryInt(const std::string& key, int32_t dft) const {
  int32_t ret = dft;
  std::map<std::string, std::string>::const_iterator it = query_map_.find(key);
  if (it != query_map_.end()) {
    try {
      ret = boost::lexical_cast<int32_t>(it->second);
    } catch (boost::bad_lexical_cast & e) {
      LOG_WARN("bad_lexical_cast : " << e.what());
    }
  }
  return ret;
}

int64_t Request::GetQueryLong(const std::string& key, int64_t dft) const {
  int64_t ret = dft;
  std::map<std::string, std::string>::const_iterator it = query_map_.find(key);
  if (it != query_map_.end()) {
    try {
      ret = boost::lexical_cast<int64_t>(it->second);
    } catch (boost::bad_lexical_cast & e) {
      LOG_WARN("bad_lexical_cast : " << e.what());
    }
  }
  return ret;
}

bool Request::Response() {
  if (!fcgi_out_) {
    LOG_WARN("null fcgi out stream");
    return false;
  }
  return true;
}

const std::string & Request::GetParam(const std::string & name) const {
  std::map<std::string, std::string>::const_iterator i = param_map_.find(name);
  if (i != param_map_.end()) {
    return i->second;
  }
  static std::string empty;
  return empty;
}

std::string Request::GetClientAddr() const {
  std::string addr = GetParam("HTTP_X_FORWARDED_FOR");
  if (addr.empty()) {
    addr = GetParam("REMOTE_ADDR");
  } else {
    size_t pos = addr.find(',');
    if (pos != std::string::npos) {
      addr = addr.substr(0, pos);
    }
  }
  return addr;
}

void Request::InitParamMap(FCGX_Request* req) {
  for(int i = 0; req->envp[i]; ++i) {
    std::string s = req->envp[i];
    size_t pos = s.find_first_of('=');
    if (pos > 0 && pos < s.size() - 1) {
      param_map_.insert(make_pair(s.substr(0,pos), s.substr(pos+1)));
    }
  }
}

void Request::Init(FCGX_Request* r) {
  InitParamMap(r); 

  //parse query_string
  ParseKeyValue(GetParam("QUERY_STRING").c_str(), "&", '=', &query_map_);

  //parse cookie
  ParseKeyValue(GetParam("HTTP_COOKIE").c_str(), "; ", '=', &cookie_map_);

  //parse post body
  int content_len = 0;
  try {
    content_len = boost::lexical_cast<int32_t>(GetParam("CONTENT_LENGTH"));
  } catch (boost::bad_lexical_cast & e) {}

  if (content_len > 0) {
    char * str = new char[content_len + 1];
    int count = FCGX_GetStr(str, content_len, r->in);
    str[content_len] = 0;
    if (count > 0) {
      body_ = std::string(str, count);
    }
    delete [] str;
  }
}

}

