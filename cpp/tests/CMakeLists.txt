add_executable(test_sem test_sem.cpp)

target_include_directories(test_sem PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(test_sem PRIVATE -lpthread dumbtrader)