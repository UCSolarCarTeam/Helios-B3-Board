################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Components/Core/RunInterface.cpp 

OBJS += \
./Components/Core/RunInterface.o 

CPP_DEPS += \
./Components/Core/RunInterface.d 


# Each subdirectory must supply rules for building sources it contributes
Components/Core/%.o Components/Core/%.su Components/Core/%.cyclo: ../Components/Core/%.cpp Components/Core/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++20 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L152xE -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Libraries/embedded-template-library/include" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Core/Inc" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Cube++/Drivers/Inc" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Components" -I"C:/Users/Chris/iCloudDrive/Desktop/SC/Helios-B3-Board/Components/Core/Inc" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wno-volatile -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Components-2f-Core

clean-Components-2f-Core:
	-$(RM) ./Components/Core/RunInterface.cyclo ./Components/Core/RunInterface.d ./Components/Core/RunInterface.o ./Components/Core/RunInterface.su

.PHONY: clean-Components-2f-Core

