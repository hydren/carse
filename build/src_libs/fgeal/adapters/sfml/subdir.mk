################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src_libs/fgeal/adapters/sfml/core.cpp \
../src_libs/fgeal/adapters/sfml/display.cpp \
../src_libs/fgeal/adapters/sfml/event.cpp \
../src_libs/fgeal/adapters/sfml/font.cpp \
../src_libs/fgeal/adapters/sfml/graphics.cpp \
../src_libs/fgeal/adapters/sfml/image.cpp \
../src_libs/fgeal/adapters/sfml/input.cpp \
../src_libs/fgeal/adapters/sfml/sound.cpp 

OBJS += \
./src_libs/fgeal/adapters/sfml/core.o \
./src_libs/fgeal/adapters/sfml/display.o \
./src_libs/fgeal/adapters/sfml/event.o \
./src_libs/fgeal/adapters/sfml/font.o \
./src_libs/fgeal/adapters/sfml/graphics.o \
./src_libs/fgeal/adapters/sfml/image.o \
./src_libs/fgeal/adapters/sfml/input.o \
./src_libs/fgeal/adapters/sfml/sound.o 

CPP_DEPS += \
./src_libs/fgeal/adapters/sfml/core.d \
./src_libs/fgeal/adapters/sfml/display.d \
./src_libs/fgeal/adapters/sfml/event.d \
./src_libs/fgeal/adapters/sfml/font.d \
./src_libs/fgeal/adapters/sfml/graphics.d \
./src_libs/fgeal/adapters/sfml/image.d \
./src_libs/fgeal/adapters/sfml/input.d \
./src_libs/fgeal/adapters/sfml/sound.d 


# Each subdirectory must supply rules for building sources it contributes
src_libs/fgeal/adapters/sfml/%.o: ../src_libs/fgeal/adapters/sfml/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"BUILD_PATH/src" -I"BUILD_PATH/src_libs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


