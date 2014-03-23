################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"$(KERNEL)/dev.c" \
"$(KERNEL)/devled.c" \
"$(KERNEL)/err.c" \
"$(KERNEL)/namespace.c" \
"$(KERNEL)/portal.c" \

C_SRCS += \
$(KERNEL)/dev.c \
$(KERNEL)/devled.c \
$(KERNEL)/err.c \
$(KERNEL)/namespace.c \
$(KERNEL)/portal.c \

OBJS += \
./src/manos/kernel/dev.o \
./src/manos/kernel/devled.o \
./src/manos/kernel/err.o \
./src/manos/kernel/namespace.o \
./src/manos/kernel/portal.o \

C_DEPS += \
./src/manos/kernel/dev.d \
./src/manos/kernel/devled.d \
./src/manos/kernel/err.d \
./src/manos/kernel/namespace.d \
./src/manos/kernel/portal.d \

OBJS_QUOTED += \
"./src/manos/kernel/dev.o" \
"./src/manos/kernel/devled.o" \
"./src/manos/kernel/err.o" \
"./src/manos/kernel/namespace.o" \
"./src/manos/kernel/portal.o" \

C_DEPS_QUOTED += \
"./src/manos/kernel/dev.d" \
"./src/manos/kernel/devled.d" \
"./src/manos/kernel/err.d" \
"./src/manos/kernel/namespace.d" \
"./src/manos/kernel/portal.d" \

OBJS_OS_FORMAT += \
./src/manos/kernel/dev.o \
./src/manos/kernel/devled.o \
./src/manos/kernel/err.o \
./src/manos/kernel/namespace.o \
./src/manos/kernel/portal.o \


# Each subdirectory must supply rules for building sources it contributes
src/manos/kernel/dev.o: $(KERNEL)/dev.c
	@echo 'Building file: $<'
	@echo 'Executing target #10 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/kernel/dev.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/kernel/dev.o"
	@echo 'Finished building: $<'
	@echo ' '

src/manos/kernel/devled.o: $(KERNEL)/devled.c
	@echo 'Building file: $<'
	@echo 'Executing target #11 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/kernel/devled.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/kernel/devled.o"
	@echo 'Finished building: $<'
	@echo ' '

src/manos/kernel/err.o: $(KERNEL)/err.c
	@echo 'Building file: $<'
	@echo 'Executing target #12 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/kernel/err.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/kernel/err.o"
	@echo 'Finished building: $<'
	@echo ' '

src/manos/kernel/namespace.o: $(KERNEL)/namespace.c
	@echo 'Building file: $<'
	@echo 'Executing target #13 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/kernel/namespace.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/kernel/namespace.o"
	@echo 'Finished building: $<'
	@echo ' '

src/manos/kernel/portal.o: $(KERNEL)/portal.c
	@echo 'Building file: $<'
	@echo 'Executing target #14 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/kernel/portal.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/kernel/portal.o"
	@echo 'Finished building: $<'
	@echo ' '


