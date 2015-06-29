#include "scoped_reply.h"

namespace redis {

ScopedReply::ScopedReply(redisReply * reply) : reply_(reply) {
}

ScopedReply::~ScopedReply() {
  if (reply_) {
    freeReplyObject(reply_);
  }
}

}

