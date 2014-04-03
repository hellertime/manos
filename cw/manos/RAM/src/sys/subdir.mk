################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../src/sys/close.c" \
"../src/sys/dirread.c" \
"../src/sys/execv.c" \
"../src/sys/getInfo.c" \
"../src/sys/getcwd.c" \
"../src/sys/open.c" \
"../src/sys/read.c" \
"../src/sys/walk.c" \

C_SRCS += \
../src/sys/close.c \
../src/sys/dirread.c \
../src/sys/execv.c \
../src/sys/getInfo.c \
../src/sys/getcwd.c \
../src/sys/open.c \
../src/sys/read.c \
../src/sys/walk.c \

OBJS += \
./src/sys/close.o \
./src/sys/dirread.o \
./src/sys/execv.o \
./src/sys/getInfo.o \
./src/sys/getcwd.o \
./src/sys/open.o \
./src/sys/read.o \
./src/sys/walk.o \

C_DEPS += \
./src/sys/close.d \
./src/sys/dirread.d \
./src/sys/execv.d \
./src/sys/getInfo.d \
./src/sys/getcwd.d \
./src/sys/open.d \
./src/sys/read.d \
./src/sys/walk.d \

OBJS_QUOTED += \
"./src/sys/close.o" \
"./src/sys/dirread.o" \
"./src/sys/execv.o" \
"./src/sys/getInfo.o" \
"./src/sys/getcwd.o" \
"./src/sys/open.o" \
"./src/sys/read.o" \
"./src/sys/walk.o" \

C_DEPS_QUOTED += \
"./src/sys/close.d" \
"./src/sys/dirread.d" \
"./src/sys/execv.d" \
"./src/sys/getInfo.d" \
"./src/sys/getcwd.d" \
"./src/sys/open.d" \
"./src/sys/read.d" \
"./src/sys/walk.d" \

OBJS_OS_FORMAT += \
./src/sys/close.o \
./src/sys/dirread.o \
./src/sys/execv.o \
./src/sys/getInfo.o \
./src/sys/getcwd.o \
./src/sys/open.o \
./src/sys/read.o \
./src/sys/walk.o \


# Each subdirectory must supply rules for building sources it contributes
src/sys/close.o: ../src/sys/close.c
	@echo 'Building file: $<'
	@echo 'Executing target #13 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/close.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/close.o"
	@echo 'Finished building: $<'
	@echo ' '

src/sys/dirread.o: ../src/sys/dirread.c
	@echo 'Building file: $<'
	@echo 'Executing target #14 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/dirread.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/dirread.o"
	@echo 'Finished building: $<'
	@echo ' '

src/sys/execv.o: ../src/sys/execv.c
	@echo 'Building file: $<'
	@echo 'Executing target #15 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/execv.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/execv.o"
	@echo 'Finished building: $<'
	@echo ' '

src/sys/getInfo.o: ../src/sys/getInfo.c
	@echo 'Building file: $<'
	@echo 'Executing target #16 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/getInfo.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/getInfo.o"
	@echo 'Finished building: $<'
	@echo ' '

src/sys/getcwd.o: ../src/sys/getcwd.c
	@echo 'Building file: $<'
	@echo 'Executing target #17 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/getcwd.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/getcwd.o"
	@echo 'Finished building: $<'
	@echo ' '

src/sys/open.o: ../src/sys/open.c
	@echo 'Building file: $<'
	@echo 'Executing target #18 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/open.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/open.o"
	@echo 'Finished building: $<'
	@echo ' '

src/sys/read.o: ../src/sys/read.c
	@echo 'Building file: $<'
	@echo 'Executing target #19 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/read.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/read.o"
	@echo 'Finished building: $<'
	@echo ' '

src/sys/walk.o: ../src/sys/walk.c
	@echo 'Building file: $<'
	@echo 'Executing target #20 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/sys/walk.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/sys/walk.o"
	@echo 'Finished building: $<'
	@echo ' '


