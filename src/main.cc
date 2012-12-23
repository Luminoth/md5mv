#include "pch.h"
#include <iostream>
#include <signal.h>
#include "common.h"
#include "ClientConfiguration.h"
#include "Engine.h"

boost::filesystem::path g_configfilename(client_conf());

void print_help()
{
    std::cerr << "Usage: md5model [options]" << std::endl << std::endl
        << "MD5 Model Loader" << std::endl << std::endl
        << "Options:" << std::endl
        << "\t-h, --help        show this help message and exit" << std::endl;
}

bool parse_arguments(int argc, char* const argv[])
{
#if defined WIN32
    std::cerr << "Implement parse_arguments()!" << std::endl;
#else
    static struct option long_options[] =
    {
        { "help", 0, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    while(true) {
        int c = getopt_long(argc, argv, "h", long_options, NULL);
        if(c == -1) break;

        switch(c)
        {
        case 'h':
        case '?':
            print_help();
            return false;
        }
    }
#endif

    return true;
}

bool initialize_config_directory()
{
    if(!boost::filesystem::exists(home_conf_dir())) {
        try {
            std::cout << "Creating home configuration directory " << home_conf_dir() << std::endl;
            boost::filesystem::create_directory(home_conf_dir());
        } catch(const boost::filesystem::filesystem_error& e) {
            std::cerr << "Could not create configuration directory: " << e.what() << std::endl;
            return false;
        }
    }

    return true;
}

bool initialize_configuration()
{
    std::cout << "Initializing configuration..." << std::endl;
    ClientConfiguration& config(ClientConfiguration::instance());

    std::cout << "Reading configuration from '" << g_configfilename << "'..." << std::endl;
    bool ret = config.load(g_configfilename);
    config.validate();

    return ret;
}

void save_configuration()
{
    std::cout << "Saving configuration to '" << g_configfilename << "'..." << std::endl;

    ClientConfiguration& config(ClientConfiguration::instance());
    config.save(g_configfilename);
}

bool initialize_logger()
{
    ClientConfiguration& config(ClientConfiguration::instance());
    if(!Logger::configure(config.logging_type(), config.logging_level(), config.logging_filename())) {
        return false;
    }

    Logger& logger(Logger::instance("md5mv.main"));
    LOG_INFO("Logger initialized!" << std::endl);

    return true;
}

void signal_handler(int signum)
{
    Logger& logger(Logger::instance("md5mv.main"));
    if(signum == SIGINT) {
        LOG_INFO("Caught SIGINT, quitting..." << std::endl);
        Engine::instance().shutdown();
    }
}

void initialize_signal_handlers()
{
    Logger& logger(Logger::instance("md5mv.main"));
    LOG_INFO("Initializing signal handlers..." << std::endl);

    signal(SIGINT, signal_handler);
}

int main(int argc, char* argv[])
{
    if(!parse_arguments(argc, argv)) {
        return 1;
    }

    if(!initialize_config_directory()) {
        return 1;
    }

    try {
        if(!initialize_configuration()) {
            std::cerr << "WARNING: Reading configuration failed!" << std::endl;
            save_configuration();
        }
    } catch(const ConfigurationError& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    ClientConfiguration& config(ClientConfiguration::instance());

    if(!initialize_logger()) {
        std::cerr << "ERROR: Could not initialize logger!" << std::endl;
        return 1;
    }

    Logger& logger(Logger::instance("md5mv.main"));
#if USE_SSE
    LOG_INFO("Using SSE" << std::endl);
#endif
    config.dump(logger);

    // initialize the signal handlers
    initialize_signal_handlers();

    if(!Engine::instance().init(argc, argv)) {
        Engine::instance().shutdown();
        return 1;
    }

    LOG_INFO("Entering event loop..." << std::endl);
    Engine::instance().run();

    Engine::instance().shutdown();
    save_configuration();
    return 0;
}
