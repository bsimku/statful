set(SOURCES
    alloc.c
    bar.c
    block_battery.c
    block_cpu.c
    block_memory.c
    block_time.c
    block_volume.c
    main.c
)

set(LIBRARIES
    ${PULSE_LIBRARIES}
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBRARIES})
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)
