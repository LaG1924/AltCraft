#include "core/Core.hpp"
#include "gamestate/Game.hpp"

INITIALIZE_EASYLOGGINGPP

int main(){
    el::Configurations loggerConfiguration;
    loggerConfiguration.set(el::Level::Info,el::ConfigurationType::Format,"%level: %msg");
    el::Loggers::reconfigureAllLoggers(loggerConfiguration);
    Core core;
    core.Exec();
}