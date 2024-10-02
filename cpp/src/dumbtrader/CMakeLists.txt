# add_subdirectory(container)
# add_subdirectory(ipc)
add_subdirectory(network)
add_subdirectory(order_book)
# add_subdirectory(utils)

add_library(dumbtrader SHARED)
# header only modules no need to compile
target_link_libraries(dumbtrader PRIVATE network order_book)

if(APPLE)
    message(STATUS "Detected OSX platform")
    target_link_libraries(dumbtrader PUBLIC)
elseif(UNIX)
    message(STATUS "Detected Linux platform")
    target_link_libraries(dumbtrader PUBLIC pthread rt)
endif()
