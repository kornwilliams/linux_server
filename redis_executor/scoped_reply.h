#ifndef _REDIS_SCOPED_REPLY_H_
#define _REDIS_SCOPED_REPLY_H_

#include <hiredis/hiredis.h>

namespace redis {

class ScopedReply
{
public:
  explicit ScopedReply(redisReply * reply);
  ~ScopedReply();

  redisReply * operator->() const { return reply_; }

  redisReply & operator*() const { return * reply_; }

  operator void*() const { return reply_; }

  operator bool() const { return reply_ != NULL; }

private:
  ScopedReply(const ScopedReply&);
  const ScopedReply& operator=(const ScopedReply&);

  redisReply * const reply_;
};

}

#endif // _REDIS_SCOPED_REPLY_H_

