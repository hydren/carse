################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/states/course_selection_state.cpp \
../src/states/main_menu_state.cpp \
../src/states/options_menu_state.cpp \
../src/states/race_state.cpp \
../src/states/vehicle_selection_state.cpp 

OBJS += \
./src/states/course_selection_state.o \
./src/states/main_menu_state.o \
./src/states/options_menu_state.o \
./src/states/race_state.o \
./src/states/vehicle_selection_state.o 

CPP_DEPS += \
./src/states/course_selection_state.d \
./src/states/main_menu_state.d \
./src/states/options_menu_state.d \
./src/states/race_state.d \
./src/states/vehicle_selection_state.d 


# Each subdirectory must supply rules for building sources it contributes
src/states/%.o: ../src/states/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


