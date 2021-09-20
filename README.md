# Device Monitoring

## Task

> The problem to solve is following:
>
>                You monitor devices, which are sending data to you.
>
>                Each device have a unique name.
>
>                Each device produces measurements.
>
>
>
>The challenge is:
>
>                Compute number of messages you got or read from the devices.
>
>
>
>The main outcome of your task will describe the concept / architecture of proposed solution (UML, documentation, work split, …).
>The scope is open, you must decide how the “devices” will work in your >system.
>The solution should be posted on GitHub or a similar page for a review.
>The solution should contain proof of concept (preferably in C++).
>Please add documentation explaining us how to run your code.

## Task expanded

- we monitor random number of "devices"
- devices sends us in random intervals "messages"
- each "message" contain device "name" and up to 3 measured values (and maybe some other values):
  - each device can send us "Voltage", "Current" and "Temperature"
  - not all "values" may be measured by device (one device can measure only Temperature, some other device can measure all of the values and some can none of them).
- we want to get statistics about:
  - total number messages we received
  - number of message per device
  - number of measurements of values per device

## Data generation and testing

For this purpose a simple Python script (device_simulator.py) was created. Script simulates set of devices for given amount of time and generates their messages. Messages are sent to the backend via REST API endpoint "POST /device/measurement". After given amount of time (i.e. after script generates all messages it can) it asks the backend for the statistics via REST API endpoint "GET /device/results" and prints them. Script itself keeps track of how many of what messages it sends, so it is possible to check if the backend calculates values correctly. **IMPORTANT:** if the script is executed several times without restart of backend, the backend will accumulate message counts from each script's execution.

## Backend description

### Overview

Backend is a multithreaded C++ application based around "producer-consumer" problem with REST API as a communication interface. It was chosen to use "top-down bottom-up layered" architecture for this task:

- I/O layer - for communication purposes and validation  
- Middleware layer - for additional processing (... in our task it just simply forwards received messages and stores it; other processing may be implemented)
- Data storage layer - for storing and querying data (... in our task only non-persistent in-memory storage; other storage methods may be implemented here)

### I/O Layer and communication APIs

Purpose is to *receive and validate* messages in JSON format. Our simulated devices will produce messages in JSON format that must correspond with defined JSON schema. Non-conforming messages are discarded. When message is received, it is inserted into queue and API notifies processing layer about new data. Processing layer will extract message from queue.

### Middleware and message processing

Middleware waits for notification about new messages from APIs. If there This layer processes received messages further. In our case it just forwards message to storage layer.

### Data storage

Data storage in our case is simple std::map<> in memory storage. As a key we use "name" of the device and "name" of value that is measured. All keys (names) are hashed by fast 64-bit non-cryptographic hash to speed up searching when adding new measurements. Data storage also provides simple interface for summary retrieval by REST API.

## Description of runtime

- backend starts (the "device-monitor")
  - REST API listens on port 50000 on localhost
- device simulator starts (the "device_simulator.py")
- device simulator will be generating messages in JSON format and send them to via POST method to endpoint "/device/measurement"
- on backend REST API will be receiving messages:
  - JSON message syntax is checked
  - JSON message content is checked against JSON schema
  - if both checks pass, new message is inserted into internal queue and middleware (message processor) is notified.
  - if message is rejected HTTP error code is returned to device simulator and message is discarded
- middleware/message processor extracts new messages from API queue and processes them further - in our case it only stores the message in the DataStorage (calculates hashes from names, counts etc...)
- when device simulator finishes generating data it requests summary of messages via REST API on GET /device/results endpoint and prints results
- then device simulator can start again **IMPORTANT:** if the simulator is executed several times without restart of backend, the backend will accumulate message counts from each script's execution.

## Prerequisites

- Debian 10 Buster
- needed packages:
  - g++
  - make
  - cmake
  - rapidjson-dev
  - librestbed-dev
  - python3
  - bash
  - git
  
## Compilation and execution

- clone repository

    ``` bash
    # clone repository
    git clone https://github.com/martinhovorka/device-message-monitor.git
    ```

- build and run backend - the "device-monitor"
     ``` bash
    # change directory to the project root
    cd device-message-monitor
    # execute 
    ./run_device_monitor.sh
    # ./run_device_monitor.sh will detect if requested binary is present and if not it will run compilation process
    ```

- run device simulator

    ``` bash
    # change directory to the project root
    cd device-message-monitor
    # execute 
    ./run_device_simulator.sh
    ```

- **NOTE**: all prerequisites must be met.
- **NOTE**: device-simulator must be run from the root of the project (path for the JSON schema is hardcoded, otherwise it wont be found); **the most convenient way is to use ./run_device_simulator.sh and ./run_device_monitor.sh scripts**

## Final notes

- all CMakeLists, libfnv, liblogger, libsignalhandler are reused from my previous projects
- program is missing configuration management - i.e. all hardcoded values should be made configurable (not implemented due time reasons)
