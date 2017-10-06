################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/pseudo3d/vehicle_gfx.cpp 

OBJS += \
./src/pseudo3d/vehicle_gfx.o 

CPP_DEPS += \
./src/pseudo3d/vehicle_gfx.d 


# Each subdirectory must supply rules for building sources it contributes
src/pseudo3d/%.o: ../src/pseudo3d/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


