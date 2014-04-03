################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../src/dev/dev.c" \
"../src/dev/devled.c" \
"../src/dev/devroot.c" \
"../src/dev/devswpb.c" \
"../src/dev/fromDeviceId.c" \

C_SRCS += \
../src/dev/dev.c \
../src/dev/devled.c \
../src/dev/devroot.c \
../src/dev/devswpb.c \
../src/dev/fromDeviceId.c \

OBJS += \
./src/dev/dev.o \
./src/dev/devled.o \
./src/dev/devroot.o \
./src/dev/devswpb.o \
./src/dev/fromDeviceId.o \

C_DEPS += \
./src/dev/dev.d \
./src/dev/devled.d \
./src/dev/devroot.d \
./src/dev/devswpb.d \
./src/dev/fromDeviceId.d \

OBJS_QUOTED += \
"./src/dev/dev.o" \
"./src/dev/devled.o" \
"./src/dev/devroot.o" \
"./src/dev/devswpb.o" \
"./src/dev/fromDeviceId.o" \

C_DEPS_QUOTED += \
"./src/dev/dev.d" \
"./src/dev/devled.d" \
"./src/dev/devroot.d" \
"./src/dev/devswpb.d" \
"./src/dev/fromDeviceId.d" \

OBJS_OS_FORMAT += \
./src/dev/dev.o \
./src/dev/devled.o \
./src/dev/devroot.o \
./src/dev/devswpb.o \
./src/dev/fromDeviceId.o \


# Each subdirectory must supply rules for building sources it contributes
src/dev/dev.o: ../src/dev/dev.c
	@echo 'Building file: $<'
	@echo 'Executing target #28 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/dev/dev.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/dev/dev.o"
	@echo 'Finished building: $<'
	@echo ' '

src/dev/devled.o: ../src/dev/devled.c
	@echo 'Building file: $<'
	@echo 'Executing target #29 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/dev/devled.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/dev/devled.o"
	@echo 'Finished building: $<'
	@echo ' '

src/dev/devroot.o: ../src/dev/devroot.c
	@echo 'Building file: $<'
	@echo 'Executing target #30 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/dev/devroot.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/dev/devroot.o"
	@echo 'Finished building: $<'
	@echo ' '

src/dev/devswpb.o: ../src/dev/devswpb.c
	@echo 'Building file: $<'
	@echo 'Executing target #31 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/dev/devswpb.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/dev/devswpb.o"
	@echo 'Finished building: $<'
	@echo ' '

src/dev/fromDeviceId.o: ../src/dev/fromDeviceId.c
	@echo 'Building file: $<'
	@echo 'Executing target #32 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/dev/fromDeviceId.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/dev/fromDeviceId.o"
	@echo 'Finished building: $<'
	@echo ' '


