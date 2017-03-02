################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adc.c \
../src/cr_startup_lpc5410x.c \
../src/crp.c \
../src/sysinit.c 

OBJS += \
./src/adc.o \
./src/cr_startup_lpc5410x.o \
./src/crp.o \
./src/sysinit.o 

C_DEPS += \
./src/adc.d \
./src/cr_startup_lpc5410x.d \
./src/crp.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__MULTICORE_NONE -D__USE_LPCOPEN -DCORE_M4 -DDEBUG -D__CODE_RED -DCORE_M4 -D__LPC5410X__ -D__REDLIB__ -I"C:\nxp\Good_Workspace\lpc_chip_5410x\inc" -I"C:\nxp\Good_Workspace\lpc_board_lpcxpresso_54102\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


