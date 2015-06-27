#include "logging.h"

namespace base {

void InitLogger(const std::string& name, 
                const std::string& path, 
                const std::string& loglevel,
                const std::string& pattern)
{
  log4cplus::SharedAppenderPtr appender(new log4cplus::DailyRollingFileAppender(path, log4cplus::HOURLY , true, 48));
  appender->setName(name);
  appender->setLayout( std::auto_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)) );
  log4cplus::Logger::getInstance(name).addAppender(appender);
  log4cplus::Logger::getInstance(name).setLogLevel(log4cplus::LogLevelManager().fromString(loglevel));
}

}

