################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/FatFs/option/ccsbcs.c \
../source/FatFs/option/syscall.c \
../source/FatFs/option/unicode.c 

OBJS += \
./source/FatFs/option/ccsbcs.o \
./source/FatFs/option/syscall.o \
./source/FatFs/option/unicode.o 

C_DEPS += \
./source/FatFs/option/ccsbcs.d \
./source/FatFs/option/syscall.d \
./source/FatFs/option/unicode.d 


# Each subdirectory must supply rules for building sources it contributes
source/FatFs/option/%.o: ../source/FatFs/option/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK82FN256VDC15" -DNO_OV7670 -D__FPU_PRESENT -DARM_MATH_CM4 -DARM_MATH_CM4 -DNOSIPSERVER -DHAVE_CONFIG_H -DOSIP_MONOTHREAD -DWINC1500 -I../source/cameraLib -I/home/mhanuel/Devel/Networking/ArduinoJson/include -I../source/winc1500_firmware_updater -I../source/Wire -I../source/artik_cloud -I../dma_manager -I../source/audioLib -I../source/cameraLib/encoder -I../source/Firmata -I../source/Firmata/utility -I../source/FatFs -I../source/cameraLib/ov7670 -I../source/WebSockets -I../source/jrtplib -I../source/include -I../source/winc1500 -I/home/chroots/arm-devel/usr/local/lib/include/ -I../source/TinyWebServer -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


