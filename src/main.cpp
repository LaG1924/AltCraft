#include "Event.hpp"
#include "Utility.hpp"
#include "ThreadGame.hpp"
#include "ThreadRender.hpp"
#include "ThreadNetwork.hpp"

#include <set>

const char *getTimeSinceProgramStart(void) {
    static auto initialTime = std::chrono::steady_clock().now();
    auto now = std::chrono::steady_clock().now();
    std::chrono::duration<double> seconds = now - initialTime;
    static char buffer[30];
    sprintf(buffer, "%.2f", seconds.count());
    return buffer;
}

INITIALIZE_EASYLOGGINGPP

#undef main

int main(int argc, char** argv) {
    srand(time(0));
	el::Configurations loggerConfiguration;
	el::Helpers::installCustomFormatSpecifier(
			el::CustomFormatSpecifier("%startTime", std::bind(getTimeSinceProgramStart)));
	std::string format = "[%startTime][%level][%thread][%fbase]: %msg";
	loggerConfiguration.set(el::Level::Info, el::ConfigurationType::Format, format);
	loggerConfiguration.set(el::Level::Error, el::ConfigurationType::Format, format);
	loggerConfiguration.set(el::Level::Fatal, el::ConfigurationType::Format, format);
	loggerConfiguration.set(el::Level::Warning, el::ConfigurationType::Format, format);
	el::Helpers::setThreadName("Render");
	el::Loggers::reconfigureAllLoggers(loggerConfiguration);
	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
	LOG(INFO) << "Logger is configured";

	LOG(WARNING) << "Sizeof EventData is " << sizeof(EventData);

    try {
        if (SDL_Init(0) == -1)
            throw std::runtime_error("SDL initialization failed: " + std::string(SDL_GetError()));
    } catch (std::exception& e) {
        LOG(ERROR) << e.what();
        return -1;
    }

	ThreadGame game;
	std::thread threadGame(&ThreadGame::Execute, game);

    ThreadNetwork network;
	std::thread threadNetwork(&ThreadNetwork::Execute, network);

    ThreadRender render;
	render.Execute();

	threadGame.join();
	threadNetwork.join();
	return 0;
}