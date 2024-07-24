add_executable(consumer consumer.cpp)
add_executable(producer producer.cpp)

target_include_directories(consumer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(producer PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(consumer PRIVATE -lpthread dumbtrader)
target_link_libraries(producer PRIVATE -lpthread dumbtrader)