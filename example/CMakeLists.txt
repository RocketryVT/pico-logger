if (COMPILE_EXAMPLE)
    add_executable(example
                   example.cpp
    )

    #    pico_set_binary_type(example copy_to_ram)
    target_link_libraries(example pico_stdlib pico_rand pico_multicore pico_logger)

    pico_enable_stdio_usb(example 1)
    pico_enable_stdio_uart(example 0)

    pico_add_extra_outputs(example)
endif()
