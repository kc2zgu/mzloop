cmake_minimum_required(VERSION 3.6)
project(mzloop CXX)

find_package(PkgConfig)
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET jsoncpp)
pkg_check_modules(MOSQ REQUIRED IMPORTED_TARGET libmosquittopp)
find_package(Boost REQUIRED COMPONENTS program_options)

set(SRC "mzloop.cc" "Log.cc" "Loop.cc" "Zone.cc" "LeafZone.cc" "CompositeZone.cc"
  "MqttAgent.cc" "SensorMqtt.cc" "OutputMqtt.cc" "Schedule.cc")

set(CMAKE_CXX_FLAGS "-std=gnu++17")

add_executable("mzloop" ${SRC})
include_directories(${JSON_INCLUDES})
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries("mzloop" PkgConfig::JSON PkgConfig::MOSQ ${Boost_LIBRARIES})

add_executable("mzsched" "mzsched.cc" "Schedule.cc" "Log.cc")
