# 顶层CMakeLists.txt

# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.16)

# 项目信息
SET(PRJ_NAME qjsMain)
project ($(PRJ_NAME))

SET(ROOT_DIR ..)
SET(CMAKE_DIR ./cmake/)

# 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE  -g -ggdb -std=gnu99")
set(CMAKE_C_COMPILER gcc)

add_definitions(-DCONFIG_VERSION=\"2020-07-05\")
add_definitions(-DCONFIG_BIGNUM)

list(APPEND PROJECT_SRC ${ROOT_DIR}/main.c)
list(APPEND PROJECT_SRC ${ROOT_DIR}/src/cModule.c)
list(APPEND PROJECT_SRC ${ROOT_DIR}/../cutils.c)
list(APPEND PROJECT_SRC ${ROOT_DIR}/../libregexp.c)
list(APPEND PROJECT_SRC ${ROOT_DIR}/../libunicode.c)
list(APPEND PROJECT_SRC ${ROOT_DIR}/../quickjs.c)
list(APPEND PROJECT_SRC ${ROOT_DIR}/../quickjs-libc.c)
list(APPEND PROJECT_SRC ${ROOT_DIR}/../libbf.c)
FILE(GLOB SRC_FILES ${ROOT_DIR}/runtime/*.c)
list(APPEND PROJECT_SRC ${SRC_FILES})


list(APPEND PROJECT_INCLUDE ${ROOT_DIR}/../)

include_directories(${PROJECT_INCLUDE})
include_directories(${ROOT_DIR}/include/quickjs)

find_package (Threads)

# 构建可执行程序
add_executable(qjsMain ${PROJECT_SRC})
TARGET_LINK_LIBRARIES(qjsMain m)
target_link_libraries (qjsMain ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(qjsMain -ldl)
