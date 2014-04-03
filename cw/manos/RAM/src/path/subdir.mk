################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../src/path/mkPath.c" \

C_SRCS += \
../src/path/mkPath.c \

OBJS += \
./src/path/mkPath.o \

C_DEPS += \
./src/path/mkPath.d \

OBJS_QUOTED += \
"./src/path/mkPath.o" \

C_DEPS_QUOTED += \
"./src/path/mkPath.d" \

OBJS_OS_FORMAT += \
./src/path/mkPath.o \


# Each subdirectory must supply rules for building sources it contributes
src/path/mkPath.o: ../src/path/mkPath.c
	@echo 'Building file: $<'
	@echo 'Executing target #25 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/path/mkPath.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/path/mkPath.o"
	@echo 'Finished building: $<'
	@echo ' '


