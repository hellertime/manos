################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"$(BOOT)/main.c" \

C_SRCS += \
$(BOOT)/main.c \

OBJS += \
./src/boot/main.o \

C_DEPS += \
./src/boot/main.d \

OBJS_QUOTED += \
"./src/boot/main.o" \

C_DEPS_QUOTED += \
"./src/boot/main.d" \

OBJS_OS_FORMAT += \
./src/boot/main.o \


# Each subdirectory must supply rules for building sources it contributes
src/boot/main.o: $(BOOT)/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #37 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/boot/main.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/boot/main.o"
	@echo 'Finished building: $<'
	@echo ' '


