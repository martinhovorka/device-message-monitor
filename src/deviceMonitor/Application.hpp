#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "apis/AbstractAPI.hpp"
#include "apis/RestAPI.hpp"
#include "middleware/MessageProcessor.hpp"
#include "storage/DataStorage.hpp"
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
    static Application &get(void);

    /**
     * @brief
     *
     */
    int run(void);

    /**
     * @brief
     *
     */
    void stop(void);

private:
    /**
     * @brief Construct a new Application object
     *
     */
    Application();

    /**
     * @brief Destroy the Application object
     *
     */
    ~Application();

    // TODO: we use only one API in this example; we should use much smarter solution
    AbstractAPI *api = nullptr;
    MessageProcessor *processor = nullptr;
    DataStorage storage;
    bool runApplication = true;
};

#endif