project(${TARGET})

add_library(
    ${TARGET} STATIC
    ${TARGET_SRCS}
)

target_include_directories(
    ${TARGET}
    PUBLIC
    .
)

if(DEFINED TARGET_LIBS)
    target_link_libraries(
        ${TARGET}
        ${TARGET_LIBS}
    )
endif(DEFINED TARGET_LIBS)

if(DEFINED TARGET_DEPENDS)
    add_dependencies(
        ${TARGET}
        ${TARGET_DEPENDS}
    )
endif(DEFINED TARGET_DEPENDS)
