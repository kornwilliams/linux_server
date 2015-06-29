#include <mysql++/mysql++.h>
#include <boost/thread.hpp>

namespace base {

// 注意，多线程模式时，需要在线程运行时候，先调用mysql_thread_init(), 线程结束时候，
// 调用 mysql_thread_stop(). 
// 例如，可以封装线程对象，确保线程run/stop的时候, 调用相应的API.
class SimpleConnectionPool : public mysqlpp::ConnectionPool {
public:
  // The object's only constructor
  SimpleConnectionPool(const char * db, const char * server, const char * user, const char * password) :
    db_(db),
    server_(server),
    user_(user),
    password_(password) {
  }

  // The destructor.  We _must_ call ConnectionPool::clear() here,
  // because our superclass can't do it for us.
  ~SimpleConnectionPool();

  // Do a simple form of in-use connection limiting: wait to return
  // a connection until there are a reasonably low number in use
  // already.  Can't do this in create() because we're interested in
  // connections actually in use, not those created.  Also note that
  // we keep our own count; ConnectionPool::size() isn't the same!
  mysqlpp::Connection* grab();

  // Other half of in-use conn count limit
  void release(const mysqlpp::Connection* pc);

protected:
  // Superclass overrides
  mysqlpp::Connection* create();

  void destroy(mysqlpp::Connection* cp);

  unsigned int max_idle_time();

private:
  boost::mutex mutex_;

  // Our connection parameters
  std::string db_, server_, user_, password_;
};

}

