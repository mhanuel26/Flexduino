################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/driver/source/m2m_ate_mode.c \
../source/driver/source/m2m_hif.c \
../source/driver/source/m2m_ota.c \
../source/driver/source/m2m_periph.c \
../source/driver/source/m2m_wifi.c \
../source/driver/source/nmasic.c \
../source/driver/source/nmbus.c \
../source/driver/source/nmdrv.c \
../source/driver/source/nmi2c.c \
../source/driver/source/nmspi.c \
../source/driver/source/nmuart.c 

OBJS += \
./source/driver/source/m2m_ate_mode.o \
./source/driver/source/m2m_hif.o \
./source/driver/source/m2m_ota.o \
./source/driver/source/m2m_periph.o \
./source/driver/source/m2m_wifi.o \
./source/driver/source/nmasic.o \
./source/driver/source/nmbus.o \
./source/driver/source/nmdrv.o \
./source/driver/source/nmi2c.o \
./source/driver/source/nmspi.o \
./source/driver/source/nmuart.o 

C_DEPS += \
./source/driver/source/m2m_ate_mode.d \
./source/driver/source/m2m_hif.d \
./source/driver/source/m2m_ota.d \
./source/driver/source/m2m_periph.d \
./source/driver/source/m2m_wifi.d \
./source/driver/source/nmasic.d \
./source/driver/source/nmbus.d \
./source/driver/source/nmdrv.d \
./source/driver/source/nmi2c.d \
./source/driver/source/nmspi.d \
./source/driver/source/nmuart.d 


# Each subdirectory must supply rules for building sources it contributes
source/driver/source/%.o: ../source/driver/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -DDEBUG -DCPU_MK82FN256VDC15 -DFRDM_K82F -DFREEDOM -I../startup -I../board -I../utilities -I../CMSIS -I../drivers -I../source/FatFs -I../source/TinyWebServer -I../source/winc1500 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


