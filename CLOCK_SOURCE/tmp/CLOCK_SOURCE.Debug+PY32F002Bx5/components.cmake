# components.cmake

# component ARM::CMSIS:CORE@6.2.0
add_library(ARM_CMSIS_CORE_6_2_0 INTERFACE)
target_include_directories(ARM_CMSIS_CORE_6_2_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  "${CMSIS_PACK_ROOT}/ARM/CMSIS/6.3.0/CMSIS/Core/Include"
)
target_compile_definitions(ARM_CMSIS_CORE_6_2_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_link_libraries(ARM_CMSIS_CORE_6_2_0 INTERFACE
  ${CONTEXT}_ABSTRACTIONS
)

# component Puya::Device:Startup@1.0.0
add_library(Puya_Device_Startup_1_0_0 OBJECT
  "${SOLUTION_ROOT}/CLOCK_SOURCE/RTE/Device/PY32F002Bx5/startup_py32f002bxx.s"
  "${SOLUTION_ROOT}/CLOCK_SOURCE/RTE/Device/PY32F002Bx5/system_py32f002b.c"
)
target_include_directories(Puya_Device_Startup_1_0_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  "${CMSIS_PACK_ROOT}/Puya/PY32F0xx_DFP/1.2.8/Drivers/CMSIS/Device/PY32F0xx/Include"
)
target_compile_definitions(Puya_Device_Startup_1_0_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Puya_Device_Startup_1_0_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Puya_Device_Startup_1_0_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)
set(COMPILE_DEFINITIONS
  _RTE_
)
cbuild_set_defines(AS_ARM COMPILE_DEFINITIONS)
set_source_files_properties("${SOLUTION_ROOT}/CLOCK_SOURCE/RTE/Device/PY32F002Bx5/startup_py32f002bxx.s" PROPERTIES
  COMPILE_FLAGS "${COMPILE_DEFINITIONS}"
)
