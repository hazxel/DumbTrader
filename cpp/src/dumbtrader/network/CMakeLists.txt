file(GLOB_RECURSE ORDER_BOOK_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
add_library(network OBJECT ${ORDER_BOOK_SOURCES})

find_package(OpenSSL REQUIRED)

target_include_directories(network PUBLIC ${PROJECT_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})
target_link_libraries(network PUBLIC OpenSSL::SSL OpenSSL::Crypto)

if(UNIX AND NOT APPLE)
    message(STATUS "Detected Linux platform, try to enable liburing")
    include(CheckIncludeFile)
    check_include_file("liburing.h" HAVE_LIBURING_H)
    if(HAVE_LIBURING_H)
        find_library(LIBURING_LIBRARIES NAMES uring)
        if(LIBURING_LIBRARIES)
            message(STATUS "liburing is available, enabling related optimizations")
            add_compile_definitions(LIBURING_ENABLED)
            target_link_libraries(network PUBLIC ${LIBURING_LIBRARIES})
        else()
            message(WARNING "liburing header found but library not found")
        endif()
    else()
        message(WARNING "liburing not available, optimizations will not be enabled")
    endif()
endif()