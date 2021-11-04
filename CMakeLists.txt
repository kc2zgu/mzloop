cmake_minimum_required(VERSION 3.6)
project(mzloop CXX)

find_package(PkgConfig)
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET jsoncpp)
pkg_check_modules(MOSQ REQUIRED IMPORTED_TARGET libmosquittopp)
find_package(Boost REQUIRED COMPONENTS program_options)

set(CORE_SRC "Log.cc" "Loop.cc" "Zone.cc" "LeafZone.cc" "CompositeZone.cc"
  "MqttAgent.cc" "SensorMqtt.cc" "OutputMqtt.cc" "Schedule.cc")

set(CMAKE_CXX_FLAGS "-std=gnu++17")

include_directories(${JSON_INCLUDES})
include_directories(${Boost_INCLUDE_DIRS})

add_library("mzloop-core" STATIC ${CORE_SRC})
target_link_libraries("mzloop-core" PkgConfig::JSON PkgConfig::MOSQ ${Boost_LIBRARIES})

add_executable("mzloop" "mzloop.cc")
target_link_libraries("mzloop" "mzloop-core")

add_executable("mzsched" "mzsched.cc")
target_link_libraries("mzsched" "mzloop-core")
