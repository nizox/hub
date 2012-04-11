project(tNETencule)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAME_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(${CMAKE_SYSTEM_NAME} True)


find_package(Event COMPONENTS core REQUIRED)
find_package(Tapcfg REQUIRED)

set(SOURCES_LIST hub.c tun.c
	main.c)

if (UNIX)
    include_directories(/usr/include/ /usr/local/include)
endif (UNIX)
include_directories(${TAPCFG_INCLUDE_DIR}
	${EVENT_INCLUDE_DIR})

add_definitions(-D${CMAKE_SYSTEM_NAME})

add_executable(tNET
	${SOURCES_LIST})

target_link_libraries(tNET
	${EVENT_LIBRARIES})

target_link_libraries(tNET
	${TAPCFG_LIBRARIES})
