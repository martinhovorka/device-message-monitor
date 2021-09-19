#include "SignalHandler.hpp"
#include <system_error>
#include <cstring>
#include <sstream>

namespace sighandler
{
    const int SignalHandler::MIN_SIG_ID(1);
    const int SignalHandler::MAX_SIG_ID(__SIGRTMIN);

    ///////////////////////////////////////////////////////////////////////
    void throwSystemErrorException(const int signum)
    {
        std::stringstream ss;
        ss << "program execution interrupted by signal: " << signum << " (" << strsignal(signum) << ')';
        throw std::system_error(std::error_code(EINTR, std::system_category()), ss.str());
    }

    ///////////////////////////////////////////////////////////////////////
    SignalHandler::SignalHandler()
    {
        for (int signalNumber(MIN_SIG_ID); signalNumber < MAX_SIG_ID; ++signalNumber)
        {
            if (isValidSignalNumber(signalNumber) != 0)
            {
                continue;
            }

            if (handlers.count(signalNumber) == 0)
            {
                handlers.insert(std::make_pair(signalNumber, std::stack<sighandler_t>()));
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////
    SignalHandler::~SignalHandler()
    {
    }

    ///////////////////////////////////////////////////////////////////////
    SignalHandler &SignalHandler::get()
    {
        static SignalHandler instance;
        return instance;
    }

    ///////////////////////////////////////////////////////////////////////
    int SignalHandler::pushHandler(const int signalNumber, const sighandler_t newHandler)
    {
        if ((isValidSignalNumber(signalNumber) != 0) || (newHandler == nullptr))
        {
            return -1;
        }

        sighandler_t previous = signal(signalNumber, newHandler);

        if (previous == SIG_ERR)
        {
            return -2;
        }

        handlers[signalNumber].push(previous);
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////
    int SignalHandler::popHandler(const int signalNumber)
    {
        if (isValidSignalNumber(signalNumber) != 0)
        {
            return -1;
        }

        if (handlers[signalNumber].empty())
        {
            return -2;
        }

        sighandler_t actual = signal(signalNumber, handlers[signalNumber].top());

        if (actual == SIG_ERR)
        {
            return -3;
        }

        handlers[signalNumber].pop();
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////
    int SignalHandler::setAllHandlers(const sighandler_t handler)
    {
        for (int signalNumber(MIN_SIG_ID); signalNumber < MAX_SIG_ID; ++signalNumber)
        {
            if (isValidSignalNumber(signalNumber) != 0)
            {
                continue;
            }

            int rc(pushHandler(signalNumber, handler));

            if (rc)
            {
                return rc;
            }
        }

        return 0;
    }

    ///////////////////////////////////////////////////////////////////////
    int SignalHandler::isValidSignalNumber(const int signalNumber)
    {
        if ((signalNumber < MIN_SIG_ID) || (signalNumber >= MAX_SIG_ID) || (signalNumber == SIGKILL) || (signalNumber == SIGSTOP))
        {
            return -1;
        }

        return 0;
    }
}
