
#ifndef ABSTRACTAPI_HPP
#define ABSTRACTAPI_HPP

#include "Logger.hpp"
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/schema.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <string>
#include <thread>


class AbstractAPI
{
public:
    typedef std::shared_ptr<rapidjson::Document> pJsonMessage_t;

    /**
     * @brief Construct a new Abstract API object
     * 
     * @param schema json schema for current API
     */
    AbstractAPI(const std::string &schema) : jsonSchema(schema)
    {
        loadJSONSchema();
    }

    /**
     * @brief Destroy the Abstract API object
     * 
     */
    virtual ~AbstractAPI(void)
    {
    }

    /**
     * @brief common implementation of API start procedure
     * 
     * @return true API started successfully
     * @return false if API did not start
     */
    bool start(void)
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

    /**
     * @brief stop API and deallocate resources
     * 
     */
    void stop(void)
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

    /**
     * @brief Get the next message in message queue
     * 
     * @return pJsonMessage_t or nullptr of message is empty
     */
    pJsonMessage_t getNextMessage(void)
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

    /**
     * @brief checks if json document/message is valid by give JSON schema
     * 
     * @param document checked JSON
     * @return true if valid
     * @return false if not valid and/or schema is not loaded
     */
    bool isValidJSON(rapidjson::Document &document)
    {
        if (!validatorInitialized)
        {
            LOG_MSG_ERR("JSON validator not initialized; unable to check JSON message");
            return false;
        }

        return document.Accept(*pSchemaValidator);
    }

    /**
     * @brief add newly received message to queue
     * 
     * @param newMessage newly received message
     * @return true if pushed successfully
     * @return false on error
     */
    bool pushNewMessage(pJsonMessage_t newMessage)
    try
    {
        std::lock_guard<std::mutex> lock(queueLock);
        messageQueue.push(newMessage);
        return true;
    }
    catch (std::exception &ex)
    {
        LOG_FMT_ERR("unable to push new message to queue; error %s", ex.what());
        return false;
    }

protected:
    /**
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    virtual bool setupApi(void) = 0;

    /**
     * @brief 
     * 
     */
    virtual bool shutdownApi(void) = 0;

    /**
     * @brief 
     * 
     */
    virtual void run(void) = 0;

private:
    /**
     * @brief loads json schema for validation on given API
     * 
     */
    void loadJSONSchema(void)
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

    /**
     * @brief 
     * 
     * @param thisApi 
     */
    static void threadBody(AbstractAPI *thisApi)
    {
        while (thisApi->getRunFlag())
        {
            thisApi->run();
        }
    }

    /**
     * @brief Get the Run Flag object
     * 
     * @return true 
     * @return false 
     */
    bool getRunFlag()
    {
        std::lock_guard<std::mutex> lock(runFlagLock);
        return runFlag;
    }

    /**
     * @brief Set the Run Flag object
     * 
     * @param value 
     * @return true 
     * @return false 
     */
    void setRunFlag(const bool value)
    {
        std::lock_guard<std::mutex> lock(runFlagLock);
        runFlag = value;
    }

    const std::string jsonSchema;
    bool validatorInitialized = false;
    std::unique_ptr<rapidjson::SchemaDocument> pSchemaDocument;
    std::unique_ptr<rapidjson::SchemaValidator> pSchemaValidator;

    std::mutex queueLock;
    std::queue<pJsonMessage_t> messageQueue;

    std::mutex runFlagLock;
    bool runFlag = false;
    std::thread *apiThread;
};

#endif