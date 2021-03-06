#include "Event.hpp"
#include "Utility.hpp"
#include "Game.hpp"

#include <set>

#include <SDL.h>
#include <easylogging++.h>

const char *getTimeSinceProgramStart(void) {
    static auto initialTime = std::chrono::steady_clock().now();
    auto now = std::chrono::steady_clock().now();
    std::chrono::duration<double> seconds = now - initialTime;
    static char buffer[30];
    sprintf(buffer, "%.2f", seconds.count());
    return buffer;
}

INITIALIZE_EASYLOGGINGPP

void initLogger() {
    el::Configurations loggerConfiguration;
    el::Helpers::installCustomFormatSpecifier(el::CustomFormatSpecifier("%startTime", std::bind(getTimeSinceProgramStart)));
    std::string format = "[%startTime][%level][%thread][%fbase]: %msg";
    loggerConfiguration.set(el::Level::Info, el::ConfigurationType::Format, format);
    loggerConfiguration.set(el::Level::Error, el::ConfigurationType::Format, format);
    loggerConfiguration.set(el::Level::Fatal, el::ConfigurationType::Format, format);
    loggerConfiguration.set(el::Level::Warning, el::ConfigurationType::Format, format);
    el::Helpers::setThreadName("Render");
    el::Loggers::reconfigureAllLoggers(loggerConfiguration);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    LOG(INFO) << "Logger is configured";    
}

int main(int argc, char** argv) {
    srand(time(0));
    initLogger();

    try {
        if (SDL_Init(0) == -1)
            throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
    } catch (std::exception& e) {
        LOG(ERROR) << e.what();
        return -1;
    }
    
	RunGame();
	
	return 0;
}