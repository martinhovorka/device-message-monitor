#include "RestAPI.hpp"
#include "../storage/DataStorage.hpp"

RestAPI *RestAPI::thisApi;

////////////////////////////////////////////////////////////////////////////////
RestAPI::RestAPI(const std::string &schema, const uint16_t port) : AbstractAPI(schema),
                                                                   port(port),
                                                                   settings(std::make_shared<restbed::Settings>()),
                                                                   resourcePost(std::make_shared<restbed::Resource>()),
                                                                   resourceGet(std::make_shared<restbed::Resource>())
{
    thisApi = this;
}

////////////////////////////////////////////////////////////////////////////////
bool RestAPI::setupApi()
try
{
    settings->set_port(port);
    settings->set_default_header("Connection", "close");

    // FIXME: only for demonstration purposes; need update
    resourcePost->set_path("/device/measurement");
    resourcePost->set_method_handler("POST", postHandler);

    resourceGet->set_path("/device/results");
    resourceGet->set_method_handler("GET", getHandler);

    service.publish(resourcePost);
    service.publish(resourceGet);

    return true;
}
catch (const std::exception &e)
{
    LOG_FMT_ERR("API setup exception: %s", e.what());
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void RestAPI::run()
try
{
    service.start(settings);
}
catch (const std::exception &e)
{
    LOG_FMT_ERR("API runtime exception: %s", e.what());
}

////////////////////////////////////////////////////////////////////////////////
bool RestAPI::shutdownApi()
try
{
    service.stop();
    return true;
}
catch (const std::exception &e)
{
    LOG_FMT_ERR("API shutdown exception: %s", e.what());
    return false;
}

////////////////////////////////////////////////////////////////////////////////
RestAPI::~RestAPI()
{
}

////////////////////////////////////////////////////////////////////////////////
void RestAPI::postHandler(const std::shared_ptr<restbed::Session> session)
{
    const auto request = session->get_request();

    size_t contentLength;
    request->get_header("Content-Length", contentLength);

    LOG_FMT_DBG("received %u bytes @ POST %s", contentLength, request->get_path().c_str());
    
    session->fetch(contentLength, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes &body)
                   {
                       const std::string buffer((char *)body.data(), body.size());
                       pJsonMessage_t inMessage(std::make_shared<rapidjson::Document>());

                       if (inMessage->Parse(buffer.c_str()).HasParseError())
                       {
                           session->close(restbed::BAD_REQUEST);
                           LOG_FMT_ERR("invalid JSON format; offset: %d; message: %s", inMessage->GetErrorOffset(), buffer.c_str());
                           return;
                       }

                       if (!thisApi->isValidJSON(*inMessage))
                       {
                           session->close(restbed::BAD_REQUEST);
                           LOG_FMT_ERR("invalid JSON message; %s", buffer.c_str());
                           return;
                       }

                       if (!thisApi->pushNewMessage(inMessage))
                       {
                           session->close(restbed::INTERNAL_SERVER_ERROR);
                           return;
                       }

                       session->close(restbed::OK);
                   });
}

////////////////////////////////////////////////////////////////////////////////
void RestAPI::getHandler(const std::shared_ptr<restbed::Session> session)
{
    const std::string &results(DataStorage::getResults());
    session->close(restbed::OK, results);
}