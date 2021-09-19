#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "apis/AbstractAPI.hpp"
#include "apis/RestAPI.hpp"
#include "Logger.hpp"
#include <thread>

class Application final
{
public:
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    /**
     * @brief 
     * 
     * @return Application& 
     */
    static Application &get(void)
    {
        static Application app;
        return app;
    }

    /**
     * @brief 
     * 
     */
    int run(void)
    {
        LOG_MSG_INF("starting application main loop");

        if (api == nullptr)
        {
            LOG_MSG_FTL("api not initialized; unable to run application");
            return EXIT_FAILURE;
        }

        if (api->start() == true)
        {
            while (runApplication)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        else
        {
            LOG_MSG_FTL("failed to start api");
            return EXIT_FAILURE;
        }

        api->stop();

        if (api != nullptr)
        {
            delete api;
            api = nullptr;
        }
        return EXIT_SUCCESS;
    }

    /**
     * @brief 
     * 
     */
    void stop(void)
    {
        runApplication = false;
    }

private:
    /**
     * @brief Construct a new Application object
     * 
     */
    Application()
    {
        try
        {
            // TODO: we use only one API in this example; we should use much smarter solution
            api = new RestAPI("./etc/communication_schema/communication_schema_v1.json");
        }
        catch (const std::exception &e)
        {
            LOG_FMT_FTL("failed to create API; %s", e.what());
            api = nullptr;
        }
    }

    /**
     * @brief Destroy the Application object
     * 
     */
    ~Application()
    {
        if (api != nullptr)
        {
            delete api;
            api = nullptr;
        }
    }

    // TODO: we use only one API in this example; we should use much smarter solution
    AbstractAPI *api = nullptr;
    bool runApplication = true;
};

#endif