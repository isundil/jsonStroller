cmake_minimum_required(VERSION 2.8)

add_executable(jsonstroll src/main.cpp src/params.cpp src/streamConsumer.cpp src/jsonArray.cpp src/jsonObject.cpp src/jsonElement.cpp src/jsonPrimitive.cpp src/jsonException.cpp)
add_executable(json_test test/src/main.cpp src/params.cpp src/streamConsumer.cpp src/jsonArray.cpp src/jsonObject.cpp src/jsonElement.cpp src/jsonPrimitive.cpp src/jsonException.cpp)

set_property(TARGET jsonstroll PROPERTY RUNTIME_OUTPUT_DIRECTORY bin)

add_definitions ("-Wall")
add_definitions ("-g3")

include_directories(include)

enable_testing()
set_property(TARGET json_test PROPERTY RUNTIME_OUTPUT_DIRECTORY test)
add_test(json_test test/json_test)

