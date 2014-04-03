################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../src/portal/clonePortal.c" \
"../src/portal/closePortal.c" \
"../src/portal/mkPortal.c" \

C_SRCS += \
../src/portal/clonePortal.c \
../src/portal/closePortal.c \
../src/portal/mkPortal.c \

OBJS += \
./src/portal/clonePortal.o \
./src/portal/closePortal.o \
./src/portal/mkPortal.o \

C_DEPS += \
./src/portal/clonePortal.d \
./src/portal/closePortal.d \
./src/portal/mkPortal.d \

OBJS_QUOTED += \
"./src/portal/clonePortal.o" \
"./src/portal/closePortal.o" \
"./src/portal/mkPortal.o" \

C_DEPS_QUOTED += \
"./src/portal/clonePortal.d" \
"./src/portal/closePortal.d" \
"./src/portal/mkPortal.d" \

OBJS_OS_FORMAT += \
./src/portal/clonePortal.o \
./src/portal/closePortal.o \
./src/portal/mkPortal.o \


# Each subdirectory must supply rules for building sources it contributes
src/portal/clonePortal.o: ../src/portal/clonePortal.c
	@echo 'Building file: $<'
	@echo 'Executing target #22 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/portal/clonePortal.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/portal/clonePortal.o"
	@echo 'Finished building: $<'
	@echo ' '

src/portal/closePortal.o: ../src/portal/closePortal.c
	@echo 'Building file: $<'
	@echo 'Executing target #23 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/portal/closePortal.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/portal/closePortal.o"
	@echo 'Finished building: $<'
	@echo ' '

src/portal/mkPortal.o: ../src/portal/mkPortal.c
	@echo 'Building file: $<'
	@echo 'Executing target #24 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/portal/mkPortal.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/portal/mkPortal.o"
	@echo 'Finished building: $<'
	@echo ' '


