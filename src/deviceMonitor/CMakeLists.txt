set(
    TARGET
    device-monitor
)

set(
    TARGET_SRCS
    deviceMonitor.cpp
    AbstractAPI.cpp
)

set(
    TARGET_LIBS
    restbed
    fnv
    logger
)

include(${TEMPLATE_BINARY})
