# Enable build support for NFC open source vendor modules
ifeq ($(strip $(TARGET_USES_ST_AIDL_NFC)),true)
ST_VENDOR_NFC += android.hardware.nfc-service.st
else
ST_VENDOR_NFC += android.hardware.nfc@1.2-service.st
endif
ST_VENDOR_NFC += nfc_nci.st21nfc.default

ifeq ($(strip $(TARGET_USES_ST_NFC) $(TARGET_USES_ST_AIDL_NFC)),true)
ifneq ($(TARGET_NFC_SKU),)
NFC_PERMISSIONS_DIR := $(TARGET_COPY_OUT_ODM)/etc/permissions/sku_$(TARGET_NFC_SKU)
else
NFC_PERMISSIONS_DIR := $(TARGET_COPY_OUT_VENDOR)/etc/permissions
endif
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.ese.xml:$(NFC_PERMISSIONS_DIR)/android.hardware.nfc.ese.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(NFC_PERMISSIONS_DIR)/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(NFC_PERMISSIONS_DIR)/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/android.hardware.nfc.uicc.xml:$(NFC_PERMISSIONS_DIR)/android.hardware.nfc.uicc.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(NFC_PERMISSIONS_DIR)/android.hardware.nfc.xml \
    frameworks/native/data/etc/android.hardware.se.omapi.ese.xml:$(NFC_PERMISSIONS_DIR)/android.hardware.se.omapi.ese.xml \
    frameworks/native/data/etc/android.hardware.se.omapi.uicc.xml:$(NFC_PERMISSIONS_DIR)/android.hardware.se.omapi.uicc.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(NFC_PERMISSIONS_DIR)/com.android.nfc_extras.xml

ifeq ($(strip $(TARGET_USES_ST_AIDL_NFC)),true)
ifneq ($(TARGET_NFC_SKU),)
else
DEVICE_MANIFEST_FILE += hardware/st/nfc/aidl/nfc-service-default.xml
endif
endif

PRODUCT_PACKAGES += $(ST_VENDOR_NFC)
endif