################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/jrtplib/extratransmitters/rtpfaketransmitter.cpp 

OBJS += \
./source/jrtplib/extratransmitters/rtpfaketransmitter.o 

CPP_DEPS += \
./source/jrtplib/extratransmitters/rtpfaketransmitter.d 


# Each subdirectory must supply rules for building sources it contributes
source/jrtplib/extratransmitters/%.o: ../source/jrtplib/extratransmitters/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK82FN256VDC15" -DWINC1500 -I../source/FatFs -I../source/jrtplib -I../source/include -I../source/winc1500 -I../source/TinyWebServer -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


