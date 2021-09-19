#include "RestAPI.hpp"

RestAPI *RestAPI::thisApi;

////////////////////////////////////////////////////////////////////////////////
RestAPI::RestAPI(const std::string &schema, const uint16_t port) : AbstractAPI(schema),
                                                                   port(port),
                                                                   settings(std::make_shared<restbed::Settings>()),
                                                                   resource(std::make_shared<restbed::Resource>())
{
    thisApi = this;
}

////////////////////////////////////////////////////////////////////////////////
bool RestAPI::setupApi()
try
{

    settings->set_port(port);
    settings->set_default_header("Connection", "close");

    resource->set_path("/device/measurement");
    resource->set_method_handler("POST", postHandler);
    resource->set_method_handler("GET", getHandler);

    service.publish(resource);

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

                       // LOG_FMT_DBG("%s", buffer.c_str());

                       session->close(restbed::OK);
                   });
}

////////////////////////////////////////////////////////////////////////////////
void RestAPI::getHandler(const std::shared_ptr<restbed::Session>)
{
}