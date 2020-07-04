.DEFAULT_GOAL := build
FIRMWARE = firmware.bin
# https://docs.particle.io/reference/discontinued/mesh/ 
TARGET = 1.5.2
PROJECT ?= litterbox-ventilation
DEVICE ?= xenon

deps:
	@echo Installing deps
	yarn global add particle-cli

.PHONY: $(FIRMWARE)
$(FIRMWARE):
	particle cloud compile $(DEVICE) project.properties src/$(PROJECT) --target $(TARGET) --saveTo $(FIRMWARE)

#.PHONY: $(FIRMWARE)
#$(FIRMWARE):
#	particle compile $(DEVICE) src/particle/$(PROJECT) lib/FastLED-$(FASTLED_VERSION) --target $(TARGET) --saveTo $(FIRMWARE)
##	particle compile $(DEVICE) src/particle/$(PROJECT) lib/FastLED-$(FASTLED_VERSION) --target $(TARGET) --saveTo $(FIRMWARE)

.PHONY: build
build: $(FIRMWARE)

.PHONY: flash
flash: build $(FIRMWARE)
	@echo Flashing firmware
	particle flash --usb $(FIRMWARE)
