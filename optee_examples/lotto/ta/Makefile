CFG_TEE_TA_LOG_LEVEL ?= 4
CFG_TA_OPTEE_CORE_API_COMPAT_1_1=y

# The UUID for the Trusted Application
BINARY=1ee382f6-9e82-4228-abaf-4ccbd0ebb7a0

SRC_FILES += lotto_ta.c user_ta_header.c syscalls_stubs.c

CFLAGS += -I/include/libsodium/src/libsodium/include
LDADD += /home/harris/Desktop/optee_p/optee_examples/lotto/ta/lib/libsodium.a


-include $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk

ifeq ($(wildcard $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk), )
clean:
	@echo 'Note: $$(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk not found, cannot clean TA'
	@echo 'Note: TA_DEV_KIT_DIR=$(TA_DEV_KIT_DIR)'
endif
