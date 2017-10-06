################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/automotive/engine_sound.cpp \
../src/automotive/motor.cpp 

OBJS += \
./src/automotive/engine_sound.o \
./src/automotive/motor.o 

CPP_DEPS += \
./src/automotive/engine_sound.d \
./src/automotive/motor.d 


# Each subdirectory must supply rules for building sources it contributes
src/automotive/%.o: ../src/automotive/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


