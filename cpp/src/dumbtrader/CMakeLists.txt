# header only modules no need to compile
add_library(dumbtrader SHARED
    # $<TARGET_OBJECTS:container>
    # $<TARGET_OBJECTS:ipc>
    $<TARGET_OBJECTS:network>
    $<TARGET_OBJECTS:order_book>
    # add_subdirectory(utils)
)

# add_subdirectory(container)
# add_subdirectory(ipc)
add_subdirectory(network)
add_subdirectory(order_book)
# add_subdirectory(utils)