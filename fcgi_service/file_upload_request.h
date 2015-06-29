#ifndef _XCE_NOTIFY_FILE_UPLOAD_REQUEST_H_
#define _XCE_NOTIFY_FILE_UPLOAD_REQUEST_H_

#include "fcgi_request.h"
// #include <string>

namespace fcgi {

class Request;

class FileUploadRequest : public Request {
public:
  FileUploadRequest(FCGX_Request* r) : Request(r) {
  }  
  virtual bool Response();
  virtual ~FileUploadRequest() {}
protected:
  virtual std::string GetLocalFilename();
};

}

#endif // _XCE_NOTIFY_FILE_UPLOAD_REQUEST_H_
