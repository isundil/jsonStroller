cmake_minimum_required(VERSION 2.8)

SET (CUSTOM_BINARY_OUTPUT_DIR bin)
SET (DOC_OUTPUT doc/)
add_definitions ("-Wall")
add_definitions ("-O3")

add_executable(jsonstroll
    src/main.cpp
    src/jsonContainer.cpp
    src/warning.cpp
    src/params.cpp
    src/curseOutput.cpp
    src/linearHistory.cpp
    src/outputFlag.cpp
    src/streamConsumer.cpp
    src/jsonArray.cpp
    src/jsonObjectEntry.cpp
    src/jsonObject.cpp
    src/jsonElement.cpp
    src/jsonPrimitive.cpp
    src/jsonException.cpp
    )

set_property(TARGET jsonstroll PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CUSTOM_BINARY_OUTPUT_DIR})

# Add ncurses
SET(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
target_link_libraries(jsonstroll ${ncurses++_LIBRARIES} ${CURSES_LIBRARIES})
include_directories(include ${CURSES_INCLUDE_DIRS})

# Add tests
enable_testing()
add_executable(json_test
    test/src/main.cpp

    src/jsonContainer.cpp
    src/warning.cpp
    src/linearHistory.cpp
    src/streamConsumer.cpp
    src/jsonArray.cpp
    src/jsonObjectEntry.cpp
    src/jsonObject.cpp
    src/jsonElement.cpp
    src/jsonPrimitive.cpp
    src/jsonException.cpp
    )
set_property(
    TARGET json_test
    PROPERTY RUNTIME_OUTPUT_DIRECTORY test
    )
add_executable(wrapped_test
    test/src/wrapped.cpp
    )
add_test(json_test test/json_test)

set_property(
    TARGET wrapped_test
    PROPERTY RUNTIME_OUTPUT_DIRECTORY test
    )
add_test(wrapped_test test/wrapped_test)

# Add manual page
find_program (HELP2MAN help2man)
add_custom_target(man ALL)
if (HELP2MAN)
    add_definitions(
        -DVERSIONDATE=__DATE__
        )
    add_custom_target(
        TARGET man ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_OUTPUT}
        )
    add_custom_command(
        TARGET man
        SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${CUSTOM_BINARY_OUTPUT_DIR}/jsonstroll
        COMMAND help2man ${CMAKE_CURRENT_BINARY_DIR}/${CUSTOM_BINARY_OUTPUT_DIR}/jsonstroll -o ${DOC_OUTPUT}/jsonstroll.1
        OUTPUTS ${DOC_OUTPUT}/jsonstroll.1
        )
    add_custom_command(
        TARGET man
        SOURCE man
        DEPENDS ${DOC_OUTPUT}/jsonstroll.1
        )
endif()

# Install
install(
    TARGETS jsonstroll
    DESTINATION bin
    )
install(
    FILES ${DOC_OUTPUT}/jsonstroll.1
    DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
    )

