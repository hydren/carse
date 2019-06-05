################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src_libs/fgeal/adapters/sdl/core.cpp \
../src_libs/fgeal/adapters/sdl/display.cpp \
../src_libs/fgeal/adapters/sdl/event.cpp \
../src_libs/fgeal/adapters/sdl/font.cpp \
../src_libs/fgeal/adapters/sdl/graphics.cpp \
../src_libs/fgeal/adapters/sdl/image.cpp \
../src_libs/fgeal/adapters/sdl/input.cpp \
../src_libs/fgeal/adapters/sdl/sound.cpp 

OBJS += \
./src_libs/fgeal/adapters/sdl/core.o \
./src_libs/fgeal/adapters/sdl/display.o \
./src_libs/fgeal/adapters/sdl/event.o \
./src_libs/fgeal/adapters/sdl/font.o \
./src_libs/fgeal/adapters/sdl/graphics.o \
./src_libs/fgeal/adapters/sdl/image.o \
./src_libs/fgeal/adapters/sdl/input.o \
./src_libs/fgeal/adapters/sdl/sound.o 

CPP_DEPS += \
./src_libs/fgeal/adapters/sdl/core.d \
./src_libs/fgeal/adapters/sdl/display.d \
./src_libs/fgeal/adapters/sdl/event.d \
./src_libs/fgeal/adapters/sdl/font.d \
./src_libs/fgeal/adapters/sdl/graphics.d \
./src_libs/fgeal/adapters/sdl/image.d \
./src_libs/fgeal/adapters/sdl/input.d \
./src_libs/fgeal/adapters/sdl/sound.d 


# Each subdirectory must supply rules for building sources it contributes
src_libs/fgeal/adapters/sdl/%.o: ../src_libs/fgeal/adapters/sdl/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


