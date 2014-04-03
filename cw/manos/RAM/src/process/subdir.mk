################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../src/process/getpid.c" \

C_SRCS += \
../src/process/getpid.c \

OBJS += \
./src/process/getpid.o \

C_DEPS += \
./src/process/getpid.d \

OBJS_QUOTED += \
"./src/process/getpid.o" \

C_DEPS_QUOTED += \
"./src/process/getpid.d" \

OBJS_OS_FORMAT += \
./src/process/getpid.o \


# Each subdirectory must supply rules for building sources it contributes
src/process/getpid.o: ../src/process/getpid.c
	@echo 'Building file: $<'
	@echo 'Executing target #21 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/process/getpid.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/process/getpid.o"
	@echo 'Finished building: $<'
	@echo ' '


