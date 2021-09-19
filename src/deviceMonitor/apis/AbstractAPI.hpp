
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
    AbstractAPI(const std::string &schema);

    /**
     * @brief Destroy the Abstract API object
     * 
     */
    virtual ~AbstractAPI(void);

    /**
     * @brief common implementation of API start procedure
     * 
     * @return true API started successfully
     * @return false if API did not start
     */
    bool start(void);

    /**
     * @brief stop API and deallocate resources
     * 
     */
    void stop(void);

    /**
     * @brief Get the next message in message queue
     * 
     * @return pJsonMessage_t or nullptr if queue is empty
     */
    pJsonMessage_t getNextMessage(void);

protected:
    /**
     * @brief checks if json document/message is valid by give JSON schema
     * 
     * @param document checked JSON
     * @return true if valid
     * @return false if not valid and/or schema is not loaded
     */
    bool isValidJSON(rapidjson::Document &document);

    /**
     * @brief add newly received message to queue
     * 
     * @param newMessage newly received message
     * @return true if pushed successfully
     * @return false on error
     */
    bool pushNewMessage(pJsonMessage_t newMessage);

    /**
     * @brief deviced APIs will implement setup procedures
     * 
     * @return true on success
     * @return false  on failure
     */
    virtual bool setupApi(void) = 0;

    /**
     * @brief derived APIs will implement shutdown procedures
     * 
     * @return true 
     * @return false 
     */
    virtual bool shutdownApi(void) = 0;

    /**
     * @brief derived API will implement its runtime
     * 
     */
    virtual void run(void) = 0;

private:
    /**
     * @brief loads json schema for validation on given API
     * 
     */
    void loadJSONSchema(void);

    /**
     * @brief thread body representation - a simple loop with no delay
     * 
     * @param thisApi 
     */
    static void threadBody(AbstractAPI *thisApi);

    /**
     * @brief safely returns flag if thread should continue to work
     * 
     * @return true thread should run
     * @return false thread should terminate
     */
    bool getRunFlag();

    /**
     * @brief safely sets flag if thread should continue to work
     * 
     * @param value flag value
     */
    void setRunFlag(const bool value);

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