################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
SimplyBlinkyDemo/main_blinky.obj: ../SimplyBlinkyDemo/main_blinky.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O0 --opt_for_speed=2 --include_path="C:/ti/ccsv8/ccs_base/arm/include" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS/Demo/CORTEX_M4F_MSP432_LaunchPad_IAR_CCS_Keil/Full_Demo" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS/Demo/CORTEX_M4F_MSP432_LaunchPad_IAR_CCS_Keil/driverlib/inc" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS/Demo/CORTEX_M4F_MSP432_LaunchPad_IAR_CCS_Keil/driverlib" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS-Plus/Source/FreeRTOS-Plus-CLI" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS/Demo/Common/include" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS/Source/include" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.2.LTS/include" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.2.LTS/include/" --include_path="C:/ti/ccsv8/ccs_base/arm/include/" --include_path="C:/ti/ccsv8/ccs_base/arm/include/CMSIS" --include_path="C:/Users/student/Desktop/FreeRTOSv10.1.1/FreeRTOS/Demo/CORTEX_M4F_MSP432_LaunchPad_IAR_CCS_Keil" -g --gcc --define=__MSP432P401R__ --define=USE_CMSIS_REGISTER_FORMAT=1 --define=TARGET_IS_MSP432P4XX --define=ccs --diag_warning=225 --diag_wrap=off --display_error_number --verbose_diagnostics --gen_func_subsections=on --preproc_with_compile --preproc_dependency="SimplyBlinkyDemo/main_blinky.d_raw" --obj_directory="SimplyBlinkyDemo" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


