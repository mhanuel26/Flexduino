################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/WebSockets/WebSockets.cpp \
../source/WebSockets/WebSocketsClient.cpp \
../source/WebSockets/WebSocketsServer.cpp 

OBJS += \
./source/WebSockets/WebSockets.o \
./source/WebSockets/WebSocketsClient.o \
./source/WebSockets/WebSocketsServer.o 

CPP_DEPS += \
./source/WebSockets/WebSockets.d \
./source/WebSockets/WebSocketsClient.d \
./source/WebSockets/WebSocketsServer.d 


# Each subdirectory must supply rules for building sources it contributes
source/WebSockets/%.o: ../source/WebSockets/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK82FN256VDC15" -D__FPU_PRESENT -DNOSIPSERVER -DHAVE_CONFIG_H -DOSIP_MONOTHREAD -DOV7670 -DWINC1500 -I../source/FatFs -I../source/Base64 -I../source/Firmata/utility -I../source/Firmata -I../source/artik_cloud -I/home/mhanuel/Devel/Networking/ArduinoJson/include -I../source/winc1500_firmware_updater -I../source/Wire -I../dma_manager -I../source/audioLib -I../source/cameraLib/encoder -I../source/cameraLib -I../source -I../source/cameraLib/ov7670 -I../source/WebSockets -I../source/jrtplib -I../source/include -I../source/winc1500 -I../source/TinyWebServer -I../startup -I../board -I../utilities -I/home/chroots/arm-devel/usr/local/lib/include/ -I../CMSIS -I../drivers -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


