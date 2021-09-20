#include "DataStorage.hpp"

std::mutex DataStorage::dataStoreLock;
std::map<DataStorage::deviceId, DataStorage::DeviceRecord> DataStorage::dataStore;
uint64_t DataStorage::totalCount(0);
const std::string DataStorage::key_name("name");
const std::string DataStorage::key_current("current");
const std::string DataStorage::key_voltage("voltage");
const std::string DataStorage::key_temperature("temperature");
const DataStorage::valueId DataStorage::id_current(fnv::Fnv64a(key_current));
const DataStorage::valueId DataStorage::id_voltage(fnv::Fnv64a(key_voltage));
const DataStorage::valueId DataStorage::id_temperature(fnv::Fnv64a(key_temperature));

////////////////////////////////////////////////////////////////////////////////
void DataStorage::addRecord(AbstractAPI::pJsonMessage_t newRecord)
try
{
    std::lock_guard<std::mutex> lock(dataStoreLock);
    totalCount++;

    // get name and hash from device name
    std::string name(newRecord->GetObject()[key_name.c_str()].GetString());
    deviceId idDevice(fnv::Fnv64a(name));

    // check if we have device registered if not create new record
    auto device(dataStore.find(idDevice));
    if (device == dataStore.end())
    {
        device = dataStore.insert(std::make_pair(idDevice, DeviceRecord(name))).first;
    }
    else
    {
        device->second.deviceMessageCount++;
    }

    addMeasurementRecord(newRecord, device, key_current, id_current);
    addMeasurementRecord(newRecord, device, key_voltage, id_voltage);
    addMeasurementRecord(newRecord, device, key_temperature, id_temperature);
}
catch (const std::exception &ex)
{
    LOG_FMT_ERR("unable to add new record to storage: %s", ex.what());
}
////////////////////////////////////////////////////////////////////////////////
void DataStorage::addMeasurementRecord(
    AbstractAPI::pJsonMessage_t newRecord,
    std::map<DataStorage::deviceId, DataStorage::DeviceRecord>::iterator device,
    const std::string &key,
    const valueId &id)
{
    // check if new message has measured value present
    if (newRecord->HasMember(key.c_str()))
    {
        // check if measured value is already registered
        auto value(device->second.measurements.find(id));
        if (value == device->second.measurements.end())
        {
            device->second.measurements.insert(std::make_pair(id, Value(key)));
        }
        else
        {
            value->second.measurementCount++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
std::string DataStorage::getResults()
{
    std::lock_guard<std::mutex> lock(dataStoreLock);
    std::stringstream ss;

    for (auto &device : dataStore)
    {
        ss << device.second.name << ':' << " deviceTotal: " << device.second.deviceMessageCount << "; ";

        for (auto &i : device.second.measurements)
        {
            ss << i.second.name << ": " << i.second.measurementCount << "; ";
        }

        ss << std::endl;
    }

    ss << "grandTotal: " << totalCount << std::endl;

    return ss.str();
}
