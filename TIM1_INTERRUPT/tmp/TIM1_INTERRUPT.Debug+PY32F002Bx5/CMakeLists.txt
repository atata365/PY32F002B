cmake_minimum_required(VERSION 3.27)

# Roots
include("../roots.cmake")

set(CONTEXT TIM1_INTERRUPT.Debug+PY32F002Bx5)
set(TARGET ${CONTEXT})
set(DNAME PY32F002Bx5)
set(DPACK Puya::PY32F0xx_DFP@1.2.8)
set(DPACK_DIR "${CMSIS_PACK_ROOT}/Puya/PY32F0xx_DFP/1.2.8")
set(OUT_DIR "${SOLUTION_ROOT}/out/TIM1_INTERRUPT/PY32F002Bx5/Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(HEX_FILE "TIM1_INTERRUPT.hex")
set(LD_MAP_FILE "TIM1_INTERRUPT.axf.map")
set(LD_SCRIPT "${SOLUTION_ROOT}/TIM1_INTERRUPT/RTE/Device/PY32F002Bx5/ac6_linker_script.sct.src")
set(LD_REGIONS "${SOLUTION_ROOT}/TIM1_INTERRUPT/RTE/Device/PY32F002Bx5/regions_PY32F002Bx5.h")
set(LD_SCRIPT_PP "${CMAKE_CURRENT_BINARY_DIR}/ac6_linker_script.sct")

# Processor Options
set(CPU Cortex-M0+)
set(FPU NO_FPU)
set(BYTE_ORDER Little-endian)

# Toolchain config map
set(COMPILER AC6)
include("toolchain.cmake")

# Setup project
project(${CONTEXT} LANGUAGES CXX ASM C)

# Enable color diagnostics
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Compilation database
add_custom_target(database
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" "${OUT_DIR}"
)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

# Setup context
add_executable(${CONTEXT})
set_target_properties(${CONTEXT} PROPERTIES PREFIX "" SUFFIX ".axf" OUTPUT_NAME "TIM1_INTERRUPT")
set_target_properties(${CONTEXT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
add_library(${CONTEXT}_GLOBAL INTERFACE)

# Includes
target_include_directories(${CONTEXT} PUBLIC
  "${SOLUTION_ROOT}/TIM1_INTERRUPT/RTE/_Debug_PY32F002Bx5"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS/6.3.0/CMSIS/Core/Include"
  "${CMSIS_PACK_ROOT}/Puya/PY32F0xx_DFP/1.2.8/Drivers/CMSIS/Device/PY32F0xx/Include"
)

# Defines
target_compile_definitions(${CONTEXT} PUBLIC
  $<$<COMPILE_LANGUAGE:C,CXX>:
    _RTE_
  >
)

# Compile options
target_compile_options(${CONTEXT} PUBLIC
  $<$<COMPILE_LANGUAGE:ASM>:
    "SHELL:${ASM_CPU}"
    "SHELL:${ASM_FLAGS}"
    "SHELL:${ASM_BYTE_ORDER}"
    "SHELL:-gdwarf-5"
    "SHELL:-masm=auto"
  >
  $<$<COMPILE_LANGUAGE:C>:
    "SHELL:${CC_CPU}"
    "SHELL:${CC_FLAGS}"
    "SHELL:${CC_BYTE_ORDER}"
    "SHELL:-gdwarf-5"
    "SHELL:-std=gnu11"
    "SHELL:-Wno-macro-redefined"
    "SHELL:-Wno-pragma-pack"
    "SHELL:-Wno-parentheses-equality"
    "SHELL:-Wno-license-management"
  >
  $<$<COMPILE_LANGUAGE:CXX>:
    "SHELL:${CXX_CPU}"
    "SHELL:${CXX_FLAGS}"
    "SHELL:${CXX_BYTE_ORDER}"
    "SHELL:-gdwarf-5"
    "SHELL:-Wno-macro-redefined"
    "SHELL:-Wno-pragma-pack"
    "SHELL:-Wno-parentheses-equality"
    "SHELL:-Wno-license-management"
  >
)

# Compile Options Abstractions
add_library(${CONTEXT}_ABSTRACTIONS INTERFACE)
cbuild_set_options_flags(CXX "none" "on" "" "" CXX_OPTIONS_FLAGS_${CONTEXT})
cbuild_set_options_flags(ASM "none" "on" "" "" ASM_OPTIONS_FLAGS_${CONTEXT})
cbuild_set_options_flags(CC "none" "on" "" "" CC_OPTIONS_FLAGS_${CONTEXT})
target_compile_options(${CONTEXT}_ABSTRACTIONS INTERFACE
  $<$<COMPILE_LANGUAGE:CXX>:
    "SHELL:${CXX_OPTIONS_FLAGS_${CONTEXT}}"
  >
  $<$<COMPILE_LANGUAGE:ASM>:
    "SHELL:${ASM_OPTIONS_FLAGS_${CONTEXT}}"
  >
  $<$<COMPILE_LANGUAGE:C>:
    "SHELL:${CC_OPTIONS_FLAGS_${CONTEXT}}"
  >
)

# Add groups and components
include("groups.cmake")
include("components.cmake")

target_link_libraries(${CONTEXT} PUBLIC
  Group_Source_Files
  ARM_CMSIS_CORE_6_2_0
  Puya_Device_Startup_1_0_0
)

# Linker options
target_link_options(${CONTEXT} PUBLIC
  "SHELL:${LD_CPU}"
  "SHELL:${_LS}\"${LD_SCRIPT_PP}\""
  "SHELL:${LD_MAP}"
  "SHELL:--entry=Reset_Handler"
  "SHELL:--info=summarysizes"
  "SHELL:--summary_stderr"
  "SHELL:--diag_suppress=L6314W"
)
set(LD_DEPS ${LD_SCRIPT} ${LD_REGIONS})
set_target_properties(${CONTEXT} PROPERTIES LINK_DEPENDS "${LD_DEPS}")

# Linker script pre-processing
add_custom_command(TARGET ${CONTEXT} PRE_LINK COMMAND ${CPP} ARGS ${CPP_ARGS_LD_SCRIPT} BYPRODUCTS ${LD_SCRIPT_PP})

# Hex Conversion
add_custom_command(TARGET ${CONTEXT} POST_BUILD COMMAND ${CMAKE_OBJCOPY} ${ELF2HEX})
