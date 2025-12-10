# CompSys-GroupProject CMakeLists.txt
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define project name
set(APP_NAME morseproject) 
# Define main app (app in this folder)
set(MAIN_TARGET morse_to_pico)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work =======
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ================================================================================================

# == DO NOT EDIT:  SETUP pico SDK and create project ==============================================

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(${APP_NAME} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# =================================================================================================

# First include FreeRTOS Kernel libraries
include(FreeRTOS_Kernel_import.cmake)

# Include the configuration files
target_include_directories(FreeRTOS-Kernel INTERFACE
${CMAKE_CURRENT_LIST_DIR}/config
)

# Add librabries. The TKJHAT does not use libraries from FreeRTOS
add_subdirectory(libs/TKJHAT)
# Support for usb serial communication
add_subdirectory(libs/usb-serial-debug)

# Add the source files to be compiled
add_executable(${MAIN_TARGET}
    src/main.c
)

# Include libraries necessary for running the application. Remove unecessary libs once project is finished.
target_link_libraries(${MAIN_TARGET}
        pico_stdlib
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        TKJHAT_SDK
        usb_serial_debug
)

# Include libraries necessaries to control the WiFi. If you are using the internal pico LED in W model, it is also 
# necessary 
if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${MAIN_TARGET} pico_cyw43_arch_none)
endif()

#Support for stdio (printf, fwrite, puts...) via usb or UART. 
pico_enable_stdio_usb(${MAIN_TARGET} 1)
pico_enable_stdio_uart(${MAIN_TARGET} 0)

pico_add_extra_outputs(morse_to_pico)