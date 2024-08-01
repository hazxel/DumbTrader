cmake_minimum_required(VERSION 3.19)

project(dumbtrader LANGUAGES C CXX)

if(APPLE)
    set(CMAKE_C_COMPILER /usr/bin/clang)
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
elseif(UNIX)
    message(WARNING "only tested under CentOS")
elseif(WIN32)
    message(FATAL_ERROR "does not support Windows")
endif()

set(CMAKE_CXX_STANDARD 20)          # 设置 C++ 标准为 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 确保使用指定的 C++ 标准
set(CMAKE_CXX_EXTENSIONS OFF)       # 禁用编译器特定的扩展

set(CMAKE_CXX_FLAGS -fPIC)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -Wl,-z,relro,-z,now,-z,noexecstack -s -shared -fPIC")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now -pie")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro,-z,now")

add_subdirectory(src)
add_subdirectory(tests)