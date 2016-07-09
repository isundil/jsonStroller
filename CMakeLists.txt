cmake_minimum_required(VERSION 2.8)

add_executable(jsonstroll src/main.cpp src/jsonContainer.cpp src/params.cpp src/curseOutput.cpp src/streamConsumer.cpp src/jsonArray.cpp src/jsonObjectEntry.cpp src/jsonObject.cpp src/jsonElement.cpp src/jsonPrimitive.cpp src/jsonException.cpp)
add_executable(json_test test/src/main.cpp src/jsonContainer.cpp src/params.cpp src/streamConsumer.cpp src/jsonArray.cpp src/jsonObjectEntry.cpp src/jsonObject.cpp src/jsonElement.cpp src/jsonPrimitive.cpp src/jsonException.cpp)
add_executable(wrapped_test test/src/wrapped.cpp)

set_property(TARGET jsonstroll PROPERTY RUNTIME_OUTPUT_DIRECTORY bin)

add_definitions ("-Wall")
add_definitions ("-g3")

# Add ncurses
SET(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
target_link_libraries(jsonstroll ${ncurses++_LIBRARIES} ${CURSES_LIBRARIES})
include_directories(include ${CURSES_INCLUDE_DIRS})

enable_testing()
set_property(TARGET json_test PROPERTY RUNTIME_OUTPUT_DIRECTORY test)
add_test(json_test test/json_test)
set_property(TARGET wrapped_test PROPERTY RUNTIME_OUTPUT_DIRECTORY test)
add_test(wrapped_test test/wrapped_test)

