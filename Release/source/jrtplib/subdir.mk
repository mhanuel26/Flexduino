################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/jrtplib/rtcpapppacket.cpp \
../source/jrtplib/rtcpbyepacket.cpp \
../source/jrtplib/rtcpcompoundpacket.cpp \
../source/jrtplib/rtcpcompoundpacketbuilder.cpp \
../source/jrtplib/rtcppacket.cpp \
../source/jrtplib/rtcppacketbuilder.cpp \
../source/jrtplib/rtcprrpacket.cpp \
../source/jrtplib/rtcpscheduler.cpp \
../source/jrtplib/rtcpsdesinfo.cpp \
../source/jrtplib/rtcpsdespacket.cpp \
../source/jrtplib/rtcpsrpacket.cpp \
../source/jrtplib/rtpbyteaddress.cpp \
../source/jrtplib/rtpcollisionlist.cpp \
../source/jrtplib/rtpdebug.cpp \
../source/jrtplib/rtperrors.cpp \
../source/jrtplib/rtpexternaltransmitter.cpp \
../source/jrtplib/rtpinternalsourcedata.cpp \
../source/jrtplib/rtpipv4address.cpp \
../source/jrtplib/rtpipv6address.cpp \
../source/jrtplib/rtplibraryversion.cpp \
../source/jrtplib/rtppacket.cpp \
../source/jrtplib/rtppacketbuilder.cpp \
../source/jrtplib/rtppollthread.cpp \
../source/jrtplib/rtprandom.cpp \
../source/jrtplib/rtprandomrand48.cpp \
../source/jrtplib/rtprandomrands.cpp \
../source/jrtplib/rtprandomurandom.cpp \
../source/jrtplib/rtpsession.cpp \
../source/jrtplib/rtpsessionparams.cpp \
../source/jrtplib/rtpsessionsources.cpp \
../source/jrtplib/rtpsourcedata.cpp \
../source/jrtplib/rtpsources.cpp \
../source/jrtplib/rtptimeutilities.cpp \
../source/jrtplib/rtpudpv4transmitter.cpp \
../source/jrtplib/rtpudpv6transmitter.cpp 

OBJS += \
./source/jrtplib/rtcpapppacket.o \
./source/jrtplib/rtcpbyepacket.o \
./source/jrtplib/rtcpcompoundpacket.o \
./source/jrtplib/rtcpcompoundpacketbuilder.o \
./source/jrtplib/rtcppacket.o \
./source/jrtplib/rtcppacketbuilder.o \
./source/jrtplib/rtcprrpacket.o \
./source/jrtplib/rtcpscheduler.o \
./source/jrtplib/rtcpsdesinfo.o \
./source/jrtplib/rtcpsdespacket.o \
./source/jrtplib/rtcpsrpacket.o \
./source/jrtplib/rtpbyteaddress.o \
./source/jrtplib/rtpcollisionlist.o \
./source/jrtplib/rtpdebug.o \
./source/jrtplib/rtperrors.o \
./source/jrtplib/rtpexternaltransmitter.o \
./source/jrtplib/rtpinternalsourcedata.o \
./source/jrtplib/rtpipv4address.o \
./source/jrtplib/rtpipv6address.o \
./source/jrtplib/rtplibraryversion.o \
./source/jrtplib/rtppacket.o \
./source/jrtplib/rtppacketbuilder.o \
./source/jrtplib/rtppollthread.o \
./source/jrtplib/rtprandom.o \
./source/jrtplib/rtprandomrand48.o \
./source/jrtplib/rtprandomrands.o \
./source/jrtplib/rtprandomurandom.o \
./source/jrtplib/rtpsession.o \
./source/jrtplib/rtpsessionparams.o \
./source/jrtplib/rtpsessionsources.o \
./source/jrtplib/rtpsourcedata.o \
./source/jrtplib/rtpsources.o \
./source/jrtplib/rtptimeutilities.o \
./source/jrtplib/rtpudpv4transmitter.o \
./source/jrtplib/rtpudpv6transmitter.o 

CPP_DEPS += \
./source/jrtplib/rtcpapppacket.d \
./source/jrtplib/rtcpbyepacket.d \
./source/jrtplib/rtcpcompoundpacket.d \
./source/jrtplib/rtcpcompoundpacketbuilder.d \
./source/jrtplib/rtcppacket.d \
./source/jrtplib/rtcppacketbuilder.d \
./source/jrtplib/rtcprrpacket.d \
./source/jrtplib/rtcpscheduler.d \
./source/jrtplib/rtcpsdesinfo.d \
./source/jrtplib/rtcpsdespacket.d \
./source/jrtplib/rtcpsrpacket.d \
./source/jrtplib/rtpbyteaddress.d \
./source/jrtplib/rtpcollisionlist.d \
./source/jrtplib/rtpdebug.d \
./source/jrtplib/rtperrors.d \
./source/jrtplib/rtpexternaltransmitter.d \
./source/jrtplib/rtpinternalsourcedata.d \
./source/jrtplib/rtpipv4address.d \
./source/jrtplib/rtpipv6address.d \
./source/jrtplib/rtplibraryversion.d \
./source/jrtplib/rtppacket.d \
./source/jrtplib/rtppacketbuilder.d \
./source/jrtplib/rtppollthread.d \
./source/jrtplib/rtprandom.d \
./source/jrtplib/rtprandomrand48.d \
./source/jrtplib/rtprandomrands.d \
./source/jrtplib/rtprandomurandom.d \
./source/jrtplib/rtpsession.d \
./source/jrtplib/rtpsessionparams.d \
./source/jrtplib/rtpsessionsources.d \
./source/jrtplib/rtpsourcedata.d \
./source/jrtplib/rtpsources.d \
./source/jrtplib/rtptimeutilities.d \
./source/jrtplib/rtpudpv4transmitter.d \
./source/jrtplib/rtpudpv6transmitter.d 


# Each subdirectory must supply rules for building sources it contributes
source/jrtplib/%.o: ../source/jrtplib/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -D"CPU_MK82FN256VDC15" -D__FPU_PRESENT -DWINC1500 -DARM_MATH_CM4 -I../startup -I../source/Wire -I../source/INA2XX -I../source/cameraLib -I../source/cameraLib/encoder -I../source/WebSockets -I../source/cameraLib/ov7670 -I../source/include -I../source/TinyWebServer -I../board -I../utilities -I../source/winc1500 -I../source/audioLib -I../source/winc1500_firmware_updater -I../CMSIS -I../drivers -I../source/FatFs -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


