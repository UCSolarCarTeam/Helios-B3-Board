################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32l152retx.s 

S_DEPS += \
./Core/Startup/startup_stm32l152retx.d 

OBJS += \
./Core/Startup/startup_stm32l152retx.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m3 -g3 -DDEBUG -c -I"/Users/cjchan/Desktop/SC/Helios-B3-Board/Cube++/Libraries/embedded-template-library/include" -I"/Users/cjchan/Desktop/SC/Helios-B3-Board/Cube++" -I"/Users/cjchan/Desktop/SC/Helios-B3-Board/Cube++/Core/Inc" -I"/Users/cjchan/Desktop/SC/Helios-B3-Board/Cube++/Drivers/Inc" -I"/Users/cjchan/Desktop/SC/Helios-B3-Board/Components" -I"/Users/cjchan/Desktop/SC/Helios-B3-Board/Components/Core/Inc" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32l152retx.d ./Core/Startup/startup_stm32l152retx.o

.PHONY: clean-Core-2f-Startup
