################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/subdir.mk
-include startup/subdir.mk
-include source/winc1500_firmware_updater/subdir.mk
-include source/winc1500/spi_flash/source/subdir.mk
-include source/winc1500/socket/source/subdir.mk
-include source/winc1500/driver/source/subdir.mk
-include source/winc1500/common/source/subdir.mk
-include source/winc1500/bus_wrapper/source/subdir.mk
-include source/winc1500/bsp/source/subdir.mk
-include source/winc1500/subdir.mk
-include source/cameraLib/ov7670/subdir.mk
-include source/cameraLib/encoder/subdir.mk
-include source/cameraLib/subdir.mk
-include source/audioLib/subdir.mk
-include source/Wire/utility/subdir.mk
-include source/Wire/subdir.mk
-include source/WebSockets/libsha1/subdir.mk
-include source/WebSockets/libb64/subdir.mk
-include source/WebSockets/subdir.mk
-include source/TinyWebServer/subdir.mk
-include source/INA2XX/subdir.mk
-include source/Firmata/utility/subdir.mk
-include source/Firmata/subdir.mk
-include source/FatFs/option/subdir.mk
-include source/FatFs/subdir.mk
-include source/subdir.mk
-include drivers/subdir.mk
-include dma_manager/subdir.mk
-include board/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
nxp_contest.srec \

SECONDARY_SIZE += \
nxp_contest.siz \


# All Target
all: nxp_contest.elf secondary-outputs

# Tool invocations
nxp_contest.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -T ../MK82FN256xxx15_flash.ld -Xlinker --gc-sections -L/home/chroots/arm-devel/usr/local/lib/lib/ -L/home/mhanuel/Devel/Networking/ArduinoJson/lib -Wl,-Map,"nxp_contest.map" -specs=nosys.specs -specs=nano.specs -Xlinker -z -Xlinker muldefs -Xlinker -defsym=__heap_size__=0x2000 -Xlinker -defsym=__stack_size__=0x2000 -o "nxp_contest.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

nxp_contest.srec: nxp_contest.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O srec "nxp_contest.elf"  "nxp_contest.srec"
	@echo 'Finished building: $@'
	@echo ' '

nxp_contest.siz: nxp_contest.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "nxp_contest.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_SIZE)$(C++_DEPS)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(CC_DEPS)$(SECONDARY_FLASH)$(CPP_DEPS)$(CXX_DEPS)$(C_UPPER_DEPS)$(S_UPPER_DEPS) nxp_contest.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
