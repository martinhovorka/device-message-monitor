#include "AbstractAPI.hpp"
#include "../core/MessageProcessor.hpp"

////////////////////////////////////////////////////////////////////////////////
AbstractAPI::AbstractAPI(const std::string &schema) : jsonSchema(schema)
{
    loadJSONSchema();
}

////////////////////////////////////////////////////////////////////////////////
AbstractAPI::~AbstractAPI(void)
{
}

////////////////////////////////////////////////////////////////////////////////
bool AbstractAPI::start(void)
{
    if (!validatorInitialized)
    {
        LOG_MSG_ERR("unable to start API, JSON validator not initialized");
        return false;
    }

    if (!setupApi())
    {
        LOG_MSG_FTL("failed to setup API");
        return false;
    }

    try
    {
        setRunFlag(true);
        apiThread = new std::thread(threadBody, this);
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
void AbstractAPI::stop(void)
{
    setRunFlag(false);

    shutdownApi();

    if (apiThread != nullptr)
    {
        apiThread->join();
        delete apiThread;
        apiThread = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
AbstractAPI::pJsonMessage_t AbstractAPI::getNextMessage(void)
{
    std::lock_guard<std::mutex> lock(queueLock);

    if (messageQueue.empty())
    {
        return pJsonMessage_t(nullptr);
    }

    pJsonMessage_t messageToGet(messageQueue.front());
    messageQueue.pop();
    return messageToGet;
}

////////////////////////////////////////////////////////////////////////////////
bool AbstractAPI::isValidJSON(rapidjson::Document &document)
{
    if (!validatorInitialized)
    {
        LOG_MSG_ERR("JSON validator not initialized; unable to check JSON message");
        return false;
    }

    return document.Accept(*pSchemaValidator);
}

////////////////////////////////////////////////////////////////////////////////
bool AbstractAPI::pushNewMessage(pJsonMessage_t newMessage)
try
{
    std::lock_guard<std::mutex> lock(queueLock);
    messageQueue.push(newMessage);
    MessageProcessor::notify();
    return true;
}
catch (std::exception &ex)
{
    LOG_FMT_ERR("unable to push new message to queue; error %s", ex.what());
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void AbstractAPI::loadJSONSchema(void)
try
{
    // prepare input streams
    std::ifstream inputFileStream(jsonSchema);
    rapidjson::IStreamWrapper inputStreamWrapper(inputFileStream);

    // load JSON schema and validate for errors
    rapidjson::Document jsonDocument;
    if (jsonDocument.ParseStream(inputStreamWrapper).HasParseError())
    {
        LOG_FMT_FTL("invalid json schema; error %d; offset: %d", jsonDocument.GetParseError(), jsonDocument.GetErrorOffset());
        return;
    }

    // load schema document
    pSchemaDocument = std::unique_ptr<rapidjson::SchemaDocument>(new rapidjson::SchemaDocument(jsonDocument));

    // load schema validator
    pSchemaValidator = std::unique_ptr<rapidjson::SchemaValidator>(new rapidjson::SchemaValidator(*pSchemaDocument));
    validatorInitialized = true;
}
catch (const std::exception &ex)
{
    LOG_FMT_FTL("unable to load JSON schema", ex.what());
    validatorInitialized = false;
}

////////////////////////////////////////////////////////////////////////////////
void AbstractAPI::threadBody(AbstractAPI *thisApi)
{
    while (thisApi->getRunFlag())
    {
        thisApi->run();
    }
}

////////////////////////////////////////////////////////////////////////////////
bool AbstractAPI::getRunFlag()
{
    std::lock_guard<std::mutex> lock(runFlagLock);
    return runFlag;
}

////////////////////////////////////////////////////////////////////////////////
void AbstractAPI::setRunFlag(const bool value)
{
    std::lock_guard<std::mutex> lock(runFlagLock);
    runFlag = value;
}