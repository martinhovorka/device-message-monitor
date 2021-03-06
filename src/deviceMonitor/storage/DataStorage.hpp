#ifndef DATASTORAGE_HPP
#define DATASTORAGE_HPP

#include "../apis/AbstractAPI.hpp"
#include "fnv.hpp"
#include "Logger.hpp"
#include <cinttypes>
#include <iostream>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <mutex>

class DataStorage
{
public:
    // we use fast non-cryptographic hash for instead if string for device/measured value identification
    typedef fnv::fnv64_t deviceId;
    typedef fnv::fnv64_t valueId;

    struct Value
    {
        Value(const std::string &name) : name(name), measurementCount(1) {}
        std::string name;
        uint64_t measurementCount;
    };

    struct DeviceRecord
    {
        DeviceRecord(const std::string &name) : name(name), deviceMessageCount(1) {}
        std::string name;
        uint64_t deviceMessageCount;
        std::map<valueId, Value> measurements;
    };

    /**
     * @brief add new record to the datastore
     *
     * @param newRecord
     */
    static void addRecord(AbstractAPI::pJsonMessage_t newRecord);

    /**
     * @brief Get the Results object
     *
     * @return std::string
     */
    static std::string getResults();

private:
    /**
     * @brief add measurement record to the database
     *
     * @param newRecord message that was read from API
     * @param device iterator for device whose measurements should be updated
     * @param key JSON key
     * @param id hash of JSON key
     */
    static void addMeasurementRecord(
        AbstractAPI::pJsonMessage_t newRecord,
        std::map<DataStorage::deviceId, DataStorage::DeviceRecord>::iterator device,
        const std::string &key,
        const valueId &id);

    static const std::string key_name;
    static const std::string key_current;
    static const std::string key_voltage;
    static const std::string key_temperature;
    static const valueId id_current;
    static const valueId id_voltage;
    static const valueId id_temperature;

    static std::mutex dataStoreLock;
    static std::map<deviceId, DeviceRecord> dataStore;
    static uint64_t totalCount;
};

#endif
