################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/bus_wrapper/source/nm_bus_wrapper_mk82f.c 

OBJS += \
./source/bus_wrapper/source/nm_bus_wrapper_mk82f.o 

C_DEPS += \
./source/bus_wrapper/source/nm_bus_wrapper_mk82f.d 


# Each subdirectory must supply rules for building sources it contributes
source/bus_wrapper/source/%.o: ../source/bus_wrapper/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -DDEBUG -DCPU_MK82FN256VDC15 -DFRDM_K82F -DFREEDOM -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -I../source/FatFs -I../source/TinyWebServer -I../source/winc1500 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


