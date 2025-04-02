set(TARGET_NAME "pico_logger")

if (NOT TARGET ${TARGET_NAME})
    add_library(${TARGET_NAME} INTERFACE)

    target_sources(${TARGET_NAME} INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/src/pico_logger.cpp
    )
    target_include_directories(${TARGET_NAME} INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/include
    )

    target_link_libraries(${TARGET_NAME} INTERFACE
        pico_stdlib pico_flash hardware_flash hardware_exception hardware_sync
    )
endif()

if (COMPILE_EXAMPLE)
    cmake_minimum_required(VERSION 3.12)

    set(PICO_BOARD pico)

    include(include/pico-sdk/pico_sdk_init.cmake)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

    project(pico_logger_example C CXX ASM)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)

    if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
        message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
    endif()

    pico_sdk_init()

    add_subdirectory(example)

    add_compile_options(
            -Wall
            -Wno-format
            -Wno-unused-function
    )

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wno-maybe-uninitialized)
    endif()
endif()
