file(GLOB_RECURSE ORDER_BOOK_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
add_library(order_book OBJECT ${ORDER_BOOK_SOURCES})
target_include_directories(order_book PUBLIC ${PROJECT_SOURCE_DIR}/include)