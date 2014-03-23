################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"$(LIBC)/malloc.c" \
"$(LIBC)/string.c" \
"$(LIBC)/util.c" \

C_SRCS += \
$(LIBC)/malloc.c \
$(LIBC)/string.c \
$(LIBC)/util.c \

OBJS += \
./src/manos/libc/malloc.o \
./src/manos/libc/string.o \
./src/manos/libc/util.o \

C_DEPS += \
./src/manos/libc/malloc.d \
./src/manos/libc/string.d \
./src/manos/libc/util.d \

OBJS_QUOTED += \
"./src/manos/libc/malloc.o" \
"./src/manos/libc/string.o" \
"./src/manos/libc/util.o" \

C_DEPS_QUOTED += \
"./src/manos/libc/malloc.d" \
"./src/manos/libc/string.d" \
"./src/manos/libc/util.d" \

OBJS_OS_FORMAT += \
./src/manos/libc/malloc.o \
./src/manos/libc/string.o \
./src/manos/libc/util.o \


# Each subdirectory must supply rules for building sources it contributes
src/manos/libc/malloc.o: $(LIBC)/malloc.c
	@echo 'Building file: $<'
	@echo 'Executing target #7 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/libc/malloc.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/libc/malloc.o"
	@echo 'Finished building: $<'
	@echo ' '

src/manos/libc/string.o: $(LIBC)/string.c
	@echo 'Building file: $<'
	@echo 'Executing target #8 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/libc/string.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/libc/string.o"
	@echo 'Finished building: $<'
	@echo ' '

src/manos/libc/util.o: $(LIBC)/util.c
	@echo 'Building file: $<'
	@echo 'Executing target #9 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/manos/libc/util.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/manos/libc/util.o"
	@echo 'Finished building: $<'
	@echo ' '


