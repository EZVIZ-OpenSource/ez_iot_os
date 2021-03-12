#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

#项目名称
PROJECT(DEV_SDK_TEST)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -Wall -Wno-unused-variable")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -Wno-unused-variable")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(lib_ez_iot  -lez_iot)
ADD_DEFINITIONS("-DTEST_MICROKERNEL_ENABLE")

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  boot)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../../platform/wrapper/linux warpper)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../../platform/wrapper comm_def)

#########################################################添加依赖库######################################################
SET(lib_rt -lpthread -lm -lrt)

#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/
                    ${PROJECT_SOURCE_DIR}/../../inc
                    ${PROJECT_SOURCE_DIR}/../../include
                    ${PROJECT_SOURCE_DIR}/../../platform/wrapper/linux
                    ${PROJECT_SOURCE_DIR}/../../platform/wrapper
                    ${PROJECT_SOURCE_DIR}/../../eziot/core/link)
#添加依赖库目录
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../lib/gcc)

#添加需要编译的源文件
ADD_EXECUTABLE(device_reg_demo_bin ${boot} ${warpper} ${comm_def})

#添加链接库
target_link_libraries(device_reg_demo_bin
                        ${lib_ez_iot}
                        ${lib_rt})

SET_TARGET_PROPERTIES(device_reg_demo_bin PROPERTIES OUTPUT_NAME device_reg_demo)