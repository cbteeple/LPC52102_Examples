################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/FreeRTOSCommonHooks.c \
../src/blinky.c \
../src/cr_startup_lpc5410x.c \
../src/croutine.c \
../src/crp.c \
../src/heap_3.c \
../src/list.c \
../src/port.c \
../src/queue.c \
../src/sysinit.c \
../src/tasks.c \
../src/timers.c 

OBJS += \
./src/FreeRTOSCommonHooks.o \
./src/blinky.o \
./src/cr_startup_lpc5410x.o \
./src/croutine.o \
./src/crp.o \
./src/heap_3.o \
./src/list.o \
./src/port.o \
./src/queue.o \
./src/sysinit.o \
./src/tasks.o \
./src/timers.o 

C_DEPS += \
./src/FreeRTOSCommonHooks.d \
./src/blinky.d \
./src/cr_startup_lpc5410x.d \
./src/croutine.d \
./src/crp.d \
./src/heap_3.d \
./src/list.d \
./src/port.d \
./src/queue.d \
./src/sysinit.d \
./src/tasks.d \
./src/timers.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__MULTICORE_NONE -D__USE_LPCOPEN -DCORE_M4 -DDEBUG -D__CODE_RED -D__LPC5410X__ -D__REDLIB__ -I"C:\nxp\Good_Workspace\lpc_chip_5410x\inc" -I"C:\nxp\Good_Workspace\lpc_board_lpcxpresso_54102\inc" -I"C:\nxp\Good_Workspace\freertos_blinky\inc" -I"C:\nxp\Good_Workspace\freertos\inc" -I"C:\nxp\Good_Workspace\freertos\inc_arm_4f" -I"C:\nxp\Good_Workspace\freertos\inc_freertoslpc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


