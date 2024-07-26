add_executable(test_sem test_sem.cpp)
add_executable(test_shared_mem test_shared_mem.cpp)

target_include_directories(test_sem PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(test_shared_mem PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(test_sem PRIVATE -lpthread dumbtrader)
target_link_libraries(test_shared_mem PRIVATE -lpthread dumbtrader)