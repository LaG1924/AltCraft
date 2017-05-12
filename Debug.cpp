#include "Debug.hpp"

const bool isPrintSourceFile = false;
const debug::LogLevel debugPrintLevel = debug::LogLevel::INFO;

void ::debug::WriteLog(std::string message, debug::LogLevel level, debug::LogSource source, std::string sourceFile,
                       int lineInFile) {
    if (debugPrintLevel > level)
        return;
    std::string levelText;
    switch (level) {
        case FATAL:
            levelText = "FATAL";
            break;
        case ERROR:
            levelText = "ERROR";
            break;
        case WARNING:
            levelText = "WARNING";
            break;
        case INFO:
            levelText = "INFO";
            break;
        case DEBUG:
            levelText = "DEBUG";
            break;
    }
    std::string file = " " + sourceFile + ":" + std::to_string(lineInFile);
    std::cout << "[" << levelText << "]: " << message << (isPrintSourceFile ? file : "") << std::endl;
    if (level <= LogLevel::ERROR) {
        if (level <= LogLevel::FATAL)
            exit(-1);
        Exception exception;
        exception.message = message;
        exception.level = level;
        exception.source = source;
        throw exception;
    }
}