################################################################################
# Automatically-generated file. Do not edit!
################################################################################

#Progress monitor hint: 40
first : all
-include ../makefile.init

# This file contains definitions of environment variables used in the makefiles and .args files if exist.
-include makefile.local

RM := "$(GNU_Make_Install_DirEnv)/rm" -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/walk/subdir.mk
-include src/torgo/subdir.mk
-include src/torgo/commands/subdir.mk
-include src/sys/subdir.mk
-include src/process/subdir.mk
-include src/portal/subdir.mk
-include src/path/subdir.mk
-include src/nodeinfo/subdir.mk
-include src/kmalloc/subdir.mk
-include src/dev/subdir.mk
-include src/crumb/subdir.mk
-include src/boot/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(SX_DEPS)),)
-include $(SX_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
manos.elf \

EXECUTABLES_QUOTED += \
"manos.elf" \

EXECUTABLES_OS_FORMAT += \
manos.elf \


# All Target
all: warning-messages explicit-dependencies manos.elf secondary-outputs

# Tool invocations
manos.elf: $(OBJS) $(USER_OBJS) ../Project_Settings/Linker_Files/MK70FN1M0_ram.ld
	@echo 'Building target: $@'
	@echo 'Executing target #41 $@'
	@echo 'Invoking: ARM Ltd Windows GCC C Linker'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc"  $(USER_OBJS_QUOTED) $(LIBS_QUOTED) @"manos.args" -o"manos.elf"
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(S_DEPS_QUOTED) "./*/*/*.o" "./*/*/*/*.o"  $(S_UPPER_DEPS_QUOTED) $(ASM_DEPS_QUOTED) $(ASM_UPPER_DEPS_QUOTED) $(SX_DEPS_QUOTED) $(EXECUTABLES_QUOTED) "./*/*/*.d" "./*/*/*/*.d"  $(ProjDirPath)/RAM/manos.elf
	-@echo ' '

secondary-outputs:

.PHONY: all clean dependents explicit-dependencies warning-messages
.SECONDARY:

-include ../makefile.targets

explicit-dependencies: ../Project_Settings/Linker_Files/MK70FN1M0_ram.ld 

warning-messages: 

