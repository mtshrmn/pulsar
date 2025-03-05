MCU = atmega32u4
F_CPU = 16000000UL
F_USB = $(F_CPU)
ARCH = AVR8

TARGET = Mixer

CC = avr-gcc
OBJCOPY = avr-objcopy
LUFA_PATH_PARENT = ./lufa
LUFA_PATH = $(LUFA_PATH_PARENT)/LUFA
LUFA_SRC_USB_COMMON = $(LUFA_PATH)/Drivers/USB/Core/$(ARCH)/USBController_$(ARCH).c
LUFA_SRC_USB_DEVICE = $(LUFA_PATH)/Drivers/USB/Core/$(ARCH)/Device_$(ARCH).c
LUFA_SRC_USB_CLASS = $(LUFA_PATH)/Drivers/USB/Class/Device/HIDClassDevice.c
LUFA_SRC_USB = $(LUFA_SRC_USB_COMMON) $(LUFA_SRC_USB_DEVICE)
# SRC = $(TARGET).c $(LUFA_SRC_USB) Descriptors.c
SRC = $(LUFA_SRC_USB_CLASS) $(LUFA_SRC_USB) 
SRC += $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

CFLAGS = -std=c99 -Wall -Os -flto -ffunction-sections -fdata-sections
CFLAGS += -I. -I$(LUFA_PATH_PARENT)
CFLAGS += -D F_CPU=$(F_CPU) -mmcu=$(MCU) -D F_USB=$(F_USB) -D ARCH=ARCH_$(ARCH)

CFLAGS += -D USB_DEVICE_ONLY
CFLAGS += -D USE_FLASH_DESCRIPTORS
CFLAGS += -D USE_STATIC_OPTIONS="(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)"
CFLAGS += -D FIXED_CONTROL_ENDPOINT_SIZE=8
CFLAGS += -D FIXED_NUM_CONFIGURATIONS=1

all: $(TARGET).hex

DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

flash: $(TARGET).hex
	avrdude -c avr109 -p $(MCU) -P /dev/ttyACM0 -U flash:w:$(TARGET).hex -v -v

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(OBJ)
