cmake_minimum_required(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

PROJECT(ez_iot_os)
SET(USE_STATIC_LIB_LINKAGE ON)


#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")
ADD_DEFINITIONS("-DWITH_POSIX")

if(DEFINED PRECOMPILE_MACRO)
	ADD_DEFINITIONS("${PRECOMPILE_MACRO}")
endif()

if(DEFINED PLATFORM_C_FLAGS)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PLATFORM_C_FLAGS}")
else()
	SET(CMAKE_C_FLAGS "-O2 -g  -Wall")
endif()	

if(DEFINED PLATFORM_CXX_FLAGS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PLATFORM_CXX_FLAGS}")
else()
	SET(CMAKE_CXX_FLAGS "-O2 -g -Wall -fno-rtti -fno-exceptions ")
endif()

if (${RT_THREAD})
    ADD_DEFINITIONS("-D_RT_THREAD_")
    INCLUDE(${PROJECT_SOURCE_DIR}/../pro/rt-thread/private_CMakeLists.txt)
elseif(${ANDROID})
    ADD_DEFINITIONS("-D_ANDROID_")
else()
    #linux
endif()

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  link)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../../../components/mbedtls src_mbedtls)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../../../components/json    src_json)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../../../components/xml     src_xml)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../../../components/mqtt    mqtt)


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/
                    ${PROJECT_SOURCE_DIR}/../../../components
					${PROJECT_SOURCE_DIR}/../../../components/mbedtls
					${PROJECT_SOURCE_DIR}/../../../components/json
					${PROJECT_SOURCE_DIR}/../../../components/xml
					${PROJECT_SOURCE_DIR}/../../../components/mqtt
					 ${PROJECT_SOURCE_DIR}/../../../include
                    ${PROJECT_SOURCE_DIR}/../inc)

MESSAGE(STATUS "c compile option: ${CMAKE_C_FLAGS}")
MESSAGE(STATUS "c++ compile option: ${CMAKE_CXX_FLAGS}")

ADD_LIBRARY(ez_iot_STATIC STATIC ${link} ${src_mbedtls} ${src_json} ${src_xml} ${mqtt} )
SET_TARGET_PROPERTIES(ez_iot_STATIC PROPERTIES OUTPUT_NAME ez_iot)

IF (${DYNAMIC})
	ADD_LIBRARY(ez_iot_SHARED SHARED ${link} ${src_mbedtls} ${src_json} ${src_xml} ${mqtt})
	SET_TARGET_PROPERTIES(ez_iot_SHARED PROPERTIES OUTPUT_NAME ez_iot)
ENDIF (${DYNAMIC})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib/linux)
