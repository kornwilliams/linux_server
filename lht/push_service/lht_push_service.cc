#include "LhtPushService_constants.h"
#include "LhtPushService.h"

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include "melon/service/service_arch.h"
#include "melon/service/base_service_impl.h"

#include "base/logging.h"
#include "base/get_local_ip.h"

#include "thread_manager.h"
#include "push_notification_to_user_task.h"
#include "push_message_to_user_task.h"

using namespace melon::service;
using namespace std;

namespace lht {

int g_retry_limit = 3;
string g_auth;

class LhtPushServiceHandler : public LhtPushServiceIf, public BaseServiceImpl {
 public:
  LhtPushServiceHandler() {}

  virtual int64_t PushNotificationToUser(const string& alias, const string& alert, const string& title, const map<string, string>& extras, const map<string, string>& options) {
    if (!alias.empty() && !alert.empty() && !title.empty()) {
      GetThreadManager()->add(boost::shared_ptr<Runnable>(new PushNotificationToUserTask(alias, alert, title, extras, options)));
    } else {
      LOG_WARN("LhtPushServiceHandler PushNotificationToUser err : alias=" << alias << " alert=" << alert << " title=" << title);
    }
    return 0;
  }

  virtual int64_t PushMessageToUser(const string& alias, const string& content, const string& title, const string& type, const map<string, string>& extras, const map<string, string>& options) {
    if (!alias.empty() && !content.empty()) {
      GetThreadManager()->add(boost::shared_ptr<Runnable>(new PushMessageToUserTask(alias, content, title, type, extras, options)));
    } else {
      LOG_WARN("LhtPushServiceHandler PushMessageToUser err : alias=" << alias << " content=" << content << " title=" << title << " type=" << type);
    }
    return 0;
  }

};

}

int main(int argc, char **argv) {
  char * conf_file = "../conf/lht_push_service.conf";
  char opt;
  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
    case 'c':
      conf_file = optarg;
      break;
    default:
      std::cerr << "Unknown option " << optopt << std::endl;
      return 1;
    }
  }

  ConfigReader cfg(conf_file);
  if (!cfg) {
    std::cerr << "Config file " << conf_file << " read error!" << std::endl;
    return 1;
  }

  using namespace lht;
  LOG_INIT(cfg.Get("log4cplus", "file"), cfg.Get("log4cplus", "level"));
  int port = cfg.GetWithType<int>("lht_push_service", "port", 19090);
  g_retry_limit = cfg.GetWithType<int>("lht_push_service", "retry", 3);
  g_auth = cfg.Get("jpush", "auth");
  LOG_INFO("lht_push_service start : port=" << port << " g_retry_limit=" << g_retry_limit << " g_auth=" << g_auth);

  ThriftService<LhtPushServiceHandler, LhtPushServiceProcessor> service;
  service.StartRegister("/lht/lht_push_service", "1.0", 0, base::GetLocalIp(), port, 1);
  service.Start(port);
  return 0;
}
