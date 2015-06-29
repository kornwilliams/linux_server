include "BaseService.thrift"

namespace cpp lht

service LhtPushService extends BaseService.BaseService {
  i64 PushNotificationToUser(1: string user, 2: string alert, 3: string title, 4: map<string, string> extras, 5: map<string, string> options);
  i64 PushMessageToUser(1: string user, 2: string content, 3: string title, 4: string type, 5: map<string, string> extras, 6: map<string, string> options);
}
