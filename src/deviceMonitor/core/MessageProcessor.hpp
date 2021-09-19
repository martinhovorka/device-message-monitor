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
    MessageProcessor(AbstractAPI *api) : api(api)
    {
    }

    bool start(void)
    {
        try
        {
            setRunFlag(true);
            processorThread = new std::thread(threadBody, this);
        }
        catch (const std::exception &e)
        {
            setRunFlag(false);
            LOG_FMT_FTL("failed to allocate thread memory: %s", e.what());
            return false;
        }

        return true;
    }

    void stop(void)
    {
        setRunFlag(false);

        notify();

        if (processorThread != nullptr)
        {
            processorThread->join();
            delete processorThread;
            processorThread = nullptr;
        }
    }

    static void notify()
    {
        std::unique_lock<std::mutex> lock(processLock);
        processCondition.notify_all();
    }

    static void threadBody(MessageProcessor *thisProcessor)
    {
        AbstractAPI::pJsonMessage_t message;

        while (thisProcessor->getRunFlag())
        {
            {
                std::unique_lock<std::mutex> lock(processLock);
                message = thisProcessor->getApi()->getNextMessage();
                if (message == nullptr)
                {
                    processCondition.wait(lock);
                    continue;
                }
            }

            rapidjson::OStreamWrapper osw(std::cout);
            rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
            message->Accept(writer);
        }
    }

    bool getRunFlag()
    {
        std::lock_guard<std::mutex> lock(runFlagLock);
        return runFlag;
    }

    void setRunFlag(const bool value)
    {
        std::lock_guard<std::mutex> lock(runFlagLock);
        runFlag = value;
    }

    AbstractAPI *getApi()
    {
        return api;
    }

private:
    AbstractAPI *api;
    DataStorage ds;

    static std::mutex processLock;
    static std::condition_variable processCondition;

    std::mutex runFlagLock;
    bool runFlag = false;
    std::thread *processorThread;
};

#endif