cmake_minimum_required(VERSION 2.8)

add_executable(jsonstroll src/main.cpp src/params.cpp src/streamConsumer.cpp src/jsonObject.cpp src/jsonElement.cpp src/jsonPrimitive.cpp src/jsonException.cpp)

set_property(TARGET jsonstroll PROPERTY RUNTIME_OUTPUT_DIRECTORY bin)

add_definitions ("-Wall")
add_definitions ("-g3")

include_directories(src include)

