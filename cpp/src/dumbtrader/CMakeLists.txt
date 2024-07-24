add_subdirectory(container)
add_subdirectory(order_book)
add_subdirectory(ipc)

add_executable(consumer consumer.cpp)
add_executable(producer producer.cpp)

target_link_libraries(consumer PRIVATE -lpthread)
target_link_libraries(producer PRIVATE -lpthread)