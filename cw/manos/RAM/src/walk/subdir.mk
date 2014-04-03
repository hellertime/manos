################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../src/walk/emptyWalkTrail.c" \
"../src/walk/freeWalkTrail.c" \
"../src/walk/genericWalk.c" \
"../src/walk/getNodeInfoStaticNS.c" \
"../src/walk/readStaticNS.c" \

C_SRCS += \
../src/walk/emptyWalkTrail.c \
../src/walk/freeWalkTrail.c \
../src/walk/genericWalk.c \
../src/walk/getNodeInfoStaticNS.c \
../src/walk/readStaticNS.c \

OBJS += \
./src/walk/emptyWalkTrail.o \
./src/walk/freeWalkTrail.o \
./src/walk/genericWalk.o \
./src/walk/getNodeInfoStaticNS.o \
./src/walk/readStaticNS.o \

C_DEPS += \
./src/walk/emptyWalkTrail.d \
./src/walk/freeWalkTrail.d \
./src/walk/genericWalk.d \
./src/walk/getNodeInfoStaticNS.d \
./src/walk/readStaticNS.d \

OBJS_QUOTED += \
"./src/walk/emptyWalkTrail.o" \
"./src/walk/freeWalkTrail.o" \
"./src/walk/genericWalk.o" \
"./src/walk/getNodeInfoStaticNS.o" \
"./src/walk/readStaticNS.o" \

C_DEPS_QUOTED += \
"./src/walk/emptyWalkTrail.d" \
"./src/walk/freeWalkTrail.d" \
"./src/walk/genericWalk.d" \
"./src/walk/getNodeInfoStaticNS.d" \
"./src/walk/readStaticNS.d" \

OBJS_OS_FORMAT += \
./src/walk/emptyWalkTrail.o \
./src/walk/freeWalkTrail.o \
./src/walk/genericWalk.o \
./src/walk/getNodeInfoStaticNS.o \
./src/walk/readStaticNS.o \


# Each subdirectory must supply rules for building sources it contributes
src/walk/emptyWalkTrail.o: ../src/walk/emptyWalkTrail.c
	@echo 'Building file: $<'
	@echo 'Executing target #1 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/walk/emptyWalkTrail.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/walk/emptyWalkTrail.o"
	@echo 'Finished building: $<'
	@echo ' '

src/walk/freeWalkTrail.o: ../src/walk/freeWalkTrail.c
	@echo 'Building file: $<'
	@echo 'Executing target #2 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/walk/freeWalkTrail.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/walk/freeWalkTrail.o"
	@echo 'Finished building: $<'
	@echo ' '

src/walk/genericWalk.o: ../src/walk/genericWalk.c
	@echo 'Building file: $<'
	@echo 'Executing target #3 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/walk/genericWalk.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/walk/genericWalk.o"
	@echo 'Finished building: $<'
	@echo ' '

src/walk/getNodeInfoStaticNS.o: ../src/walk/getNodeInfoStaticNS.c
	@echo 'Building file: $<'
	@echo 'Executing target #4 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/walk/getNodeInfoStaticNS.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/walk/getNodeInfoStaticNS.o"
	@echo 'Finished building: $<'
	@echo ' '

src/walk/readStaticNS.o: ../src/walk/readStaticNS.c
	@echo 'Building file: $<'
	@echo 'Executing target #5 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/walk/readStaticNS.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/walk/readStaticNS.o"
	@echo 'Finished building: $<'
	@echo ' '


