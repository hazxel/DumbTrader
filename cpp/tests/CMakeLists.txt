add_executable(sem_consumer sem_consumer.cpp)
add_executable(sem_producer sem_producer.cpp)
add_executable(test_sem test_sem.cpp)

target_include_directories(sem_consumer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(sem_producer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(test_sem PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(sem_consumer PRIVATE -lpthread dumbtrader)
target_link_libraries(sem_producer PRIVATE -lpthread dumbtrader)
target_link_libraries(test_sem PRIVATE -lpthread dumbtrader)