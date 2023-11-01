################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Cube++/Core/Command.cpp \
../Cube++/Core/CubeUtils.cpp \
../Cube++/Core/Mutex.cpp \
../Cube++/Core/Queue.cpp \
../Cube++/Core/Task.cpp \
../Cube++/Core/Timer.cpp 

OBJS += \
./Cube++/Core/Command.o \
./Cube++/Core/CubeUtils.o \
./Cube++/Core/Mutex.o \
./Cube++/Core/Queue.o \
./Cube++/Core/Task.o \
./Cube++/Core/Timer.o 

CPP_DEPS += \
./Cube++/Core/Command.d \
./Cube++/Core/CubeUtils.d \
./Cube++/Core/Mutex.d \
./Cube++/Core/Queue.d \
./Cube++/Core/Task.d \
./Cube++/Core/Timer.d 


# Each subdirectory must supply rules for building sources it contributes
Cube++/Core/%.o Cube++/Core/%.su Cube++/Core/%.cyclo: ../Cube++/Core/%.cpp Cube++/Core/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++20 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L152xE -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Libraries/embedded-template-library/include" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Core/Inc" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Drivers/Inc" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Components" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Components/Core/Inc" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wno-volatile -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Cube-2b--2b--2f-Core

clean-Cube-2b--2b--2f-Core:
	-$(RM) ./Cube++/Core/Command.cyclo ./Cube++/Core/Command.d ./Cube++/Core/Command.o ./Cube++/Core/Command.su ./Cube++/Core/CubeUtils.cyclo ./Cube++/Core/CubeUtils.d ./Cube++/Core/CubeUtils.o ./Cube++/Core/CubeUtils.su ./Cube++/Core/Mutex.cyclo ./Cube++/Core/Mutex.d ./Cube++/Core/Mutex.o ./Cube++/Core/Mutex.su ./Cube++/Core/Queue.cyclo ./Cube++/Core/Queue.d ./Cube++/Core/Queue.o ./Cube++/Core/Queue.su ./Cube++/Core/Task.cyclo ./Cube++/Core/Task.d ./Cube++/Core/Task.o ./Cube++/Core/Task.su ./Cube++/Core/Timer.cyclo ./Cube++/Core/Timer.d ./Cube++/Core/Timer.o ./Cube++/Core/Timer.su

.PHONY: clean-Cube-2b--2b--2f-Core

