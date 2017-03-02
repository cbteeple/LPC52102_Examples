################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Testing_GPIO.c \
../src/boot_multicore_slave.c \
../src/cr_startup_lpc5410x-m0.c \
../src/crp.c 

OBJS += \
./src/Testing_GPIO.o \
./src/boot_multicore_slave.o \
./src/cr_startup_lpc5410x-m0.o \
./src/crp.o 

C_DEPS += \
./src/Testing_GPIO.d \
./src/boot_multicore_slave.d \
./src/cr_startup_lpc5410x-m0.d \
./src/crp.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__MULTICORE_NONE -DDEBUG -D__CODE_RED -DCORE_M0PLUS -D__LPC5410X__ -D__REDLIB__ -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


