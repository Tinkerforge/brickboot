CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(PROJECT_NAME brickboot)
SET(CHIP sam3s2)
SET(MCU cortex-m3\ -mthumb)
PROJECT(${PROJECT_NAME})

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
#SET(OPTIMIZATION_LEVEL 0)
SET(DEBUG -g)

SET(OPTIMIZATION_LEVEL 3)
#SET(DEBUG "")

SET(TRACE_LEVEL 0)
SET(CMAKE_BUILD_TYPE None)

INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}/src/"
	"${PROJECT_SOURCE_DIR}/src/bricklib/drivers/"
)

# find source files
#FILE(GLOB_RECURSE SOURCES "src/*.c")
SET(SOURCES "src/main.c"
            "src/isp_cstartup.c"
            "src/bricklib/utility/syscalls.c"
            "src/bricklib/drivers/board/board_lowlevel.c"
            "src/bricklib/drivers/pio/pio.c"
            "src/bricklib/drivers/efc/efc.c"
            "src/bricklib/drivers/wdt/wdt.c"
            "src/bricklib/drivers/pmc/pmc.c"
            "src/bricklib/drivers/flash/flashd.c")

MESSAGE(STATUS "\nFound following source files:\n ${SOURCES}\n")

# define executable
ADD_EXECUTABLE(${PROJECT_NAME}.elf ${SOURCES})
SET_TARGET_PROPERTIES(${PROJECT_NAME}.elf PROPERTIES LINKER_LANGUAGE C)

# TARGET_LINK_LIBRARIES(${PROJECT_NAME}.elf -lgcc -lc)

# define compile flags
SET_TARGET_PROPERTIES(${PROJECT_NAME}.elf PROPERTIES COMPILE_FLAGS
	"${DEBUG} -mcpu=${MCU} -std=gnu99  -Wall -mlong-calls -ffunction-sections -O${OPTIMIZATION_LEVEL}"
)

#define linker flags
SET_TARGET_PROPERTIES(${PROJECT_NAME}.elf PROPERTIES LINK_FLAGS
	"-mcpu=${MCU} -Wl,--gc-sections -T\"${PROJECT_SOURCE_DIR}/src/sram_samba_${CHIP}.lds\" "
)

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}.elf POST_BUILD COMMAND 
                   ${CMAKE_OBJCOPY} -S -O binary 
                   ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}.elf POST_BUILD COMMAND 
                   ${CMAKE_SIZE} 
                   ${OBJECT} ${PROJECT_NAME}.elf)

# add preprocessor defines
ADD_DEFINITIONS(-D${CHIP} -DTRACE_LEVEL=${TRACE_LEVEL} -Dflash -Dprintf=iprintf) 
