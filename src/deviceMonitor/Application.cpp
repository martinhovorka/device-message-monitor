#include "Application.hpp"

////////////////////////////////////////////////////////////////////////////////
Application &Application::get(void)
{
    static Application app;
    return app;
}

////////////////////////////////////////////////////////////////////////////////
int Application::run(void)
{
    LOG_MSG_INF("starting application main loop");

    if ((api == nullptr) || (processor == nullptr))
    {
        LOG_MSG_FTL("api and/or processor not initialized; unable to run application");
        return EXIT_FAILURE;
    }

    bool apiStarted(api->start());

    bool processorStarted(false);

    if (apiStarted)
    {
        processorStarted = processor->start();
    }

    if (apiStarted && processorStarted)
    {
        while (runApplication)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
    else
    {
        LOG_MSG_FTL("failed to start api and/or message processor");
        return EXIT_FAILURE;
    }
    
    processor->stop();
    delete processor;
    processor = nullptr;

    api->stop();
    delete api;
    api = nullptr;

    return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
void Application::stop(void)
{
    LOG_MSG_INF("stopping main loop");
    runApplication = false;
}

////////////////////////////////////////////////////////////////////////////////
Application::Application()
{
    try
    {
        // TODO: we use only one API in this example; we should use much smarter solution
        api = new RestAPI("./etc/communication_schema/communication_schema_v1.json");
        processor = new MessageProcessor(api);
    }
    catch (const std::exception &e)
    {
        LOG_FMT_FTL("failed to create API; %s", e.what());
        api = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
Application::~Application()
{
    if (api != nullptr)
    {
        delete api;
        api = nullptr;
    }
}