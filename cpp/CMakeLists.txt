cmake_minimum_required(VERSION 3.19)

project(dumbtrader LANGUAGES C CXX)

if(APPLE)
    set(CMAKE_C_COMPILER /usr/bin/clang)
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
elseif(UNIX)
    message(WARNING "not tested under other unix os")
elseif(WIN32)
    message(WARNING "not tested under windows")
endif()

# if(NOT CMAKE_C_COMPILER)
#     find_program(GCC_COMPILER NAMES gcc)
#     if(GCC_COMPILER)
#         set(CMAKE_C_COMPILER ${GCC_COMPILER})
#         message(STATUS "C compiler set to ${GCC_COMPILER}")
#     else()
#         message(FATAL_ERROR "No suitable C compiler found. Please install GCC or set CMAKE_C_COMPILER manually.")
#     endif()
# endif()

# if(NOT CMAKE_CXX_COMPILER)
#     find_program(GXX_COMPILER NAMES g++)
#     if(GXX_COMPILER)
#         set(CMAKE_CXX_COMPILER ${GXX_COMPILER})
#         message(STATUS "C++ compiler set to ${GXX_COMPILER}")
#     else()
#         message(FATAL_ERROR "No suitable C++ compiler found. Please install G++ or set CMAKE_CXX_COMPILER manually.")
#     endif()
# endif()
# message(INFO "C compiler: ${CMAKE_C_COMPILER}")
# message(INFO "C++ compiler: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 17)          # 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 确保使用指定的 C++ 标准
set(CMAKE_CXX_EXTENSIONS OFF)       # 禁用编译器特定的扩展

add_subdirectory(src)
add_subdirectory(tests)