################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/common/source/nm_common.c 

OBJS += \
./source/common/source/nm_common.o 

C_DEPS += \
./source/common/source/nm_common.d 


# Each subdirectory must supply rules for building sources it contributes
source/common/source/%.o: ../source/common/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -DDEBUG -DCPU_MK82FN256VDC15 -DFRDM_K82F -DFREEDOM -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -I../source/FatFs -I../source/TinyWebServer -I../source/winc1500 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


