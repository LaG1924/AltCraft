#include "core/Core.hpp"

#define ELPP_THREAD_SAFE
#define ELPP_FEATURE_CRASH_LOG
#define ELPP_DISABLE_LOGS
INITIALIZE_EASYLOGGINGPP

int main() {
    el::Configurations loggerConfiguration;
    std::string format = "[%level]{%fbase}: %msg";
    loggerConfiguration.set(el::Level::Info, el::ConfigurationType::Format, format);
    loggerConfiguration.set(el::Level::Error, el::ConfigurationType::Format, format);
    el::Loggers::reconfigureAllLoggers(loggerConfiguration);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    LOG(INFO) << "Logger is configured";

    Core core;
    core.Exec();

    return 0;
}