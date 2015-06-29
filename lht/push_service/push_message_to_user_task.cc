#include <string>
#include "base/curl_client.h"
#include "base/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "push_message_to_user_task.h"

using namespace std;
using namespace rapidjson;

namespace lht {

extern int g_retry_limit;
extern string g_auth;

PushMessageToUserTask::PushMessageToUserTask(const string& alias_in_, const string& content_in_, const string& title_in_, const string& type_in_, const map<string, string>& extras_in_, const map<string, string>& options_in_): alias(alias_in_), content(content_in_), title(title_in_), type(type_in_), extras(extras_in_), options(options_in_) {
  // do nothing
}

void PushMessageToUserTask::run() {
  LOG_INFO("PushMessageToUserTask receive : alias=" << alias << " content=" << content << " title=" << title << " type=" << type);

  Document document;
  Document::AllocatorType& allocator = document.GetAllocator();

  // 处理platform
  Value requestObj(kObjectType);
  requestObj.AddMember("platform", "all", allocator);

  // 处理audience
  Value aliasStr, aliasArr(kArrayType);
  aliasStr.SetString(alias.c_str(), allocator);
  aliasArr.PushBack(aliasStr, allocator);
  Value audienceObj(kObjectType);
  audienceObj.AddMember("alias", aliasArr, allocator);
  requestObj.AddMember("audience", audienceObj, allocator);
  
  // 处理message
  Value messageObj(kObjectType);
  Value contentStr, titleStr, typeStr;
  contentStr.SetString(content.c_str(), allocator);
  titleStr.SetString(title.c_str(), allocator);
  typeStr.SetString(type.c_str(), allocator);
  messageObj.AddMember("msg_content", contentStr.GetString(), allocator);
  messageObj.AddMember("title", titleStr.GetString(), allocator);
  messageObj.AddMember("content_type", typeStr.GetString(), allocator);

  // 处理extras  
  if (!extras.empty()) {
    Value extrasObj(kObjectType);
    for(map<string, string>::const_iterator it = extras.begin(); it != extras.end(); ++it) {
      Value valueStr;
      valueStr.SetString((it->second).c_str(), allocator);
      extrasObj.AddMember((it->first).c_str(), valueStr.GetString(), allocator);
    }
    messageObj.AddMember("extras", extrasObj, allocator);
  }

  // message加入request
  requestObj.AddMember("message", messageObj, allocator);

  // 处理options
  if (!options.empty()) {
    Value optionsObj(kObjectType);
    for(map<string, string>::const_iterator it = options.begin(); it != options.end(); ++it) {
      Value valueStr;
      valueStr.SetString((it->second).c_str(), allocator);
      optionsObj.AddMember((it->first).c_str(), valueStr.GetString(), allocator);
    }
    requestObj.AddMember("options", optionsObj, allocator);
  }

  // 通过requestObj得到json串
  rapidjson::StringBuffer requestStrBuff;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(requestStrBuff);
  requestObj.Accept(writer);
  std::string requestJson(requestStrBuff.GetString(), requestStrBuff.Size());
  LOG_DEBUG("PushMessageUserTask param : requestJson=" << requestJson);  
 
  // TODO 极光的接口调用需要在user－agent中加入验证串，因此当前的CurlClient的方法不满足需求，需要修改库文件
//int retry = g_retry_limit;
//string response;
//while (retry > 0 && base::CurlClient::Instance().Post(host_address_, ss.str(), &response) < 0) {
//  --retry;
//}
//if (retry <= 0) {
//  LOG_WARN("PushMessageToPhoneTask receive : alias=" << alias << " content=" << content << " title=" << title << " type=" << type << " result=JPush接口调用重试超时");
//  return;
//}
//Document document;
//document.Parse<0>(response.c_str());
//if (document.HasParseError()) {
//  LOG_WARN("PushMessageToPhoneTask receive : alias=" << alias << " content=" << content << " title=" << title << " type=" << type << " result=JPush返回结果Json格式错误");
//  return;
//}

  // TODO 此处应有JPush返回结果的验证和LOG
}

}



