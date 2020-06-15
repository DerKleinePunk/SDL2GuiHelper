#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "OsmSoutLogger"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "osmsoutlogger.h"
#include "../easylogging/easylogging++.h"

namespace utils {
    
osmsoutlogger::osmsoutlogger(){
     el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
     debugDestination_.SetLevel(DEBUG);
     warnDestination_.SetLevel(WARN);
     infoDestination_.SetLevel(INFO);
     errorDestination_.SetLevel(ERROR);
}

osmsoutlogger::~osmsoutlogger(){
    
}

osmscout::Logger::Line osmsoutlogger::Log(Level level){
    if (level == DEBUG) {
        return Line(debugDestination_);
    }
    else if (level == WARN) {
      return Line(warnDestination_);
    }
    else if (level == INFO) {
      return Line(infoDestination_);
    }
    else {
      return Line(errorDestination_);
    }
}

osmsoutlogger::StringDestination::StringDestination(){
    line_ = "";
}

osmsoutlogger::StringDestination::~StringDestination(){
    
}

void osmsoutlogger::StringDestination::Print(const std::string& value){
    line_ += value;
}

void osmsoutlogger::StringDestination::Print(const char* value){
    line_ += value;
}

void osmsoutlogger::StringDestination::Print(bool value){
    line_ += (value ? "true" : "false");
}

void osmsoutlogger::StringDestination::Print(short value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::Print(unsigned short value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::Print(int value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::Print(unsigned int value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::Print(long value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::Print(unsigned long value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::Print(long long value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::Print(unsigned long long value){
    line_ += std::to_string(value);
}

void osmsoutlogger::StringDestination::PrintLn(){
    if(level_ == DEBUG){
        LOG(DEBUG) << line_;
    }
    else if(level_ == INFO){
        LOG(INFO) << line_;
    }
    else if(level_ == WARN){
        LOG(WARNING) << line_;
    }
    else{
        LOG(ERROR) << line_;
    }
    line_ = "";
}

void osmsoutlogger::StringDestination::SetLevel(Level level){
    level_ = level;
}

}//namespace utils
