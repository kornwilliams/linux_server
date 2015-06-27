#ifndef _BASE_OBJECT_POOL_H_
#define _BASE_OBJECT_POOL_H_

#include <iostream>
#include <map>
#include <set>
#include <list>
#include <string>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "base/logging.h"

namespace base {

// 可以是任意client类型, 经ClientWrap封装后放在pool中
template <class ObjectType>
class ObjectPool {
 protected:
  virtual ObjectType * Create(const std::string & key) = 0;
  virtual void Destroy(ObjectType * obj) = 0;
 private:
  typedef std::map<std::string, std::set<ObjectType *> >  ActivePoolType;
  typedef std::map<ObjectType *, std::string> ObjectKeyMap;
  typedef std::map<std::string, std::list<ObjectType *> > IdlePoolType;

  // 注意是每个key对应的limit, 而不是总的limit
  size_t idle_min_;
  size_t idle_max_;
  size_t total_min_;
  size_t total_max_;

  IdlePoolType idle_pool_;
  boost::mutex idle_mutex_;

  ActivePoolType active_pool_;
  ObjectKeyMap active_object_keys_;
  boost::mutex active_mutex_;

  // 禁止对象拷贝
  ObjectPool(const ObjectPool&);
  ObjectPool& operator=(const ObjectPool&);

 public:
  ObjectPool(size_t idle_min, size_t idle_max, size_t total_min, size_t total_max) :
      idle_min_(idle_min),
      idle_max_(idle_max),
      total_min_(total_min),
      total_max_(total_max) {
  }

  virtual ~ObjectPool() {
  }

  //获得所对应的客户端
  ObjectType * Alloc(const std::string & key) {
    {
      boost::mutex::scoped_lock lock(active_mutex_);
      std::set<ObjectType *> & active_resources = active_pool_[key];
      if (active_resources.size() >= total_max_) {
        LOG_WARN("ObjectPool Alloc err, active obj limit " << total_max_ << " reached, key=" << key);
        return NULL;
      }
    }

    ObjectType * obj = NULL;
    {
      boost::mutex::scoped_lock lock(idle_mutex_);

      std::list<ObjectType*> & idle_resources = idle_pool_[key];
      
      // LOG_DEBUG("ObjectPool key=" << key << " idle_resources.size=" << idle_resources.size());
      if (!idle_resources.empty()) {
        obj = idle_resources.front();
        idle_resources.pop_front();
      }
    }

    if (!obj) {
      obj = Create(key);
    }

    if (!obj) {
      LOG_WARN("ObjectPool Create fail, key=" << key);
      return NULL;
    }

    size_t active_count = 0;

    {
      boost::mutex::scoped_lock lock(active_mutex_);
      std::set<ObjectType *> & active_resources = active_pool_[key];
      if ((active_resources.insert(obj)).second == false) {
        LOG_WARN("ObjectPool active item insert error: key=" << key
                  << " key_pool_size=" << active_resources.size()
                  << " duplicate=" << (active_resources.find(obj) != active_resources.end())
                  << " obj=" << obj);
      }
      active_object_keys_[obj] = key;
      active_count = active_resources.size();
    }
//  LOG_DEBUG("ObjectPool Alloc key=" << key << " obj=" << obj << " active_count=" << active_count);
    return obj;
  }

  // 释放客户端. Alloc()之后，务必调用Release().
  // recycle 表示是否回收

  void Release(ObjectType * obj, bool recycle) {
    std::string key;
    bool err = false;

    {
      boost::mutex::scoped_lock lock(active_mutex_);
      typename ObjectKeyMap::const_iterator it = active_object_keys_.find(obj);
      if (active_object_keys_.end() == it) {
        err = true;
        LOG_WARN("ObjectPool Release " << obj << " key not found.");
      } else {
        key = it->second;
      }
    }

    Release(key, obj, err ? false : recycle);
  }

  void Release(const std::string & key, ObjectType * obj, bool recycle) {
    if (obj == NULL) {
      return;
    }

    {
      boost::mutex::scoped_lock lock(active_mutex_);

      typename ActivePoolType::iterator it = active_pool_.find(key);
      int x = it->second.size();
      it->second.erase(obj);

      active_object_keys_.erase(obj);

      int y = it->second.size();
      if (x - y != 1) {
        LOG_WARN("ObjectPool remove " << obj << " from active pool err, key=" << key);
      }
    }

    if (recycle) {
      boost::mutex::scoped_lock lock(idle_mutex_);

      typename IdlePoolType::iterator it = idle_pool_.find(key);
      if (it != idle_pool_.end()) {
        std::list<ObjectType *> & idle_resources = it->second;
        
        if (idle_resources.size() < idle_max_) {
          idle_resources.push_back(obj); 
//        LOG_DEBUG("ObjectPool Release recycle key=" << key << " obj=" << obj << " idle_count=" << idle_resources.size());
        } else {
          Destroy(obj);
//        LOG_DEBUG("ObjectPool Release destroy key=" << key << " extra-obj=" << obj << " idle_count=" << idle_resources.size());
        }
      }
    } else {
      Destroy(obj);
//    LOG_DEBUG("ObjectPool Release destroy key=" << key << " bad-obj=" << obj);
    }
  }
};

}

#endif // _BASE_OBJECT_POOL_H_

