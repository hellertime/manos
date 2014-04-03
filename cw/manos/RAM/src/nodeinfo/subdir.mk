################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"$(NODEINFO)/mkNodeInfo.c" \

C_SRCS += \
$(NODEINFO)/mkNodeInfo.c \

OBJS += \
./src/nodeinfo/mkNodeInfo.o \

C_DEPS += \
./src/nodeinfo/mkNodeInfo.d \

OBJS_QUOTED += \
"./src/nodeinfo/mkNodeInfo.o" \

C_DEPS_QUOTED += \
"./src/nodeinfo/mkNodeInfo.d" \

OBJS_OS_FORMAT += \
./src/nodeinfo/mkNodeInfo.o \


# Each subdirectory must supply rules for building sources it contributes
src/nodeinfo/mkNodeInfo.o: $(NODEINFO)/mkNodeInfo.c
	@echo 'Building file: $<'
	@echo 'Executing target #26 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/nodeinfo/mkNodeInfo.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/nodeinfo/mkNodeInfo.o"
	@echo 'Finished building: $<'
	@echo ' '


