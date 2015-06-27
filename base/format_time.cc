#include "format_time.h"

#include <sstream>

namespace base {

std::string GmtTime(time_t t) {
  static const std::string week[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  static const std::string month[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                                    "Aug", "Sep", "Oct", "Nov", "Dec" };
  struct tm tms;
  gmtime_r(&t, &tms);
  std::ostringstream res;
  // Expires: Mon, 26 Jul 1997 05:00:00 GMT
  res << week[tms.tm_wday] << ", ";
  if(tms.tm_mday < 10){
    res << 0;
  }
  res << tms.tm_mday << " "
      << month[tms.tm_mon] << " " << tms.tm_year + 1900 << " ";
  if (tms.tm_hour < 10) {
    res << 0;
  }
  res << tms.tm_hour << ":";

  if (tms.tm_min < 10) {
    res << 0;
  }
  res << tms.tm_min << ":";

  if (tms.tm_sec < 10) {
    res << 0;
  }
  res << tms.tm_sec << " GMT";
  return res.str();
}

std::string FriendlyTime(time_t t) {
  struct tm tms;
  localtime_r(&t, &tms);
  char buf[64] = {0};
  snprintf(buf, 63, "%d-%02d-%02d %02d:%02d:%02d", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday,
           tms.tm_hour, tms.tm_min, tms.tm_sec);
  return std::string(buf);
}

}

