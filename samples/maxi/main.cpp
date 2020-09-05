/**
* @file main.cpp
* @author Michael Nenninger
* @brief Mainentry
*/

#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "main"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#define SDL_MAIN_HANDLED
#include <iostream>
#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/commonutils.h"
#include "../../src/MiniKernel.h"
#include "SampleApp.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv)
{
    std::cout << "Starting Test App" << std::endl;
    START_EASYLOGGINGPP(argc, argv);
    if(utils::FileExists("logger.conf")) {
        // Load configuration from file
        el::Configurations conf("logger.conf");
        // Now all the loggers will use configuration from file and new loggers
        el::Loggers::setDefaultConfigurations(conf, true);
    }

    el::Helpers::setThreadName("Main");

    auto returncode = 0;
    try
	{
		std::cout << "Building Kernel" << std::endl;

		auto kernel = new MiniKernel();
		if (!kernel->StartUp(argc, argv)) {
			delete kernel;
			return -1;
		}

		std::cout << "Kernel is build" << std::endl;
        auto app = new SampleApp(kernel);
        app->Startup();
        kernel->Run();
        app->Shutdown();
        kernel->Shutdown();

		printf("all down deleting pointer\n");

		delete app;
		delete kernel;
	} catch(std::runtime_error const& exp) {
        LOG(ERROR) << exp.what();
        returncode = 1;
    }

    return returncode;
}