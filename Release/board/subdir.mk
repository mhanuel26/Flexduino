################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c \
../board/pin_mux.c 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/pin_mux.o 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -D"CPU_MK82FN256VDC15" -D__FPU_PRESENT -DARM_MATH_CM4 -DWINC1500 -I../startup -I../source/Wire -I../source/INA2XX -I../source/cameraLib/encoder -I../source/audioLib -I../source/include -I../source/winc1500 -I../source/winc1500_firmware_updater -I../board -I../utilities -I../source/cameraLib/ov7670 -I../source/cameraLib -I../CMSIS -I../source/WebSockets -I../source/TinyWebServer -I../source/FatFs -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


