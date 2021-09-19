#include <memory>
#include <cstdlib>
#include <restbed>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/schema.h>
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <queue>
#include <memory>

using namespace std;
//using namespace restbed;

void post_method_handler(const shared_ptr<restbed::Session> session)
{
    const auto request = session->get_request();

    size_t content_length;
    request->get_header("Content-Length", content_length);

    std::string body;
    request->get_body(body);

    session->fetch(content_length, [](const shared_ptr<restbed::Session> session, const restbed::Bytes &body)
                   {
                       fprintf(stdout, "%.*s\n", (int)body.size(), body.data());
                       session->close(restbed::OK, "Hello, World!", {{"Content-Length", "13"}});
                   });
}

void foo()
{
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/device/measurement");
    resource->set_method_handler("POST", post_method_handler);

    auto settings = make_shared<restbed::Settings>();
    settings->set_port(50000);
    settings->set_default_header("Connection", "close");
    // settings->set_bind_address("127.0.0.1")

    restbed::Service service;
    service.publish(resource);
    service.start(settings);
}

int main(const int, const char **)
{

    std::ifstream inputFileStream("./etc/communication_schema/communication_schema_v1.json");
    rapidjson::IStreamWrapper inputStreamWrapper(inputFileStream);

    rapidjson::Document schemaDocument;
    schemaDocument.ParseStream(inputStreamWrapper);
    if (schemaDocument.HasParseError())
    {
        std::cout << "invalid schema document" << std::endl;
    } 
    else
    {
        rapidjson::OStreamWrapper ostreamWrapper(std::cout);
        rapidjson::Writer<rapidjson::OStreamWrapper> coutWriter(ostreamWrapper);
        schemaDocument.Accept(coutWriter);
        std::cout << std::endl << "schema document valid" << std::endl;
    }
   
    rapidjson::SchemaDocument schema(schemaDocument);
    rapidjson::SchemaValidator validator(schema);
        
    rapidjson::Document d;
    d.Parse("{\"name\": \"not_specified\", \"timestamp\": \"1970-01-01T00:00:00.000UTC\"}");   

    if (d.HasParseError())
    {
        std::cout << "error document" << std::endl;
    }

    if (!d.Accept(validator))
    {
        std::cout << "invalid document" << std::endl;
    }

    //d.ParseStream(isw);

    // std::queue<std::shared_ptr<rapidjson::Document>> q;

    //if(doc.HasParseError())
    //{
    //    std::cout << doc.GetParseError() << std::endl;
    //    std::cout << doc.GetErrorOffset() << std::endl;
    //}

    // rapidjson::Document src;
    // std::make_shared<rapidjson::Document>()->Parse("[1,2,3]");
    // auto p1(std::make_shared<rapidjson::Document>());
    // auto p2(std::make_shared<rapidjson::Document>());
    // auto p3(std::make_shared<rapidjson::Document>());
    // p1->Parse("[1]");
    // p2->Parse("[1,2]");
    // p3->Parse("[1,2.3]");
    // q.push(p1);
    // q.push(p2);
    // q.push(p3);

    //rapidjson::Document dst(src.GetObject());

    // rapidjson::OStreamWrapper osw(std::cout);
    // rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
    // q.front()->Accept(writer);

    return EXIT_SUCCESS;
}
