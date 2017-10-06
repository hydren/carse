################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/carse_game.cpp \
../src/course.cpp \
../src/main.cpp \
../src/vehicle.cpp 

OBJS += \
./src/carse_game.o \
./src/course.o \
./src/main.o \
./src/vehicle.o 

CPP_DEPS += \
./src/carse_game.d \
./src/course.d \
./src/main.d \
./src/vehicle.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


