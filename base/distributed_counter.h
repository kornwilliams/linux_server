#ifndef _BASE_DISTRIBUTED_COUNTER_H_
#define _BASE_DISTRIBUTED_COUNTER_H_

#include <zookeeper.h>

#include <sstream>
#include <boost/lexical_cast.hpp>

#include "base/logging.h"
#include "base/zk_client.h"
#include "base/config_reader.h"

namespace base {


class DistributedCounter {
 public:
  DistributedCounter(const char * path) : path_(path),
      zk_client_(ConfigReader("../conf/zookeeper.conf").Get("zookeeper", "counters").c_str()) {
    zk_client_.Init();
  }

  virtual ~DistributedCounter() {
  }

  int Reset() {
    return ResetTo(0L);
  }

  int ResetTo(int64_t value) {
    std::stringstream new_value;
    new_value << value;
    return zk_client_.SetValue(path_.c_str(), new_value.str());
  }

  int64_t GetAndInc(int64_t incr) {
    struct Stat stat;
    std::string zk_buf;

    while(true) {
      int rc = zk_client_.GetValue(path_.c_str(), &zk_buf, &stat);

      if (rc != 0) {
        // TODO : 处理读错误
        LOG_WARN("zookeeper GetValue err, path_=" << path_);
        return -1;
      }

      int64_t current;
      try {
        current = boost::lexical_cast<int64_t>(zk_buf);
      } catch(boost::bad_lexical_cast & e) {
        LOG_WARN("zookeeper counter bad number : " << zk_buf << " path_=" << path_);
      }

      std::stringstream new_counter;
      new_counter << (current + incr);

      rc = zk_client_.SetValue(path_.c_str(), new_counter.str(), stat.version);
      if (rc == 0) {
        LOG_INFO("zookeeper counter ok, path_=" << path_ << " counter=" << current);
        return current;
      } else {
        LOG_WARN("zookeeper SetValue err, start retry.");
      }
    }
  }
 private:
  std::string path_;
  ZookeeperClient zk_client_;
};

}

#endif // _BASE_DISTRIBUTED_COUNTER_H_

