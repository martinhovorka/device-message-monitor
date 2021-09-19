
#ifndef ABSTRACTAPI_HPP
#define ABSTRACTAPI_HPP

#include <string>

class AbstractAPI
{
    public:
        AbstractAPI(const std::string& schema): jsonSchema(schema)
        {}

        virtual ~AbstractAPI()
        {}
    private:
        const std::string jsonSchema;
};

#endif