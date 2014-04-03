################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"$(CRUMB)/mkCrumb.c" \
"$(CRUMB)/popCrumb.c" \
"$(CRUMB)/pushCrumb.c" \
"$(CRUMB)/topCrumb.c" \

C_SRCS += \
$(CRUMB)/mkCrumb.c \
$(CRUMB)/popCrumb.c \
$(CRUMB)/pushCrumb.c \
$(CRUMB)/topCrumb.c \

OBJS += \
./src/crumb/mkCrumb.o \
./src/crumb/popCrumb.o \
./src/crumb/pushCrumb.o \
./src/crumb/topCrumb.o \

C_DEPS += \
./src/crumb/mkCrumb.d \
./src/crumb/popCrumb.d \
./src/crumb/pushCrumb.d \
./src/crumb/topCrumb.d \

OBJS_QUOTED += \
"./src/crumb/mkCrumb.o" \
"./src/crumb/popCrumb.o" \
"./src/crumb/pushCrumb.o" \
"./src/crumb/topCrumb.o" \

C_DEPS_QUOTED += \
"./src/crumb/mkCrumb.d" \
"./src/crumb/popCrumb.d" \
"./src/crumb/pushCrumb.d" \
"./src/crumb/topCrumb.d" \

OBJS_OS_FORMAT += \
./src/crumb/mkCrumb.o \
./src/crumb/popCrumb.o \
./src/crumb/pushCrumb.o \
./src/crumb/topCrumb.o \


# Each subdirectory must supply rules for building sources it contributes
src/crumb/mkCrumb.o: $(CRUMB)/mkCrumb.c
	@echo 'Building file: $<'
	@echo 'Executing target #33 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/crumb/mkCrumb.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/crumb/mkCrumb.o"
	@echo 'Finished building: $<'
	@echo ' '

src/crumb/popCrumb.o: $(CRUMB)/popCrumb.c
	@echo 'Building file: $<'
	@echo 'Executing target #34 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/crumb/popCrumb.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/crumb/popCrumb.o"
	@echo 'Finished building: $<'
	@echo ' '

src/crumb/pushCrumb.o: $(CRUMB)/pushCrumb.c
	@echo 'Building file: $<'
	@echo 'Executing target #35 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/crumb/pushCrumb.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/crumb/pushCrumb.o"
	@echo 'Finished building: $<'
	@echo ' '

src/crumb/topCrumb.o: $(CRUMB)/topCrumb.c
	@echo 'Building file: $<'
	@echo 'Executing target #36 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"src/crumb/topCrumb.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"src/crumb/topCrumb.o"
	@echo 'Finished building: $<'
	@echo ' '


