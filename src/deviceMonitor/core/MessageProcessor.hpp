#ifndef MESSAGEPROCESSOR_HPP
#define MESSAGEPROCESSOR_HPP

#include "../apis/AbstractAPI.hpp"
#include "DataStorage.hpp"
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

class MessageProcessor
{
public:
    /**
     * @brief Construct message processor
     *
     * @param api api from which will message processor read data
     */
    MessageProcessor(AbstractAPI *api);

    /**
     * @brief start message processor thread
     *
     * @return true on success
     * @return false on failure
     */
    bool start(void);

    /**
     * @brief stop message processor
     *
     */
    void stop(void);

    /**
     * @brief static method for API to notify message processor that new data are available
     *
     */
    static void notify();

    /**
     * @brief Get the Data Store object
     *
     * @return const DataStorage&
     */
    DataStorage &getDataStore();

private:
    /**
     * @brief thread body implementation
     *
     * @param thisProcessor
     */
    static void threadBody(MessageProcessor *thisProcessor);

    /**
     * @brief get flag that indicates
     *
     * @return true
     * @return false
     */
    bool getRunFlag();

    /**
     * @brief Set the Run Flag object
     *
     * @param value
     */
    void setRunFlag(const bool value);

    AbstractAPI *api;
    DataStorage ds;

    static std::mutex processLock;
    static std::condition_variable processCondition;

    std::mutex runFlagLock;
    bool runFlag = false;
    std::thread *processorThread;
};

#endif