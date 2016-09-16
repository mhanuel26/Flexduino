################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/socket/source/socket.c \
../source/socket/source/socket_buffer.c 

OBJS += \
./source/socket/source/socket.o \
./source/socket/source/socket_buffer.o 

C_DEPS += \
./source/socket/source/socket.d \
./source/socket/source/socket_buffer.d 


# Each subdirectory must supply rules for building sources it contributes
source/socket/source/%.o: ../source/socket/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -DDEBUG -DCPU_MK82FN256VDC15 -DFRDM_K82F -DFREEDOM -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -I../source/FatFs -I../source/TinyWebServer -I../source/winc1500 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


