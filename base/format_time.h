#ifndef _BASE_FORMAT_TIME_H_
#define _BASE_FORMAT_TIME_H_

#include <stdio.h>
#include <string>
#include <time.h>

namespace base {

std::string GmtTime(time_t t);
std::string FriendlyTime(time_t t);

}

#endif // _BASE_FORMAT_TIME_H_

