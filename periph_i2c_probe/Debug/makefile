################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: periph_i2c_probe.axf

# Tool invocations
periph_i2c_probe.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\nxp\Good_Workspace\lpc_chip_5410x\libs" -L"C:\nxp\Good_Workspace\lpc_chip_5410x\Debug" -L"C:\nxp\Good_Workspace\lpc_board_lpcxpresso_54102\Debug" -Xlinker -Map="periph_i2c_probe.map" -Xlinker --gc-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -T "periph_periph_i2cm_interrupt_Debug.ld" -o "periph_i2c_probe.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) periph_i2c_probe.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "periph_i2c_probe.axf"; # arm-none-eabi-objcopy -v -O binary "periph_i2c_probe.axf" "periph_i2c_probe.bin" ; # checksum -p LPC54102J512 -d "periph_i2c_probe.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
