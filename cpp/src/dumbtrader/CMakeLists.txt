# header only modules no need to compile
add_library(dumbtrader SHARED
    # $<TARGET_OBJECTS:container>
    # $<TARGET_OBJECTS:ipc>
    $<TARGET_OBJECTS:network>
    $<TARGET_OBJECTS:order_book>
    # $<TARGET_OBJECTS:utils>
)

# add_subdirectory(container)
# add_subdirectory(ipc)
add_subdirectory(network)
add_subdirectory(order_book)
# add_subdirectory(utils)

find_package(OpenSSL REQUIRED)
if(APPLE)
    target_link_libraries(dumbtrader PUBLIC OpenSSL::SSL OpenSSL::Crypto)
elseif(UNIX)
    target_link_libraries(dumbtrader PUBLIC OpenSSL::SSL OpenSSL::Crypto pthread rt)
endif()
