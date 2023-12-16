set(SOURCES
    alloc.c
    bar.c
    config.c
    blocks.c
    block_battery.c
    block_cpu.c
    block_lua.c
    block_memory.c
    block_time.c
    block_volume.c
    main.c
)

set(LIBRARIES
    ${PULSE_LIBRARIES}
    ${LUAJIT_LIBRARIES}
)

set(INCLUDES
    ${LUAJIT_INCLUDE_DIRS}
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBRARIES})
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)
