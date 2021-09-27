cmake_minimum_required(VERSION 3.5)

project(embeddedrtps)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Add lwIP
  SET(LWIP_CONTRIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/contrib")
  SET(LWIP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lwip")

  SET(PORT_ROOT "${LWIP_CONTRIB_DIR}/ports/unix")
  SET(ARCH_DIR "${PORT_ROOT}/port/include")

  include(${LWIP_CONTRIB_DIR}/ports/CMakeCommon.cmake)

  set (LWIP_DEFINITIONS -DLWIP_DEBUG)
  set (LWIP_INCLUDE_DIRS
          "${LWIP_DIR}/src/include"
          "${LWIP_CONTRIB_DIR}"
          "${ARCH_DIR}"
          "${CMAKE_CURRENT_SOURCE_DIR}/lwip"
          "${WPDPACK_INCLUDE_DIR}"
          )

  include(${LWIP_DIR}/src/Filelists.cmake)
  include(${LWIP_CONTRIB_DIR}/Filelists.cmake)
  include(${PORT_ROOT}/Filelists.cmake)

  add_library(lwip STATIC ${lwipcore_SRCS} ${lwipcore3_SRCS} ${lwipcontribportunix_SRCS})

  target_include_directories(lwip PUBLIC ${LWIP_INCLUDE_DIRS})
  include_directories(lwip PUBLIC embeddedrtps/thirdparty/lwip/)

  target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS})
  target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS})

  target_link_libraries(lwip ${LWIP_SANITIZER_LIBS}  lwipcontribaddons lwipallapps lwipcore lwipcontribportunix)
endif()

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
)


add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})

ament_target_dependencies(${PROJECT_NAME} microcdr)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(${PROJECT_NAME} PRIVATE lwip microcdr)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lwip>"
    "$<INSTALL_INTERFACE:include>")

target_include_directories(${PROJECT_NAME} PRIVATE
    "$<BUILD_INTERFACE:${microcdr_INCLUDE_DIR}>"
  )

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