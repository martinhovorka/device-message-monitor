set(
    TARGET
    device-monitor
)

set(
    TARGET_SRCS
    deviceMonitor.cpp
    apis/AbstractAPI.cpp
    apis/RestAPI.cpp
)

set(
    TARGET_LIBS
    restbed
    fnv
    logger
    signalhandler
)

include(${TEMPLATE_BINARY})
