################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/carse_game.cpp \
../src/carse_game_logic.cpp \
../src/carse_game_properties_io.cpp \
../src/carse_game_shared_resources.cpp \
../src/course.cpp \
../src/course_editor_state.cpp \
../src/course_parser.cpp \
../src/course_selection_state.cpp \
../src/main.cpp \
../src/main_menu_classic_layout_state.cpp \
../src/main_menu_simple_list_state.cpp \
../src/options_menu_state.cpp \
../src/pseudo3d_race_state.cpp \
../src/pseudo3d_race_state_physics.cpp \
../src/vehicle.cpp \
../src/vehicle_selection_showroom_layout_state.cpp \
../src/vehicle_selection_simple_list_state.cpp \
../src/vehicle_spec_parser.cpp 

OBJS += \
./src/carse_game.o \
./src/carse_game_logic.o \
./src/carse_game_properties_io.o \
./src/carse_game_shared_resources.o \
./src/course.o \
./src/course_editor_state.o \
./src/course_parser.o \
./src/course_selection_state.o \
./src/main.o \
./src/main_menu_classic_layout_state.o \
./src/main_menu_simple_list_state.o \
./src/options_menu_state.o \
./src/pseudo3d_race_state.o \
./src/pseudo3d_race_state_physics.o \
./src/vehicle.o \
./src/vehicle_selection_showroom_layout_state.o \
./src/vehicle_selection_simple_list_state.o \
./src/vehicle_spec_parser.o 

CPP_DEPS += \
./src/carse_game.d \
./src/carse_game_logic.d \
./src/carse_game_properties_io.d \
./src/carse_game_shared_resources.d \
./src/course.d \
./src/course_editor_state.d \
./src/course_parser.d \
./src/course_selection_state.d \
./src/main.d \
./src/main_menu_classic_layout_state.d \
./src/main_menu_simple_list_state.d \
./src/options_menu_state.d \
./src/pseudo3d_race_state.d \
./src/pseudo3d_race_state_physics.d \
./src/vehicle.d \
./src/vehicle_selection_showroom_layout_state.d \
./src/vehicle_selection_simple_list_state.d \
./src/vehicle_spec_parser.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


