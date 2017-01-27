PARTICLE = particle
SRC_DIR_PREFIX = ADPL_
TARGET_VERSION = 0.5.3
DEVICE_ID = ADPLDuke0785

compile-electron:
	$(PARTICLE) compile electron $(SRC_DIR_PREFIX)electron

flash-electron:
	$(PARTICLE) flash --target $(TARGET_VERSION) $(DEVICE_ID) $(SRC_DIR_PREFIX)electron

compile-photon:
	$(PARTICLE) compile photon $(SRC_DIR_PREFIX)photon

clean:
	rm electron_firmware*.bin
