#include "MessageProcessor.hpp"

std::mutex MessageProcessor::processLock;
std::condition_variable MessageProcessor::processCondition;

////////////////////////////////////////////////////////////////////////////////
MessageProcessor::MessageProcessor(AbstractAPI *api) : api(api)
{
}

////////////////////////////////////////////////////////////////////////////////
bool MessageProcessor::start(void)
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

////////////////////////////////////////////////////////////////////////////////
void MessageProcessor::stop(void)
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

////////////////////////////////////////////////////////////////////////////////
void MessageProcessor::notify()
{
    std::unique_lock<std::mutex> lock(processLock);
    processCondition.notify_all();
}

////////////////////////////////////////////////////////////////////////////////
void MessageProcessor::threadBody(MessageProcessor *thisProcessor)
{
    AbstractAPI::pJsonMessage_t message;

    while (thisProcessor->getRunFlag())
    {
        {
            std::unique_lock<std::mutex> lock(processLock);
            message = thisProcessor->api->getNextMessage();
            if (message == nullptr)
            {
                processCondition.wait(lock);
                continue;
            }
        }

        thisProcessor->ds.addRecord(message);
    }
}

////////////////////////////////////////////////////////////////////////////////
bool MessageProcessor::getRunFlag()
{
    std::lock_guard<std::mutex> lock(runFlagLock);
    return runFlag;
}

////////////////////////////////////////////////////////////////////////////////
void MessageProcessor::setRunFlag(const bool value)
{
    std::lock_guard<std::mutex> lock(runFlagLock);
    runFlag = value;
}

////////////////////////////////////////////////////////////////////////////////
DataStorage &MessageProcessor::getDataStore()
{
    return ds;
}