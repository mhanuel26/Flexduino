################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/audioLib/AudioFlex.cpp 

C_SRCS += \
../source/audioLib/audio_dma.c \
../source/audioLib/pdm2pcm.c 

OBJS += \
./source/audioLib/AudioFlex.o \
./source/audioLib/audio_dma.o \
./source/audioLib/pdm2pcm.o 

C_DEPS += \
./source/audioLib/audio_dma.d \
./source/audioLib/pdm2pcm.d 

CPP_DEPS += \
./source/audioLib/AudioFlex.d 


# Each subdirectory must supply rules for building sources it contributes
source/audioLib/%.o: ../source/audioLib/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -D"CPU_MK82FN256VDC15" -D__FPU_PRESENT -DWINC1500 -DARM_MATH_CM4 -I../startup -I../source/Wire -I../source/INA2XX -I../source/cameraLib -I../source/cameraLib/encoder -I../source/WebSockets -I../source/cameraLib/ov7670 -I../source/include -I../source/TinyWebServer -I../board -I../utilities -I../source/winc1500 -I../source/audioLib -I../source/winc1500_firmware_updater -I../CMSIS -I../drivers -I../source/FatFs -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/audioLib/%.o: ../source/audioLib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -D"CPU_MK82FN256VDC15" -D__FPU_PRESENT -DARM_MATH_CM4 -DWINC1500 -I../startup -I../source/Wire -I../source/INA2XX -I../source/cameraLib/encoder -I../source/audioLib -I../source/include -I../source/winc1500 -I../source/winc1500_firmware_updater -I../board -I../utilities -I../source/cameraLib/ov7670 -I../source/cameraLib -I../CMSIS -I../source/WebSockets -I../source/TinyWebServer -I../source/FatFs -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


