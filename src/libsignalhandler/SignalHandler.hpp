#ifndef SIGNALHANDLER_HPP
#define SIGNALHANDLER_HPP

#include <csignal>
#include <map>
#include <stack>
#include <stdexcept>

namespace sighandler
{

    void throwSystemErrorException(const int signum);

    class SignalHandler final
    {
    public:
        SignalHandler(const SignalHandler &) = delete;
        SignalHandler &operator=(const SignalHandler &) = delete;

        static const int MIN_SIG_ID;
        static const int MAX_SIG_ID;

        /**
         * @brief get singleton instance
         *
         * @return SignalHandler&
         */
        static SignalHandler &get();

        /**
         * @brief set neẃ signal handler saving the previoius one
         *
         * @param signalNumber signal id
         * @param newHandler handler functionpushHandler(signalNumber, handler)
         * @return int 0 on success; value < 0 on failure
         */
        int pushHandler(const int signalNumber, const sighandler_t newHandler);

        /**
         * @brief remove actual siignal handler and restore the previous
         *
         * @param signalNumber signalo id
         * @return int 0 on success; value < 0 on failure
         */
        int popHandler(const int signalNumber);

        /**
         * @brief set the same handler for all signals
         *
         * @param handler handler function
         * @return int 0 on success; value < 0 on failure
         */
        int setAllHandlers(const sighandler_t handler = throwSystemErrorException);

        /**
         * @brief check if signal number is within correct boundaries and it is not unmaskable signals
         *
         * @param signalNumber signal id
         * @return int 0 opn success; value < 0 on failure
         */
        int isValidSignalNumber(const int signalNumber);

    private:
        /**
         * @brief Construct a new Signal Handler object
         *
         */
        SignalHandler();

        /**
         * @brief Destroy the Signal Handler object
         *
         */
        ~SignalHandler();

        std::map<int, std::stack<sighandler_t>> handlers;
    };
}

#endif
