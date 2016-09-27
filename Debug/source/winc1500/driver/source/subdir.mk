################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/winc1500/driver/source/m2m_ate_mode.c \
../source/winc1500/driver/source/m2m_hif.c \
../source/winc1500/driver/source/m2m_ota.c \
../source/winc1500/driver/source/m2m_periph.c \
../source/winc1500/driver/source/m2m_wifi.c \
../source/winc1500/driver/source/nmasic.c \
../source/winc1500/driver/source/nmbus.c \
../source/winc1500/driver/source/nmdrv.c \
../source/winc1500/driver/source/nmi2c.c \
../source/winc1500/driver/source/nmspi.c \
../source/winc1500/driver/source/nmuart.c 

OBJS += \
./source/winc1500/driver/source/m2m_ate_mode.o \
./source/winc1500/driver/source/m2m_hif.o \
./source/winc1500/driver/source/m2m_ota.o \
./source/winc1500/driver/source/m2m_periph.o \
./source/winc1500/driver/source/m2m_wifi.o \
./source/winc1500/driver/source/nmasic.o \
./source/winc1500/driver/source/nmbus.o \
./source/winc1500/driver/source/nmdrv.o \
./source/winc1500/driver/source/nmi2c.o \
./source/winc1500/driver/source/nmspi.o \
./source/winc1500/driver/source/nmuart.o 

C_DEPS += \
./source/winc1500/driver/source/m2m_ate_mode.d \
./source/winc1500/driver/source/m2m_hif.d \
./source/winc1500/driver/source/m2m_ota.d \
./source/winc1500/driver/source/m2m_periph.d \
./source/winc1500/driver/source/m2m_wifi.d \
./source/winc1500/driver/source/nmasic.d \
./source/winc1500/driver/source/nmbus.d \
./source/winc1500/driver/source/nmdrv.d \
./source/winc1500/driver/source/nmi2c.d \
./source/winc1500/driver/source/nmspi.d \
./source/winc1500/driver/source/nmuart.d 


# Each subdirectory must supply rules for building sources it contributes
source/winc1500/driver/source/%.o: ../source/winc1500/driver/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK82FN256VDC15" -DOV7670 -D__FPU_PRESENT -DARM_MATH_CM4 -DARM_MATH_CM4 -DNOSIPSERVER -DHAVE_CONFIG_H -DOSIP_MONOTHREAD -DWINC1500 -I../source/cameraLib -I../source/Base64 -I/home/mhanuel/Devel/Networking/ArduinoJson/include -I../source/winc1500_firmware_updater -I../source/Wire -I../source/artik_cloud -I../dma_manager -I../source/audioLib -I../source/cameraLib/encoder -I../source/Firmata -I../source/Firmata/utility -I../source/FatFs -I../source/cameraLib/ov7670 -I../source/WebSockets -I../source/jrtplib -I../source/include -I../source/winc1500 -I/home/chroots/arm-devel/usr/local/lib/include/ -I../source/TinyWebServer -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


