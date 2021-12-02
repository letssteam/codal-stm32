project(codal-stm32)

include("${CODAL_UTILS_LOCATION}")

#set(CMAKE_CXX_CLANG_TIDY
#    clang-tidy;
#    --extra-arg=-std=c++17;
#    --extra-arg=-Wno-error;
#    --extra-arg=-Wno-register;)

RECURSIVE_FIND_DIR(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/inc" "*.h")
list(APPEND INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/stm32"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/stm32/LL"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/stm32/usb"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/stm32/usb/cdc"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/stm32/usb/hid"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/stm32/usb/webusb"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/CMSIS_5/CMSIS/Core/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Drivers/CMSIS/Device/ST/${device.series}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Drivers/CMSIS/Device/ST/${device.series}/Source/Templates/gcc"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/${device.series}"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Drivers/${device.series}_HAL_Driver/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Drivers/${device.series}_HAL_Driver/Src"
    "${CMAKE_CURRENT_SOURCE_DIR}/variants/${device.variant}"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Middlewares/ST/STM32_USB_Device_Library/Core/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Middlewares/ST/STM32_USB_Device_Library/Core/Src"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Middlewares/ST/STM32_USB_Device_Library/Class/WebUSB/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/Middlewares/ST/STM32_USB_Device_Library/Class/WebUSB/Src"
    )
 
string(TOLOWER  "${device.series}" PROCESSOR_NAME)

RECURSIVE_FIND_FILE(CPP_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/" "*.c??")
list(APPEND CPP_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/variants/${device.variant}/${device.config.VARIANT}.cpp")

RECURSIVE_FIND_FILE(C_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/" "*.c")
list(APPEND C_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/asm/CortexContextSwitch.s")
list(APPEND C_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/stm32/startup_stm32yyxx.S")
list(APPEND C_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/system/Drivers/CMSIS/Device/ST/${device.series}/Source/Templates/system_${PROCESSOR_NAME}.c")
list(APPEND C_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/variants/${device.variant}/${device.config.PERIPHERAL_PINS}.c")

set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -T\"${CMAKE_CURRENT_LIST_DIR}/variants/${device.variant}/ldscript.ld\"" PARENT_SCOPE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T\"${CMAKE_CURRENT_LIST_DIR}/variants/${device.variant}/ldscript.ld\"" PARENT_SCOPE)
set(CMAKE_SYSTEM_PROCESSOR "armv7-m" PARENT_SCOPE)

# add them
include_directories(${INCLUDE_DIRS})

# create our target
add_library(codal-stm32 ${C_SOURCE_FILES} ${CPP_SOURCE_FILES})

target_link_libraries(
    codal-stm32 
    codal-core 
    codal-nimble
    )

# expose it to parent cmake.
target_include_directories(codal-stm32 PUBLIC ${INCLUDE_DIRS})
