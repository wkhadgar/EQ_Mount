################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Display/GFX.c \
../Drivers/Display/OLED_SH1106G.c 

OBJS += \
./Drivers/Display/GFX.o \
./Drivers/Display/OLED_SH1106G.o 

C_DEPS += \
./Drivers/Display/GFX.d \
./Drivers/Display/OLED_SH1106G.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Display/%.o Drivers/Display/%.su: ../Drivers/Display/%.c Drivers/Display/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Display

clean-Drivers-2f-Display:
	-$(RM) ./Drivers/Display/GFX.d ./Drivers/Display/GFX.o ./Drivers/Display/GFX.su ./Drivers/Display/OLED_SH1106G.d ./Drivers/Display/OLED_SH1106G.o ./Drivers/Display/OLED_SH1106G.su

.PHONY: clean-Drivers-2f-Display

