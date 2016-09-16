################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/winc1500_firmware_updater/firmware_updater.cpp 

OBJS += \
./source/winc1500_firmware_updater/firmware_updater.o 

CPP_DEPS += \
./source/winc1500_firmware_updater/firmware_updater.d 


# Each subdirectory must supply rules for building sources it contributes
source/winc1500_firmware_updater/%.o: ../source/winc1500_firmware_updater/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -D"CPU_MK82FN256VDC15" -D__FPU_PRESENT -DWINC1500 -DARM_MATH_CM4 -I../startup -I../source/Wire -I../source/INA2XX -I../source/cameraLib -I../source/cameraLib/encoder -I../source/WebSockets -I../source/cameraLib/ov7670 -I../source/include -I../source/TinyWebServer -I../board -I../utilities -I../source/winc1500 -I../source/audioLib -I../source/winc1500_firmware_updater -I../CMSIS -I../drivers -I../source/FatFs -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


