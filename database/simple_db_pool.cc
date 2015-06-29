#include <iostream>

#include "base/logging.h"
#include "database/simple_db_pool.h"

namespace base {

SimpleConnectionPool::~SimpleConnectionPool() {
  clear();
}

mysqlpp::Connection* SimpleConnectionPool::grab() {
  // return mysqlpp::ConnectionPool::grab();
  boost::mutex::scoped_lock lock(mutex_);
  
  mysqlpp::Connection * conn = NULL;
  try {
    conn = new mysqlpp::Connection(
        db_.empty() ? 0 : db_.c_str(),
        server_.empty() ? 0 : server_.c_str(),
        user_.empty() ? 0 : user_.c_str(),
        password_.empty() ? "" : password_.c_str());

    mysqlpp::Query query = conn->query("SET names 'utf8'");
    query.exec();
  } catch (mysqlpp::Exception& e) {
    LOG_WARN("new conn create ex=" << e.what()
      << " db=" << db_
      << " server_=" << server_
      << " user_=" << user_);
    delete conn;
    return NULL;
  } catch (...) {
    LOG_WARN("new conn create unknown ex"
      << " db=" << db_
      << " server_=" << server_
      << " user_=" << user_);
    delete conn;
    return NULL;
  }
  LOG_INFO("new conn created."
    << " db=" << db_
    << " server_=" << server_
    << " user_=" << user_);
  return conn;
}

void SimpleConnectionPool::release(const mysqlpp::Connection* pc) {
  mysqlpp::Connection* p = const_cast<mysqlpp::Connection*>(pc);
  if (p) {
    p->disconnect();
    delete p;
  }
  // mysqlpp::ConnectionPool::release(pc);
}

mysqlpp::Connection* SimpleConnectionPool::create() {
  mysqlpp::Connection * conn = NULL;
  try {
    conn = new mysqlpp::Connection(
        db_.empty() ? 0 : db_.c_str(),
        server_.empty() ? 0 : server_.c_str(),
        user_.empty() ? 0 : user_.c_str(),
        password_.empty() ? "" : password_.c_str());

    mysqlpp::Query query = conn->query("SET names 'utf8'");
    query.exec();
  } catch (mysqlpp::Exception& e) {
    LOG_WARN("new conn create ex=" << e.what());
    delete conn;
    return NULL;
  } catch (...) {
    LOG_WARN("new conn create unknown ex"
      << " db=" << db_
      << " server_=" << server_
      << " user_=" << user_);
    delete conn;
    return NULL;
  }
  LOG_INFO("new conn created.");
  return conn;
}

void SimpleConnectionPool::destroy(mysqlpp::Connection* pc) {
  // Our superclass can't know how we created the Connection, so
  // it delegates destruction to us, to be safe.
  if (pc) {
    pc->disconnect();
    delete pc;
  }
}

unsigned int SimpleConnectionPool::max_idle_time()
{
  // Set our idle time at an example-friendly 3 seconds.  A real
  // pool would return some fraction of the server's connection
  // idle timeout instead.
  return 30000;
}

}

