#include "Logger.hpp"
#include "apis/RestAPI.hpp"
#include "SignalHandler.hpp"
#include "Application.hpp"
#include <cstdlib>
#include <iostream>

/**
 * @brief
 *
 * @param signum
 */
void signalHandler(const int signum)
{
    LOG_FMT_INF("received termination signal %d", signum);
    Application::get().stop();
}

/**
 * @brief
 *
 */
void exitProcedure(void)
{
    logger::logDestroy_f();
}

/**
 * @brief
 *
 * @return int
 */
int initProcedure(void)
{
    logger::logInitialize_f(nullptr, logger::logDbg_e);

    if (sighandler::SignalHandler::get().setAllHandlers(signalHandler) != 0)
    {
        LOG_MSG_FTL("unable to register application termination handler");
        return EXIT_FAILURE;
    }

    if (sighandler::SignalHandler::get().pushHandler(SIGWINCH, SIG_IGN) != 0)
    {
        LOG_MSG_FTL("unable to set action for Window change signal");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(void)
try
{
    if (atexit(exitProcedure) != 0)
    {
        std::cerr << "failed to register exit procedure" << std::endl;
        return EXIT_FAILURE;
    }

    if (initProcedure() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    return Application::get().run();
}
catch (const std::exception &e)
{
    std::cerr << "unexpected exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
