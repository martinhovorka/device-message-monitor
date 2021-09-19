#ifndef DATASTORAGE_HPP
#define DATASTORAGE_HPP

#include "../apis/AbstractAPI.hpp"
#include "fnv.hpp"
#include <map>
#include <cinttypes>

class DataStorage
{
public:
    typedef fnv::fnv64_t deviceId;
    typedef fnv::fnv64_t recordId;

    struct MeasurementRecord
    {
        std::string name;
        std::string unit;
        std::string time;
        uint64_t count;
        uint64_t faultCount;
        double min;
        double max;
    };

    struct DeviceRecord
    {
        std::string name;
        std::map<recordId, MeasurementRecord> measurements;
    };  

    //{"name":"device-1","timestamp":"2021-09-19T16:08:28.491790UTC","current":{"value":2.58,"unit":"A","fault":""}}

    void addRecord(AbstractAPI::pJsonMessage_t newRecord)
    {
        //static const std::string key_name("name");
        //static const std::string key_current("current");
        //static const std::string key_voltage("voltage");
        //static const std::string key_temperature("temperature");
        //static const std::string key_value("value");
        //static const std::string key_value("unit");
        //static const std::string key_value("string");
//
        //newRecord->GetString();
    }

private:
    std::map<deviceId, DeviceRecord> dataStore;
};

#endif