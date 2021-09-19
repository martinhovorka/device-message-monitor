project(${TARGET})

add_executable(
    ${TARGET}
    ${TARGET_SRCS}
)

install(
    TARGETS ${TARGET}
    DESTINATION ${INSTALL_PREFIX}
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
