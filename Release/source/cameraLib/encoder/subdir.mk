################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/cameraLib/encoder/dct.c \
../source/cameraLib/encoder/jpegenc.c 

OBJS += \
./source/cameraLib/encoder/dct.o \
./source/cameraLib/encoder/jpegenc.o 

C_DEPS += \
./source/cameraLib/encoder/dct.d \
./source/cameraLib/encoder/jpegenc.d 


# Each subdirectory must supply rules for building sources it contributes
source/cameraLib/encoder/%.o: ../source/cameraLib/encoder/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -D"CPU_MK82FN256VDC15" -D__FPU_PRESENT -DARM_MATH_CM4 -DWINC1500 -I../startup -I../source/Wire -I../source/INA2XX -I../source/cameraLib/encoder -I../source/audioLib -I../source/include -I../source/winc1500 -I../source/winc1500_firmware_updater -I../board -I../utilities -I../source/cameraLib/ov7670 -I../source/cameraLib -I../CMSIS -I../source/WebSockets -I../source/TinyWebServer -I../source/FatFs -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


