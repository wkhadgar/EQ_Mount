################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Display/Src/sh1106.c 

OBJS += \
./Drivers/Display/Src/sh1106.o 

C_DEPS += \
./Drivers/Display/Src/sh1106.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Display/Src/%.o Drivers/Display/Src/%.su: ../Drivers/Display/Src/%.c Drivers/Display/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -DFONT_5X7=1 -c -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Include -I../Drivers/Display/Inc -I../Drivers/Steppers/Inc -I../Drivers/GNSS/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Display-2f-Src

clean-Drivers-2f-Display-2f-Src:
	-$(RM) ./Drivers/Display/Src/sh1106.d ./Drivers/Display/Src/sh1106.o ./Drivers/Display/Src/sh1106.su

.PHONY: clean-Drivers-2f-Display-2f-Src

