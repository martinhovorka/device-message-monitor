#ifndef RESTAPI_HPP
#define RESTAPI_HPP

#include "AbstractAPI.hpp"
#include "Logger.hpp"
#include <cinttypes>
#include <memory>
#include <restbed>

class RestAPI : public AbstractAPI
{
public:
    /**
     * @brief Construct a new Rest API object
     *
     * @param schema JSON validation schema
     * @param port
     */
    RestAPI(const std::string &schema, const uint16_t port = 50000);

    /**
     * @brief Destroy the Rest API object
     *
     */
    virtual ~RestAPI();

protected:
    /**
     * @brief perform API setup
     *
     * @return true on success
     * @return false  on failure
     */
    virtual bool setupApi() override;

    /**
     * @brief perform API runtime
     *
     */
    virtual void run() override;

    /**
     * @brief perform API teardown
     *
     * @return true on success
     * @return false on failure
     */
    virtual bool shutdownApi() override;

    /**
     * @brief HTTP POST method handler
     *
     * @param session
     */
    static void postHandler(const std::shared_ptr<restbed::Session> session);

    /**
     * @brief  HTTP GET method handler
     *
     */
    static void getHandler(const std::shared_ptr<restbed::Session>);

private:
    const uint16_t port;
    std::shared_ptr<restbed::Settings> settings;
    std::shared_ptr<restbed::Resource> resourcePost;
    std::shared_ptr<restbed::Resource> resourceGet;
    restbed::Service service;

    // WARNING: hack - quick solution how to access public interface from static context
    static RestAPI *thisApi;
};

#endif
