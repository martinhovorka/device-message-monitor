set(
    TARGET
    device-monitor
)

set(
    TARGET_SRCS
    main.cpp
    Application.cpp
    apis/AbstractAPI.cpp
    apis/RestAPI.cpp
    middleware/MessageProcessor.cpp
    storage/DataStorage.cpp
)

set(
    TARGET_LIBS
    restbed
    fnv
    logger
    signalhandler
)

include(${TEMPLATE_BINARY})
