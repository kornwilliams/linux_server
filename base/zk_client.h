#ifndef _BASE_ZK_CLIENT_H_
#define _BASE_ZK_CLIENT_H_

#include <zookeeper.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "zk_event_listener.h"

namespace base {

class ZookeeperClient {
 public:
  enum ERROR_CODE {
    E_NULL_HANDLE = -1998,
    E_INIT_FAIL   = -1999
  };
  ZookeeperClient(const char * root_path) : zhandle_(NULL), root_path_(root_path) {}

  ~ZookeeperClient() {
    if (zhandle_) {
      zookeeper_close(zhandle_);
    }
  }

  int Create(const char * path, const std::string & value, bool ephemeral);
  int Delete(const char * path, bool ephemeral, int version = -1);
  int SetValue(const char * path, const std::string & value, int version = -1);

  int GetValue(const char * path, std::string * value, struct Stat * stat = NULL) const {
    return DoGetValue(path, value, stat, false);
  }
  int GetChildren(const char * path, std::vector<std::string> * children) const {
    return DoGetChildren(path, children, false);
  }

  int AddNodeListener(const char * path, ZkEventListenerPtr listener);
  int AddChildListener(const char * path, ZkEventListenerPtr listener);
  int AddSessionEventListener(ZkEventListenerPtr listener);
  
  int Init(int timeout = 500);
 private:
  zhandle_t * zhandle_;
  std::string root_path_;

  static void EventWatcher(zhandle_t *zzh, int type, int state, 
      const char *path, void *watcher_ctx);

  int DoGetValue(const char * path, std::string * value, struct Stat * stat, bool watch = true) const;
  int DoGetChildren(const char * path, std::vector<std::string> * children, bool watch = true) const;

  void TriggerNodeListeners(const char * path, const std::string & data) const;
  void TriggerChildListeners(const char * path, const std::vector<std::string> & children) const;
  void TriggerSessionEventListeners(int state) const;

  void OnNodeEvent(const char * path, int state);
  void OnChildEvent(const char * path, int state);
  void OnSessionEvent(const char * path, int state);

  struct Listeners {
    Listeners() :  watching(false) {}
    mutable bool watching;
    std::set<ZkEventListenerPtr> listeners;
  };

  std::map<std::string, Listeners> node_listeners_;
  // boost::mutex node_listener_mutex_;
  mutable boost::shared_mutex node_listener_mutex_;

  std::map<std::string, Listeners> child_listeners_;
  mutable boost::shared_mutex child_listener_mutex_;

  std::set<ZkEventListenerPtr> session_event_listeners_;
  mutable boost::mutex session_event_listeners_mutex_;

  std::map<std::string, std::string> ephemeral_nodes_;
  boost::mutex ephemeral_nodes_mutex_;
};

}

#endif // _BASE_ZK_CLIENT_H_

