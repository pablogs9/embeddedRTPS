cmake_minimum_required(VERSION 3.5)

project(embeddedrtps)

find_package(ament_cmake_ros REQUIRED)
find_package(microcdr REQUIRED)

set(${PROJECT_NAME}_sources
    src/discovery/TopicData.cpp
    src/discovery/ParticipantProxyData.cpp
    src/discovery/SEDPAgent.cpp
    src/discovery/SPDPAgent.cpp
    src/messages/MessageReceiver.cpp
    src/messages/MessageTypes.cpp
    src/ThreadPool.cpp
    src/communication/UdpDriver.cpp
    src/entities/Participant.cpp
    src/entities/Domain.cpp
    src/entities/StatelessReader.cpp
    src/storages/PBufWrapper.cpp
    src/storages/SimpleHistoryCache.cpp
    src/storages/HistoryCache.cpp
    src/rtps.cpp
    thirdparty/lwip/default_netif.c
    # thirdparty/Micro-CDR/src/c/types/sequence.c
    # thirdparty/Micro-CDR/src/c/types/basic.c
    # thirdparty/Micro-CDR/src/c/types/string.c
    # thirdparty/Micro-CDR/src/c/types/array.c
    # thirdparty/Micro-CDR/src/c/common.c
)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})

target_link_libraries(${PROJECT_NAME}
  microcdr
)

target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lwip>"
    "$<INSTALL_INTERFACE:include>")

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_targets(${PROJECT_NAME})

ament_export_include_directories(${microcdr_INCLUDE_DIR})

ament_package()

# Install includes.
install(
  DIRECTORY include/
  DESTINATION include
)

# Install library.
install(
  TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)