add_library(dumbtrader SHARED
    $<TARGET_OBJECTS:ipc>
    # $<TARGET_OBJECTS:container>
    # $<TARGET_OBJECTS:order_book>
)

add_subdirectory(ipc)
# add_subdirectory(container)
# add_subdirectory(order_book)
# add_subdirectory(utils)