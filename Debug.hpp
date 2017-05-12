#pragma once

#include <exception>
#include <queue>
#include <string>
#include <iostream>

namespace debug {

    enum LogLevel {
        FATAL = 0,
        ERROR = 5,
        WARNING = 10,
        INFO = 15,
        DEBUG = 20,
    };

    enum LogSource {
        Unknown,
        Display,
        Network,
        NetworkClient,
        Game,
        World,
        Field,
        Packet,
        AssetManager,
    };

    struct Exception : public std::exception {
        std::string message;
        LogLevel level;
        LogSource source;

        const char *what() const noexcept override{
            return this->message.c_str();
        }
    };

    void WriteLog(std::string message, LogLevel level = LogLevel::INFO, LogSource source = LogSource::Unknown,
                  std::string sourceFile = "", int lineInFile = 0);
#define LOG(MESSAGE, LEVEL, SOURCE) {using namespace debug; debug::WriteLog(MESSAGE,debug::LEVEL,debug::SOURCE,__FILE__,__LINE__);}
}