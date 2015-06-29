#include "file_upload_request.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>

#include "base/logging.h"

namespace fcgi {

std::string FileUploadRequest::GetLocalFilename() {
  return "/data/www/trunk/cpp/release/uploaded_file";
}

// 读出一个command line, 返回值指向'\r\n' 的'\n'字符
const char * GetLineEnd(const char * buf, size_t len) {
  const char * p = buf + 1; // 首字符肯定不是'\n'

  for(;;) {
    p = (const char *)memchr(p, '\n', len - (p - buf));
    if (!p) {
      break;
    }
    if(*(p - 1) == '\r') {
      break;
    }
    ++ p; // p 指向 '\n' 的下一个字符
  }

  return p;
}

int FindNextPart(const char * str, int content_len, const std::string & boundary) {
  if ((size_t)content_len < boundary.size() + 4) {
    return -1;
  }
  const char * p = str;
  if (p[0] != '-' || p[1] != '-') {
    return -2;
  }
  p += 2;
  if (strncmp(p, boundary.c_str(), boundary.size()) != 0) {
    return -3;
  }
  p += boundary.size();
  if (p[0] == '-' && p[1] == '-') {
    // 最后一个part
    return 0;
  }
  if (p[0] != '\r' || p[1] != '\n') {
    return -1;
  }
  p += 2;
  const char * q;
  while((q = GetLineEnd(p, content_len - (p - str)))) {
    if (q - p == 2) {
      return q - str + 1;
    }
    p = q;
  }

  return -1;
}

bool FileUploadRequest::Response() {
  LOG_DEBUG("FileUploadRequest----------");
  if (!fcgi_out_) {
    LOG_WARN("null fcgi out stream");
    return false;
  }

  std::string content_type = GetParam("CONTENT_TYPE");

  std::stringstream ss;
  ss << "Content-type: text/html\r\n\r\n"  
     << "Request_uri: << " << GetParam("REQUEST_URI") << "<br/>"
     << "Content_type: << " << content_type << "<br/>";  

  // get boundary
//std::string pattern = "boundary=";
//std::string boundary;
//size_t pos = content_type.find(pattern);
//if (pos != std::string::npos) {
//  boundary = content_type.substr(pos + pattern.size());
//}
//LOG_DEBUG("boundary " << boundary);

//std::string length = GetParam("CONTENT_LENGTH");  
//int content_len = strtol(length.c_str(), NULL, 10);  
//ss << "Content_length: " << content_len << "<br/>";
  LOG_DEBUG("ss content" << ss.str());
//std::ofstream local_file(GetLocalFilename().c_str());

//// printf("Standard input:<br><pre>");
//if (content_len > 0) {
//  if (!body_.empty()) {
//    int pos = FindNextPart(body_.c_str(), body_.size(), boundary);
//    LOG_DEBUG("pos : " << pos);
//    if (pos > 0) {
//      // "\r\n--[boundary]--\r\n"
//      local_file.write(body_.c_str() + pos, body_.size() - pos - boundary.size() - 8);
//    }
//  }
//}
//LOG_DEBUG("file content length : " << content_len);

  FCGX_PutS(ss.str().c_str(), fcgi_out_);
  return true;
}

}

