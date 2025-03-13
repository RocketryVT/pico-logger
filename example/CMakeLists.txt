if (COMPILE_EXAMPLE)
    add_executable(example
                   example.cpp
    )

    target_link_libraries(example pico_stdlib pico_logger)

    pico_enable_stdio_usb(example 1)
    pico_enable_stdio_uart(example 0)

    pico_add_extra_outputs(example)
endif()
