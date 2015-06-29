#include "db_adapter.h"

#include "LhtUserProfileService_types.h"
#include <sstream>
#include <time.h>
#include "base/logging.h"
#include "base/config_reader.h"
#include "database/scoped_connection.h"
#include "database/simple_db_pool.h"

using std::string;
using std::stringstream;

namespace lht {

UserProfileDBAdapter::UserProfileDBAdapter() : db_conn_pool_(NULL) {
  ConfigReader config("../conf/lht_user_profile_service.conf");

  db_conn_pool_ = new base::SimpleConnectionPool(config.Get("db", "name").c_str(),
                      config.Get("db", "server_list").c_str(),
                      config.Get("db", "user").c_str(),
                      config.Get("db", "password").c_str());
}

int UserProfileDBAdapter::InsertUser(int64_t& uid, const string& phone, const string& pw) {
  time_t now = time(NULL);

  stringstream ss;
  ss << "phone=" << phone << " password=" << pw << " now=" << now;

  mysqlpp::ScopedConnection db(*db_conn_pool_);
  if (!db) {
    LOG_ERROR("UserProfileDBAdapter InsertUser : " << ss.str() << " reason=连接数据库失败");
    return -1;
  }

  mysqlpp::Query query(db->query());
  query << "INSERT INTO lht_users (user_name, password, reg_time, last_login, last_time) VALUES("
        << mysqlpp::quote << phone << ", "
        << mysqlpp::quote << pw << ", "
        << now << ", "
        << now << ", "
        << now << ")";
  try {
    query.exec();
  } catch (mysqlpp::Exception &e) {
    if (strstr(e.what(), "Duplicate")) {
      LOG_WARN("UserProfileDBAdapter InsertUser : " << ss.str() << " reason=用户已注册");
      return -3;
    }
    LOG_ERROR("UserProfileDBAdapter InsertUser : " << ss.str() << " sql=" << query.str() << " err=" << e.what());
    return -2;
  }

  query << "SELECT LAST_INSERT_ID()";
  try {
    mysqlpp::UseQueryResult ret = query.use();
    if (ret) {
      if (mysqlpp::Row row = ret.fetch_row()) {
        uid= row["user_id"];
        return 0;
      }
    }
    LOG_ERROR("UserProfileDBAdapter InsertUser : " << ss.str() << " sql=" << query.str());
    return -2;
  } catch (mysqlpp::Exception& e) {
    LOG_ERROR("UserProfileDBAdapter InsertUser : " << ss.str() << " sql=" << query.str() << " err=" << e.what());
    return -2;
  }

  return 0;
}

} // namespace lht;
