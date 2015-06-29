#ifndef _LHT_USER_PROFILE_SERVICE_DB_ADAPTER_H_
#define _LHT_USER_PROFILE_SERVICE_DB_ADAPTER_H_

#include <string>
#include "LhtUserProfileService_types.h"

using std::string;

namespace base {
class SimpleConnectionPool;
};

namespace lht {

class UserProfileDBAdapter {
public:
  static UserProfileDBAdapter & Instance() {
    static UserProfileDBAdapter cache;
    return cache;
  }

  int InsertUser(int64_t& uid, const string& phone, const string& pw);  
private:
  UserProfileDBAdapter();

  base::SimpleConnectionPool * db_conn_pool_;

};

}

#endif // _LHT_USER_PROFILE_SERVICE_DB_ADAPTER_H_


