cmake_minimum_required(VERSION 3.5.1)

SET(LIBNAME extio)
project(${LIBNAME})

include(./cmake/SetCommon.cmake)
add_definitions(-std=c++11)
add_definitions(-w)
# option
# option(SSL_TYPE "" OFF)
message(STATUS "[SSL_TYPE]: ${SSL_TYPE}")
add_definitions(-DSSL_TYPE=${SSL_TYPE})

option(SERVER_TYPE "" OFF)
message(STATUS "[SERVER_TYPE]: ${SERVER_TYPE}")
IF(SERVER_TYPE)
    add_definitions(-D${SERVER_TYPE}=1)
ENDIF()

option(STATIC_CALL "" OFF)
message(STATUS "[STATIC_CALL]: ${STATIC_CALL}")
IF(STATIC_CALL)
    add_definitions(-DSTATIC_CALL=1)
ENDIF()

option(USE_BUFFER "" OFF)
message(STATUS "[USE_BUFFER]: ${USE_BUFFER}")
IF(USE_BUFFER)
    add_definitions(-DUSE_BUFFER=1)
ENDIF()

option(MULTI_LOCKS "" OFF)
message(STATUS "[MULTI_LOCKS]: ${MULTI_LOCKS}")
IF(MULTI_LOCKS)
    add_definitions(-DMULTI_LOCKS=1)
ENDIF()

option(THREAD_COUNT "" OFF)
message(STATUS "[THREAD_COUNT]: ${THREAD_COUNT}")
IF(THREAD_COUNT)
    add_definitions(-DTHREAD_COUNT=${THREAD_COUNT})
ENDIF()

option(CLIENT_TYPE "" OFF)
message(STATUS "[CLIENT_TYPE]: ${CLIENT_TYPE}")
IF(CLIENT_TYPE)
    add_definitions(-D${CLIENT_TYPE}=1)
ENDIF()

# third party
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/rapidjson/include)

# threads
find_package(Threads REQUIRED)

SET(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN:$ORIGIN/lib")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

# IF(SSL_TYPE MATCHES 2)
#     # gmssl
#     set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
#     include_directories(${THIRD_PARTY_DIR}/gmssl/include)
#     link_directories(${THIRD_PARTY_DIR}/gmssl/lib)
#     link_libraries(tacrypto tassl)
# ENDIF()

# libraries
file(GLOB_RECURSE srcs core/src/*.cc core/src/*.cpp)
add_library(${LIBNAME} SHARED ${srcs})
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/include)

# ice
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
include_directories(${THIRD_PARTY_DIR}/ice/include)
include_directories(${THIRD_PARTY_DIR}/ice/include/generated)
# link_directories(${THIRD_PARTY_DIR}/ice/lib)

# Link local dynamic libraries
link_libraries(Ice Glacier2 IceGrid)
target_link_libraries(${LIBNAME} PUBLIC ${THIRD_PARTY_DIR}/ice/lib/libIce.so.37
    ${THIRD_PARTY_DIR}/ice/lib/libGlacier2.so.37 
    ${THIRD_PARTY_DIR}/ice/lib/libIceGrid.so.37
    pthread)

# Use to find the directory dependency path(libtacrypto, libtassl)
set_target_properties(${LIBNAME} PROPERTIES FOLDER ${LIBNAME}
    APPEND_STRING PROPERTY LINK_FLAGS "${ADD_LINK_LIB_FLAGS}")

##########################################################################################  
# targets pybind11
# add_subdirectory("./third_party/pybind11/" pybind11)
# pybind11_add_module(io_channel core/pybind11/io_channel.cc)
# link_libraries(${LIBNAME})
# target_link_libraries(io_channel PRIVATE ${LIBNAME})