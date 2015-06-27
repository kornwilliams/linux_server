#include "zk_client.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <boost/algorithm/string.hpp>

#include "base/logging.h"

namespace base {

int ZookeeperClient::AddNodeListener(const char * path, ZkEventListenerPtr listener) {
  bool watched = false;
  LOG_DEBUG("ZkClient AddNodeListener path=" << path << " start.");

  {
    // boost::mutex::scoped_lock lock(node_listener_mutex_);
    boost::unique_lock<boost::shared_mutex> wlock(node_listener_mutex_);
    std::map<std::string, Listeners>::iterator it = node_listeners_.find(path);
    if (it == node_listeners_.end()) {
      node_listeners_[path].listeners.insert(listener);
      if (node_listeners_[path].watching) {
        watched = true;
      }
    } else {
      it->second.listeners.insert(listener);
      if (it->second.watching) {
        watched = true;
      }
    }
  }

  LOG_DEBUG("ZkClient AddNodeListener path=" << path << " watched=" << watched);

  if (!watched) {
    std::string value;
    LOG_DEBUG("ZkClient " << path << " start watch node.");
    return DoGetValue(path, &value, NULL, true);
  }
  LOG_DEBUG("ZkClient " << path << " already watch node.");
  return 0;
}

int ZookeeperClient::AddChildListener(const char * path, ZkEventListenerPtr listener) {
  bool watched = false;
  {
    // boost::mutex::scoped_lock lock(child_listener_mutex_);
    boost::unique_lock<boost::shared_mutex> wlock(child_listener_mutex_);
    std::map<std::string, Listeners>::iterator it = child_listeners_.find(path);
    if (it == child_listeners_.end()) {
      child_listeners_[path].listeners.insert(listener);
      if (child_listeners_[path].watching) {
        watched = true;
      }
    } else {
      it->second.listeners.insert(listener);
      if (it->second.watching) {
        watched = true;
      }
    }
  }

  LOG_DEBUG("ZkClient AddChildListener path=" << path << " watched=" << watched);

  if (!watched) {
    LOG_DEBUG("ZkClient " << path << " start watch children.");
    std::vector<std::string> children;
    return DoGetChildren(path, &children, true);
  }

  LOG_DEBUG("ZkClient " << path << " already watch children.");
  return 0;
}

int ZookeeperClient::AddSessionEventListener(ZkEventListenerPtr listener) {
  LOG_DEBUG("ZkClient AddSessionEventListener");
  {
    boost::mutex::scoped_lock lock(session_event_listeners_mutex_);
    session_event_listeners_.insert(listener);
  }
  return 0;
}

void ZookeeperClient::EventWatcher(zhandle_t *zzh, int type, int state, 
    const char *path, void * watcher_ctx) {
  ZookeeperClient * client = reinterpret_cast<ZookeeperClient *>(watcher_ctx);
  if (type == ZOO_SESSION_EVENT) {
    client->OnSessionEvent(path, state);
  } else if (type == ZOO_CHANGED_EVENT) {
    client->OnNodeEvent(path, state);
  } else if (type == ZOO_CHILD_EVENT) {
    client->OnChildEvent(path, state);
  } else if (type == ZOO_DELETED_EVENT) {
    // client->OnNodeEvent(path, state);
    // 该事件忽略, 所有的删除事件，一律交给父节点的ZOO_CHILD_EVENT处理
    LOG_INFO("ZkClient ZOO_DELETED_EVENT node=" << path << ",type=" << type 
        << ",state=" << state);
  } else {
    LOG_WARN("ZkClient unhandled child event : " << path << ",type=" << type 
        << ",state=" << state);
  }
}

void ZookeeperClient::TriggerNodeListeners(const char * path, const std::string & data) const {
  std::set<ZkEventListenerPtr> sl;

  {
    // boost::mutex::scoped_lock lock(node_listener_mutex_);
    boost::shared_lock<boost::shared_mutex> rlock(node_listener_mutex_);
    
    std::map<std::string, Listeners>::const_iterator it = node_listeners_.find(path);
    if (it != node_listeners_.end()) {
      it->second.watching = false;
      std::set<ZkEventListenerPtr>::const_iterator i = it->second.listeners.begin();
      while(i != it->second.listeners.end()) {
        boost::shared_ptr<ZkEventListener> sp = (*i).lock();
        if (sp) {
          LOG_DEBUG("ZkClient OnNodeEvent handled, path=" << path);
          sl.insert(sp);
          ++i;
        } else {
          LOG_WARN("ZkClient OnNodeEvent obsolete listener, path=" << path);
          // TODO : 需要清理
          // it->second.listeners.erase(i++);
          i++;
        }
      }
    } else {
      LOG_DEBUG("ZkClient no node listeners, node=" << path);
    }
  }
  LOG_DEBUG("Zk node " << path << " stop watching after trigger");

  std::set<ZkEventListenerPtr>::const_iterator i = sl.begin();
  while(i != sl.end()) {
    boost::shared_ptr<ZkEventListener> sp = (*i).lock();
    if (sp) {
      LOG_DEBUG("ZkClient OnNodeEvent handled, path=" << path);
      sp->HandleNodeEvent(path, data);
      ++i;
    }
  }
}

void ZookeeperClient::TriggerChildListeners(const char * path, const std::vector<std::string> & children) const {
  std::set<ZkEventListenerPtr> sl;

  {
    boost::shared_lock<boost::shared_mutex> rlock(child_listener_mutex_);
    
    std::map<std::string, Listeners>::const_iterator it = child_listeners_.find(path);
    if (it != child_listeners_.end()) {
      std::set<ZkEventListenerPtr>::const_iterator i = it->second.listeners.begin();
      while(i != it->second.listeners.end()) {
        boost::shared_ptr<ZkEventListener> sp = (*i).lock();
        if (sp) {
          LOG_DEBUG("ZkClient handle child event : " << path);
          // sp->HandleChildEvent(path, children);
          sl.insert(sp);
          ++i;
        } else {
          LOG_WARN("ZkClient null child listener " << path);
          // TODO : 需要清理
          // it->second.listeners.erase(i++);
          ++i;
        }
      }
    } else {
      LOG_DEBUG("ZkClient path "  << path << " has no child listener.");
    }
  }

  LOG_DEBUG("Zk children " << path << " stop watching after trigger");

  std::set<ZkEventListenerPtr>::const_iterator i = sl.begin();
  while(i != sl.end()) {
    boost::shared_ptr<ZkEventListener> sp = (*i).lock();
    if (sp) {
      LOG_DEBUG("ZkClient OnChildEvent handled, path=" << path);
      sp->HandleChildEvent(path, children);
      ++i;
    }
  }
}

void ZookeeperClient::TriggerSessionEventListeners(int state) const {
  std::set<ZkEventListenerPtr> sl;
  {
  boost::mutex::scoped_lock lock(session_event_listeners_mutex_);
  std::set<ZkEventListenerPtr>::const_iterator it = session_event_listeners_.begin();
  while(it != session_event_listeners_.end()) {
    boost::shared_ptr<ZkEventListener> sp = (*it).lock();
    if (sp) {
      LOG_DEBUG("ZkClient handle session event, state=" << state);
      // sp->HandleSessionEvent(state);
      sl.insert(sp);
      ++it;
    } else {
      LOG_WARN("ZkClient remove null session event listener, state=" << state);
      // TODO : 清理过期指针
      // session_event_listeners_.erase(it++);
      it++;
    }
  }
  }

  std::set<ZkEventListenerPtr>::const_iterator i = sl.begin();
  while(i != sl.end()) {
    boost::shared_ptr<ZkEventListener> sp = (*i).lock();
    if (sp) {
      LOG_DEBUG("ZkClient OnSessionEvent handled, state=" << state);
      sp->HandleSessionEvent(state);
      ++i;
    }
  }
}

// 触发情况：
// 1. 当前连接断掉
// 2. 连接断掉后转到其他服务
// 3. 连接断掉后重新恢复
// 4. expire? 未测
void ZookeeperClient::OnSessionEvent(const char * path, int state) {
  LOG_INFO("ZkClient OnSessionEvent state=" << state);

  if (state == ZOO_EXPIRED_SESSION_STATE) {
    LOG_INFO("ZkClient session expired.");
    if (zhandle_) {
      zookeeper_close(zhandle_);
      zhandle_ = NULL;
    }
    while(true) {
      // TODO: 在这里循环，是否会导致没有线程处理其他事件?
      if (Init() == 0) {
        LOG_WARN("ZkClient reconnect ok.");
        break;
      }
      sleep(1 + rand() % 3);
      LOG_WARN("ZkClient reconnect failed.");
    }
    LOG_INFO("ZkClient reconnect success on session expiration.");
  } else if(state == ZOO_CONNECTED_STATE) {
    LOG_DEBUG("ZkClient connected.");
  } else if(state == ZOO_CONNECTING_STATE) {
    LOG_DEBUG("ZkClient connecting.");
  } else {
    LOG_INFO("ZkClient ZOO_SESSION_EVENT unhandled state : " << state);
  }

  TriggerSessionEventListeners(state);

  // TODO : 重新开始监听session事件
}

void ZookeeperClient::OnChildEvent(const char * path, int state) {
  std::vector<std::string> children; 
  DoGetChildren(path, &children, false);
  LOG_INFO("ZkClient OnChildEvent path=" << path << " state=" << state << " child_count=" << children.size());
  TriggerChildListeners(path, children);
  
  LOG_DEBUG("Zk re-watch children of path " << path);
  DoGetChildren(path, &children, true);
}

void ZookeeperClient::OnNodeEvent(const char * path, int state) {
  std::string data;
  DoGetValue(path, &data, NULL, false);
  LOG_INFO("ZkClient ZOO_CHANGED_EVENT path=" << path << " data=" << data);
  TriggerNodeListeners(path, data);

  std::string data2;
  LOG_DEBUG("Zk re-watch node of path " << path);
  DoGetValue(path, &data2, NULL, true); // start listening
  if (data != data2) {
    // TODO : 数据变化漏通知的情况
    LOG_WARN("ZkClient OnNodeEvent path=" << path << " data mismatch.");
  }
}

int ZookeeperClient::Init(int timeout) {
  if (root_path_.empty()) {
    LOG_WARN("ZkClient root path catn\'t be empty!");
    return -1;
  }

  if (zhandle_) {
    zookeeper_close(zhandle_);
  }

  zhandle_ = zookeeper_init(root_path_.c_str(), &ZookeeperClient::EventWatcher, 1000, NULL, reinterpret_cast<void*>(this), 0);
  if (!zhandle_) {
    LOG_WARN("ZkClient connect null zhandle");
    return -2;
  }

  // timout = 5000 * 100 us = 0.5s
  int retry = 0;
  int usleep_interval = 100;
  int timeout_usec = timeout * 1000;
  while(true) {
    if (zoo_state(zhandle_) != ZOO_CONNECTING_STATE) {
      LOG_INFO("ZkClient connect ok, server=" << root_path_ << " retry=" << retry);
      break;
    }
    if (++retry * usleep_interval > timeout_usec) {
      LOG_WARN("ZkClient connect timeout " << timeout);
      return -3;
    }
    usleep(usleep_interval);
  }
  
  {
    // boost::mutex::scoped_lock lock(node_listener_mutex_);
    boost::shared_lock<boost::shared_mutex> rlock(node_listener_mutex_);
    std::map<std::string, Listeners>::iterator it = node_listeners_.begin();
    std::string str;
    while(it != node_listeners_.end()) {
      DoGetValue(it->first.c_str(), &str, NULL, true);
      ++it;
    }
  }

  {
    boost::shared_lock<boost::shared_mutex> rlock(child_listener_mutex_);
    std::map<std::string, Listeners>::iterator it = child_listeners_.begin();
    std::vector<std::string> str;
    while(it != child_listeners_.end()) {
      DoGetChildren(it->first.c_str(), &str, true);
      ++it;
    }
  }

  {
    boost::mutex::scoped_lock lock(ephemeral_nodes_mutex_);
    std::map<std::string, std::string>::iterator it = ephemeral_nodes_.begin();
    for(; it != ephemeral_nodes_.end(); ++it) {
      char res_path[256];
      int rc = zoo_create(zhandle_, it->first.c_str(), it->second.c_str(), it->second.size(), 
                          &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, res_path, 256);
      if (rc) {
        LOG_WARN("ZkClient recreate ephemeral error. path=" << it->first << " rc=" << rc << "," << zerror(rc));
      }
    }
  }

  return 0;
}

int ZookeeperClient::DoGetChildren(const char * path, std::vector<std::string> * children, bool watch) const {
  if (!zhandle_) {
    LOG_WARN("ZkClient null handle");
    return E_NULL_HANDLE;
  }

  struct String_vector str_vec;
  int rc = 0;
  if (watch) {
    rc = zoo_wget_children(zhandle_, path, &ZookeeperClient::EventWatcher, (void*)this, &str_vec);
    if (rc == 0) {
      boost::shared_lock<boost::shared_mutex> rlock(child_listener_mutex_);
      std::map<std::string, Listeners>::const_iterator it = child_listeners_.find(path);
      if (it != child_listeners_.end()) {
        it->second.watching = true;
      }
    }
  } else {
    rc = zoo_wget_children(zhandle_, path, NULL, NULL, &str_vec);
  }
  if (rc != 0) {
    LOG_WARN("ZkClient DoGetChildren zoo_wget_children() error. path=" << path << " rc=" << rc << "," << zerror(rc));
    return rc;
  }
  LOG_INFO("ZkClient DoGetChildren " << path << " child_count=" << str_vec.count);
  for(int i = 0; i < str_vec.count; ++i) {
    children->push_back(str_vec.data[i]);
  }
  deallocate_String_vector(&str_vec);
  return 0;
}

int ZookeeperClient::DoGetValue(const char * path, std::string * value, struct Stat * stat, bool watch) const {
  if (!zhandle_) {
    LOG_WARN("ZkClient null handle");
    return E_NULL_HANDLE;
  }

  static const int ZK_BUF_LEN = 2048;
  // TODO : 将get的结果缓存在本地
  Stat local_stat;
  if (stat == NULL) {
    stat = &local_stat;
  }
  char * zk_buf = new char[ZK_BUF_LEN];
  zk_buf[ZK_BUF_LEN - 1] = '\0';
  int buflen = ZK_BUF_LEN - 1;

  int rc = 0;
  if (watch) {
    // rc = zoo_wget(zhandle_, path, &ZookeeperClient::EventWatcher, reinterpret_cast<void*>(this), zk_buf, &buflen, stat);
    rc = zoo_wget(zhandle_, path, &ZookeeperClient::EventWatcher, (void*)this, zk_buf, &buflen, stat);
    if (rc == 0) {
      // boost::mutex::scoped_lock lock(node_listener_mutex_);
      boost::shared_lock<boost::shared_mutex> rlock(node_listener_mutex_);
      std::map<std::string, Listeners>::const_iterator it = node_listeners_.find(path);
      if (it != node_listeners_.end()) {
        it->second.watching = true;
      }
    }
  } else {
    rc = zoo_wget(zhandle_, path, NULL, NULL, zk_buf, &buflen, stat);
  }
  if (rc) {
    LOG_WARN("ZkClient zoo_wget() error. path=" << path << " rc=" << rc << "," << zerror(rc));
  } else {
    zk_buf[buflen] = '\0';
    *value = zk_buf;
  }

  delete [] zk_buf;

  return rc;
}

int ZookeeperClient::SetValue(const char * path, const std::string & value, int version) {
  if (!zhandle_) {
    LOG_WARN("ZkClient null handle");
    return E_NULL_HANDLE;
  }
  int rc = zoo_set(zhandle_, path, value.c_str(), value.size(), version);
  if (rc) {
    LOG_WARN("ZkClient zoo_set() error. path=" << path << " ver=" << version << " rc=" << zerror(rc));
  }
  LOG_DEBUG("ZkClient zoo_set() ok. path=" << path << " ver=" << version << " value=" << value);
  return rc;
}

int ZookeeperClient::Create(const char * path, const std::string & data, bool ephemeral) {
  if (!zhandle_) {
    LOG_WARN("ZkClient null handle");
    return E_NULL_HANDLE;
  }

  {
    boost::mutex::scoped_lock lock(ephemeral_nodes_mutex_);
    ephemeral_nodes_.insert(std::make_pair(path, data));;
  }

  char res_path[256];
  int rc = zoo_create(zhandle_, path, data.c_str(), data.size(), &ZOO_OPEN_ACL_UNSAFE, ephemeral ? ZOO_EPHEMERAL : 0, res_path, 256);
  if (rc) {
    LOG_WARN("ZkClient zoo_create() error. root=" << root_path_ << " path=" << path << " rc=" << rc << "," << zerror(rc));
  }

  return rc;
}

int ZookeeperClient::Delete(const char * path, bool ephemeral, int version) {
  if (!zhandle_) {
    LOG_WARN("ZkClient null handle");
    return E_NULL_HANDLE;
  }

  if (ephemeral) {
    boost::mutex::scoped_lock lock(ephemeral_nodes_mutex_);
    ephemeral_nodes_.erase(path);
  }

  int rc = zoo_delete(zhandle_, path, version);
  if (rc) {
    LOG_WARN("ZkClient zoo_delete() error. path=" << path << " v=" << version << " rc=" << zerror(rc));
  }
  return rc;
}

}

