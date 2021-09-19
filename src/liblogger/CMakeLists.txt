set(
    TARGET
    logger
)

set(
    TARGET_SRCS
    Logger.cpp
)

set(
    TARGET_LIBS
    pthread
)

include(${TEMPLATE_STATIC_LIBRARY})
