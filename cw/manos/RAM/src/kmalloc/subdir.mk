################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"$(KMALLOC)/kmalloc.c" \

C_SRCS += \
$(KMALLOC)/kmalloc.c \

OBJS += \
./src/kmalloc/kmalloc.o \

C_DEPS += \
./src/kmalloc/kmalloc.d \

OBJS_QUOTED += \
"./src/kmalloc/kmalloc.o" \

C_DEPS_QUOTED += \
"./src/kmalloc/kmalloc.d" \

OBJS_OS_FORMAT += \
./src/kmalloc/kmalloc.o \


# Each subdirectory must supply rules for building sources it contributes
src/kmalloc/kmalloc.o: $(KMALLOC)/kmalloc.c
	@echo 'Building file: $<'
	@echo 'Executing target #27 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/kmalloc/kmalloc.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/kmalloc/kmalloc.o"
	@echo 'Finished building: $<'
	@echo ' '


