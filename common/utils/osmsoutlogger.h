#pragma once

#include <osmscout/util/Logger.h>
namespace utils
{
    class osmsoutlogger : public osmscout::Logger
    {
        private:
            class StringDestination : public Destination
            {
            private:
                std::string line_;
                Level level_;
            public:
                void Print(const std::string& value);
                void Print(const std::string_view& value);
                void Print(const char* value);
                void Print(bool value);
                void Print(short value);
                void Print(unsigned short value);
                void Print(int value);
                void Print(unsigned int value);
                void Print(long value);
                void Print(unsigned long value);
                void Print(long long value);
                void Print(unsigned long long value);
                void PrintLn();
                StringDestination();
                ~StringDestination() override;
                void SetLevel(Level level);
            };
            StringDestination debugDestination_;
            StringDestination infoDestination_;
            StringDestination warnDestination_;
            StringDestination errorDestination_;
        public:
            osmsoutlogger();
            ~osmsoutlogger() override;
            
            Line Log(Level level);
    };
}
