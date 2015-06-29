#include "replication_locator.h"

#include <set>
#include <boost/lexical_cast.hpp>

#include "base/property_util.h"

namespace redis {

static bool UseMasterServer(const char * server_key) {
  if (*server_key == '/') {
    if (strncmp("/w", server_key, 2) == 0) {
      // 调用写server
      return true;
    } else if (strncmp("/r", server_key, 2) == 0) {
      // 调用读server
      return false;
    } else {
      LOG_WARN("unsupported server_key " << server_key);
      return true;
    }
  }

  // 根据命令内容来选定读/写server
  const char * p = server_key;
  std::string cmd_name;
  std::string cmd_key;
  for(; *p != '\0' && *p != ' '; ++p) {
    cmd_name += (char)toupper(*p);
  }
  if (*p == ' ') {
    ++p;
    for(; *p != '\0' && *p != ' '; ++p) {
      cmd_key += *p;
    }
  }
  
  static std::set<std::string> read_commands;
  if (read_commands.empty()) {
    const char * v[] = {"DUMP", "EXISTS", "GET", "GETBIT", "GETRANGE", "HEXISTS", "HGET", "HGETALL",
        "HKEYS", "HLEN", "HMGET", "HVALS", "LLEN", "LRANGE", "MGET", "PTTL", "SCARD", "SDIFF",
        "SINTER", "SMEMBERS", "SRANDMEMBER", "STRLEN", "TTL", "TYPE", "ZCARD", "ZCOUNT", "ZLEXCOUNT",
        "ZRANGE", "ZRANGEBYLEX", "ZRANGEBYSCORE", "ZRANK", "ZREVRANGE", "ZREVRANGEBYSCORE",
        "ZREVRANK", "ZSCORE", "SSCAN", "HSCAN", "ZSCAN"};

    for(size_t i = 0 ; i < sizeof(v)/sizeof(v[0]); ++i) {
      read_commands.insert(v[i]);
    }
  }

  return cmd_key.empty() || read_commands.find(cmd_name) == read_commands.end();
}

void ParseSlaveEndpoints(const std::string & slave_endpoints, 
			 std::map<std::string, int32_t> * res,
                         std::map<int32_t, std::string> * weight_section,
			 int32_t * total_weight)
{
  std::vector<std::string> v;

  std::map<std::string, std::string> props;
  base::ParseProperty(slave_endpoints, &props);

  for(std::map<std::string, std::string>::const_iterator it = props.begin(); it != props.end(); ++it) {
    try {
      res->insert(std::make_pair(it->first, boost::lexical_cast<int32_t>(it->second)));
      LOG_DEBUG("slave_endpoint " << it->first << " weight=" << it->second);
    } catch(...) {
      LOG_WARN("bad slave_endpoint " << it->first << " weight=" << it->second);
    }
  }

  (*total_weight) = 0;
  for(std::map<std::string, int32_t>::const_iterator it = res->begin(); it != res->end(); ++it) {
    if (it->second > 0) {
      (*total_weight) += it->second;
      weight_section->insert(std::make_pair(*total_weight, it->first));
    }
  }
}

std::string SelectSlaveEndpoint(const std::map<int32_t, std::string> & weight_section, const int32_t & total_weight) {
  if (weight_section.empty()) {
    return std::string();
  }

  return weight_section.upper_bound(rand() % total_weight)->second;
}

ReplicationLocator::ReplicationLocator(const std::string & master_endpoint,
      const std::string & slave_endpoints) : master_endpoint_(master_endpoint) {
  ParseSlaveEndpoints(slave_endpoints, &slave_endpoints_, &weight_section_, &total_weight_);
}

std::string ReplicationLocator::Locate(const char * server_key) {
  if (UseMasterServer(server_key)) {
    LOG_DEBUG("ReplicationLocator locate write key=" << server_key << ", use master_endpoint=" << master_endpoint_);
    return master_endpoint_;
  }

  // 选取读server
  std::string selected = SelectSlaveEndpoint(weight_section_, total_weight_);
  if (selected.empty()) {
    LOG_WARN("ReplicationLocator locate read key=" << server_key << " err, use master_endpoint=" << master_endpoint_);
    return master_endpoint_;
  }

  LOG_DEBUG("ReplicationLocator locate key=" << server_key << ", use slave_endpoint=" << selected);
  return selected;
}

}

