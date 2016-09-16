################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/winc1500/bus_wrapper/source/nm_bus_wrapper_mk82f.c 

OBJS += \
./source/winc1500/bus_wrapper/source/nm_bus_wrapper_mk82f.o 

C_DEPS += \
./source/winc1500/bus_wrapper/source/nm_bus_wrapper_mk82f.d 


# Each subdirectory must supply rules for building sources it contributes
source/winc1500/bus_wrapper/source/%.o: ../source/winc1500/bus_wrapper/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK82FN256VDC15" -DNO_OV7670 -D__FPU_PRESENT -DARM_MATH_CM4 -DARM_MATH_CM4 -DNOSIPSERVER -DHAVE_CONFIG_H -DOSIP_MONOTHREAD -DWINC1500 -I../source/cameraLib -I/home/mhanuel/Devel/Networking/ArduinoJson/include -I../source/winc1500_firmware_updater -I../source/Wire -I../dma_manager -I../source/audioLib -I../source/cameraLib/encoder -I../source/FatFs -I../source/cameraLib/ov7670 -I../source/WebSockets -I../source/jrtplib -I../source/include -I../source/winc1500 -I/home/chroots/arm-devel/usr/local/lib/include/ -I../source/TinyWebServer -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


