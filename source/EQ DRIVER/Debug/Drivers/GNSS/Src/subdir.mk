################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/GNSS/Src/PA6H.c 

OBJS += \
./Drivers/GNSS/Src/PA6H.o 

C_DEPS += \
./Drivers/GNSS/Src/PA6H.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/GNSS/Src/%.o Drivers/GNSS/Src/%.su: ../Drivers/GNSS/Src/%.c Drivers/GNSS/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/paulo/CLionProjects/EQmount_custom_controller/source/EQ DRIVER/Drivers/GNSS/Inc" -I"C:/Users/paulo/CLionProjects/EQmount_custom_controller/source/EQ DRIVER/Drivers/Steppers/Inc" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-GNSS-2f-Src

clean-Drivers-2f-GNSS-2f-Src:
	-$(RM) ./Drivers/GNSS/Src/PA6H.d ./Drivers/GNSS/Src/PA6H.o ./Drivers/GNSS/Src/PA6H.su

.PHONY: clean-Drivers-2f-GNSS-2f-Src

