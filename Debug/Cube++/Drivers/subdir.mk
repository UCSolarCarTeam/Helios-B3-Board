################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Cube++/Drivers/UARTDriver.cpp 

OBJS += \
./Cube++/Drivers/UARTDriver.o 

CPP_DEPS += \
./Cube++/Drivers/UARTDriver.d 


# Each subdirectory must supply rules for building sources it contributes
Cube++/Drivers/%.o Cube++/Drivers/%.su Cube++/Drivers/%.cyclo: ../Cube++/Drivers/%.cpp Cube++/Drivers/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++20 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L152xE -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Libraries/embedded-template-library/include" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Core/Inc" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Drivers/Inc" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Components" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Components/Core/Inc" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wno-volatile -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Cube-2b--2b--2f-Drivers

clean-Cube-2b--2b--2f-Drivers:
	-$(RM) ./Cube++/Drivers/UARTDriver.cyclo ./Cube++/Drivers/UARTDriver.d ./Cube++/Drivers/UARTDriver.o ./Cube++/Drivers/UARTDriver.su

.PHONY: clean-Cube-2b--2b--2f-Drivers

