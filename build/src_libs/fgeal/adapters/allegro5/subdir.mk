################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src_libs/fgeal/adapters/allegro5/core.cpp \
../src_libs/fgeal/adapters/allegro5/display.cpp \
../src_libs/fgeal/adapters/allegro5/event.cpp \
../src_libs/fgeal/adapters/allegro5/font.cpp \
../src_libs/fgeal/adapters/allegro5/graphics.cpp \
../src_libs/fgeal/adapters/allegro5/image.cpp \
../src_libs/fgeal/adapters/allegro5/input.cpp \
../src_libs/fgeal/adapters/allegro5/sound.cpp 

OBJS += \
./src_libs/fgeal/adapters/allegro5/core.o \
./src_libs/fgeal/adapters/allegro5/display.o \
./src_libs/fgeal/adapters/allegro5/event.o \
./src_libs/fgeal/adapters/allegro5/font.o \
./src_libs/fgeal/adapters/allegro5/graphics.o \
./src_libs/fgeal/adapters/allegro5/image.o \
./src_libs/fgeal/adapters/allegro5/input.o \
./src_libs/fgeal/adapters/allegro5/sound.o 

CPP_DEPS += \
./src_libs/fgeal/adapters/allegro5/core.d \
./src_libs/fgeal/adapters/allegro5/display.d \
./src_libs/fgeal/adapters/allegro5/event.d \
./src_libs/fgeal/adapters/allegro5/font.d \
./src_libs/fgeal/adapters/allegro5/graphics.d \
./src_libs/fgeal/adapters/allegro5/image.d \
./src_libs/fgeal/adapters/allegro5/input.d \
./src_libs/fgeal/adapters/allegro5/sound.d 


# Each subdirectory must supply rules for building sources it contributes
src_libs/fgeal/adapters/allegro5/%.o: ../src_libs/fgeal/adapters/allegro5/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


