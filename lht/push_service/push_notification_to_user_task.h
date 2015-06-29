#ifndef _LHT_PUSH_NOTIFICATION_TO_USER_TASK_
#define _LHT_PUSH_NOTIFICATION_TO_USER_TASK_

#include <string>

#include "base/logging.h"

#include "thread_manager.h"

using namespace std;

namespace lht {

using apache::thrift::concurrency::Runnable;

class PushNotificationToUserTask : public Runnable {
 public:
  PushNotificationToUserTask(const string& alias_in_, const string& alert_in_, const string& title_in_, const map<string, string>& extras_in_, const map<string, string>& options_in_);
  virtual void run();
 private:
  const string& alias; 
  const string& alert;
  const string& title;
  const map<string, string>& extras;
  const map<string, string>& options;
};

}

#endif // _LHT_PUSH_NOTIFICATION_TO_USER_TASK_
