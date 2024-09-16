file(GLOB_RECURSE ORDER_BOOK_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
add_library(network ${ORDER_BOOK_SOURCES})

find_package(OpenSSL REQUIRED)

target_include_directories(network PUBLIC ${PROJECT_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})