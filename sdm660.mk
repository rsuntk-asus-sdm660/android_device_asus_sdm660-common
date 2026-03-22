#
# Copyright (C) 2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit the proprietary files
$(call inherit-product, vendor/asus/sdm660-common/sdm660-common-vendor.mk)

$(call inherit-product, $(SRC_TARGET_DIR)/product/non_ab_device.mk)

TARGET_EXCLUDES_AUDIOFX := true

# Default is nosdcard, S/W button enabled in resource
PRODUCT_CHARACTERISTICS := nosdcard

# Common Tree Path
COMMON_PATH := device/asus/sdm660-common

# AID/fs configs
PRODUCT_PACKAGES += \
    fs_config_files

# Audio
PRODUCT_PACKAGES += \
    android.hardware.audio@7.1-impl:32 \
    android.hardware.audio.effect@7.0-impl:32 \
    android.hardware.audio.service \
    android.hardware.soundtrigger@2.3-impl:32

PRODUCT_PACKAGES += \
    audio.bluetooth.default \
    audio.primary.sdm660:32 \
    sound_trigger.primary.sdm660:32 \
    audio.r_submix.default \
    audio.usb.default \
    libaudio-resampler \
    libqcompostprocbundle \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libvolumelistener \
    tinymix

PRODUCT_PACKAGES += \
    libhdmiedid \
    libhfp \
    libsndmonitor \
    libspkrprot \
    libssrec

# Audio Configs
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/audio/audio_configs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_configs.xml \
    $(COMMON_PATH)/configs/audio/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml \
    $(COMMON_PATH)/configs/audio/audio_output_policy.conf:$(TARGET_COPY_OUT_VENDOR)/etc/audio_output_policy.conf \
    $(COMMON_PATH)/configs/audio/audio_platform_info_intcodec.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_intcodec.xml \
    $(COMMON_PATH)/configs/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(COMMON_PATH)/configs/audio/audio_tuning_mixer.txt:$(TARGET_COPY_OUT_VENDOR)/etc/audio_tuning_mixer.txt \
    $(COMMON_PATH)/configs/audio/graphite_ipc_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/graphite_ipc_platform_info.xml \
    $(COMMON_PATH)/configs/audio/listen_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/listen_platform_info.xml \
    $(COMMON_PATH)/configs/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths.xml \
    $(COMMON_PATH)/configs/audio/sound_trigger_mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths.xml \
    $(COMMON_PATH)/configs/audio/sound_trigger_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_platform_info.xml

# Audio Policy
PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_in_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_in_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml

# ANT+
PRODUCT_PACKAGES += \
    AntHalService-Soong \
    com.dsi.ant@1.0.vendor

# Boot animation
TARGET_BOOTANIMATION_HALF_RES := true

# Bluetooth
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.1 \
    android.hardware.bluetooth.audio-impl \
    android.hardware.bluetooth@1.1.vendor \
    vendor.qti.hardware.btconfigstore@1.0.vendor \
    vendor.qti.hardware.bluetooth_audio@2.1.vendor \
    vendor.qti.hardware.btconfigstore@1.0.vendor \
    vendor.qti.hardware.btconfigstore@2.0.vendor

# Camera
PRODUCT_PACKAGES += \
    android.hardware.camera.provider-service_32.lineage \
    vendor.qti.hardware.camera.device@1.0.vendor \
    camera.sdm660:32 \
    libgui_vendor \
    liblz4.vendor \
    libutilscallstack.vendor \
    libpng.vendor:32 \
    libxml2 \
    Aperture

# Configstore
PRODUCT_PACKAGES += \
    disable_configstore

# Cgroup and task_profiles
PRODUCT_COPY_FILES += \
    system/core/libprocessgroup/profiles/cgroups.json:$(TARGET_COPY_OUT_VENDOR)/etc/cgroups.json \
    system/core/libprocessgroup/profiles/task_profiles.json:$(TARGET_COPY_OUT_VENDOR)/etc/task_profiles.json

# Dex/ART optimization
PRODUCT_ART_TARGET_INCLUDE_DEBUG_BUILD := false
PRODUCT_DEX_PREOPT_DEFAULT_COMPILER_FILTER := everything
USE_DEX2OAT_DEBUG := false 

# Dexpreopt
PRODUCT_DEXPREOPT_SPEED_APPS += \
    SystemUI

# Core Display Libraries
PRODUCT_PACKAGES += \
    gralloc.sdm660 \
    hwcomposer.sdm660 \
    memtrack.sdm660 \
    libdisplayconfig \
    libgralloc.qti \
    libtinyxml \
    libqdMetaData.system \
    libqdMetaData

PRODUCT_PACKAGES += \
    android.frameworks.displayservice@1.0_32 \
    android.frameworks.displayservice@1.0.vendor \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.graphics.mapper@3.0-impl-qti-display \
    android.hardware.graphics.mapper@4.0-impl-qti-display \
    android.frameworks.displayservice@1.0 \
    vendor.qti.hardware.display.allocator-service \
    vendor.qti.hardware.display.mapper@2.0.vendor \
    vendor.qti.hardware.display.mapper@3.0.vendor \
    vendor.qti.hardware.display.mapper@4.0.vendor \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service \
    vendor.display.config@1.0.vendor \
    vendor.display.config@2.0

###########################################
# Display Properties
###########################################

PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.hw=1 \
    debug.egl.hw=1 \
    debug.sf.latch_unsignaled=1 \
    debug.sf.disable_backpressure=1 \
    debug.renderengine.backend=skiaglthreaded \
    debug.hwui.renderer=skiagl \
    ro.hardware.egl=adreno \
    ro.hardware.vulkan=adreno \
    ro.opengles.version=196610 \
    vendor.gralloc.enable_fb_ubwc=1 \
    vendor.display.enable_default_color_mode=1 \
    vendor.display.disable_skip_validate=1 \
    vendor.video.disable.ubwc=1

# SurfaceFlinger properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.force_hwc_copy_for_virtual_displays=true \
    ro.surface_flinger.max_frame_buffer_acquired_buffers=3 \
    ro.surface_flinger.max_virtual_display_dimension=4096 \
    ro.surface_flinger.has_wide_color_display=false \
    ro.surface_flinger.has_HDR_display=false \
    ro.surface_flinger.use_color_management=false \
    ro.surface_flinger.protected_contents=true

# Display Device Config
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/displayconfig/display_id_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/displayconfig/display_id_0.xml

# Doze mode
PRODUCT_PACKAGES += \
    DeviceDoze

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm@1.4.vendor \
    android.hardware.drm-service.clearkey \
    libunwindstack.vendor \
    libhidlmemory.vendor:64 \
    libcrypto

# FM
PRODUCT_PACKAGES += \
    android.hardware.broadcastradio@1.0-impl

PRODUCT_PACKAGES += \
    FM2 \
    libqcomfm_jni \
    qcom.fmradio \
    qcom.fmradio.xml

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.1-service-asus

# Fwk detect
PRODUCT_PACKAGES += \
    libqti_vndfwk_detect \
    libqti_vndfwk_detect.vendor \
    libvndfwk_detect_jni.qti \
    libvndfwk_detect_jni.qti.vendor

# Gatekeeper
PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0.vendor \
    libion.vendor

# Health
PRODUCT_PACKAGES += \
    android.hardware.health-service.qti \
    android.hardware.health-service.qti_recovery
    
PRODUCT_PACKAGES += \
    android.hardware.health@2.1.vendor    

# GMS Permissions
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/privapp-permission/privapp-permissions-gms.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/privapp.permissions-gms.xml

# GPS
$(call inherit-product, $(COMMON_PATH)/gps/gps_vendor_product.mk)

PRODUCT_PACKAGES += \
    libsensorndkbridge \
    libcurl.vendor \
    libwifi-hal-ctrl

PRODUCT_PACKAGES += \
    gps.conf \
    flp.conf

PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/gps/apdr.conf:$(TARGET_COPY_OUT_VENDOR)/etc/apdr.conf \
    $(COMMON_PATH)/configs/gps/izat.conf:$(TARGET_COPY_OUT_VENDOR)/etc/izat.conf \
    $(COMMON_PATH)/configs/gps/lowi.conf:$(TARGET_COPY_OUT_VENDOR)/etc/lowi.conf \
    $(COMMON_PATH)/configs/gps/sap.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sap.conf \
    $(COMMON_PATH)/configs/gps/xtwifi.conf:$(TARGET_COPY_OUT_VENDOR)/etc/xtwifi.conf

# HIDL
PRODUCT_PACKAGES += \
    android.hidl.base@1.0 \
    libhidltransport \
    libhidltransport.vendor \
    libhwbinder \
    libhwbinder.vendor

# Input
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/keylayout/gpio-keys.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/gpio-keys.kl

# IPACM
PRODUCT_PACKAGES += \
    ipacm \
    IPACM_cfg.xml

# IPC router config
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/sec_config:$(TARGET_COPY_OUT_VENDOR)/etc/sec_config

# Keymaster
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0.vendor

# Lights
PRODUCT_PACKAGES += \
    android.hardware.light-service.asus_sdm660

# Lineage Health
PRODUCT_PACKAGES += \
    vendor.lineage.health-service.default

# Media
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/media/media_profiles.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/media_profiles.xml \
    $(COMMON_PATH)/configs/media/media_profiles.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_vendor.xml \
    $(COMMON_PATH)/configs/media/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(COMMON_PATH)/configs/media/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    $(COMMON_PATH)/configs/media/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_ODM)/etc/media_profiles_V1_0.xml # This is media/media_profiles.xml from qcom/common

# Network
PRODUCT_PACKAGES += \
    android.system.net.netd@1.1.vendor \
    libnetutils.vendor

# NFC
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/android.hardware.nfc.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/com.android.nfc_extras.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/com.nxp.mifare.xml

PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nci.conf \
    $(COMMON_PATH)/configs/nfc/libnfc-nxp.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nxp.conf

PRODUCT_PACKAGES += \
    android.hardware.nfc@1.2-service \
    com.android.nfc_extras \
    Tag

# OEM Unlock reporting
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1

# Neuralnetworks
PRODUCT_PACKAGES += \
    android.hardware.neuralnetworks@1.3.vendor

# OMX
PRODUCT_PACKAGES += \
    libc2dcolorconvert \
    libmm-omxcore \
    libOmxAacEnc \
    libOmxAmrEnc \
    libOmxCore \
    libOmxEvrcEnc \
    libOmxG711Enc \
    libOmxQcelp13Enc \
    libOmxVdec \
    libOmxVenc \
    libstagefrighthw \
    libstagefright_softomx_plugin.vendor

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(COMMON_PATH)/overlay \
    $(COMMON_PATH)/overlay-translates

PRODUCT_ENFORCE_RRO_TARGETS := *

# Permissions
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/privapp-permission/privapp-permissions-qti.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions-qti.xml \
    $(COMMON_PATH)/configs/privapp-permission/privapp-permissions-qti-system-ext.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/privapp-permissions-qti-system-ext.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.audio.pro.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.pro.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.fingerprint.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.ims.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.ims.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version.xml \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.software.freeform_window_management.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.freeform_window_management.xml \
    frameworks/native/data/etc/android.software.picture_in_picture.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.picture_in_picture.xml \
    frameworks/native/data/etc/android.software.app_widgets.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.app_widgets.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.software.opengles.deqp.level-2021-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.opengles.deqp.level.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.software.vulkan.deqp.level-2021-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.vulkan.deqp.level.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/handheld_core_hardware.xml

# Power
PRODUCT_PACKAGES += \
    android.hardware.power@1.0 \
    android.hardware.power@1.0.vendor \
    android.hardware.power@1.1 \
    android.hardware.power@1.1.vendor \
    android.hardware.power@1.2 \
    android.hardware.power@1.2.vendor \
    android.hardware.power@1.3 \
    android.hardware.power@1.3.vendor \
    android.hardware.power-service-qti \
    android.hardware.power.stats@1.0-service.mock

# Perf
PRODUCT_PACKAGES += \
   libtflite \
   libtextclassifier_hash
    
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/perf/perf-profile0.conf:$(TARGET_COPY_OUT_VENDOR)/etc/perf/perf-profile0.conf \
    $(COMMON_PATH)/configs/powerhint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/powerhint.xml    

# Public Libraries
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/public.libraries.txt:$(TARGET_COPY_OUT_VENDOR)/etc/public.libraries.txt

# Remove unwanted packages
PRODUCT_PACKAGES += \
    RemovePackages

# QCOM
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/privapp-permissions-qti.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions-qti.xml \
    $(COMMON_PATH)/configs/qti_whitelist.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/sysconfig/qti_whitelist.xml

# QMI
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/qmi/qmi_fw.conf:$(TARGET_COPY_OUT_VENDOR)/etc/qmi_fw.conf

PRODUCT_PACKAGES += \
    libcrypto_utils.vendor \
    libjsoncpp.vendor \
    libjson \
    libqti_vndfwk_detect.vendor \
    libvndfwk_detect_jni.qti \
    libvndfwk_detect_jni.qti.vendor

# QNS
PRODUCT_PACKAGES += \
    libstdc++.vendor

# RIL
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full \
    android.hardware.radio@1.5 \
    android.hardware.radio@1.5.vendor \
    android.hardware.radio.config@1.2 \
    android.hardware.radio.config@1.2.vendor \
    android.hardware.radio.deprecated@1.0 \
    android.hardware.radio.deprecated@1.0.vendor \
    android.hardware.secure_element@1.1 \
    android.hardware.secure_element@1.2 \
    android.hardware.secure_element@1.1.vendor \
    android.hardware.secure_element@1.2.vendor \
    libavservices_minijail.vendor \
    librmnetctl \
    libsqlite.vendor:64 \
    libsysutils.vendor

# Recovery
PRODUCT_PACKAGES += \
    librecovery_updater_asus

# Ramdisk
PRODUCT_PACKAGES += \
    init.class_main.sh \
    init.qcom.post_boot.sh \
    init.qti.dcvs.sh \
    init.qcom.sensors.sh \
    init.qcom.sh \
    init.zram.sh \
    init.zram_vm.sh \
    init.qcom.usb.sh

PRODUCT_PACKAGES += \
    fstab.qcom \
    init.zram.rc \
    init.qcom.asus.rc \
    init.qcom.rc \
    init.qcom.usb.rc \
    init.sysfs_permissions.rc \
    init.recovery.qcom.rc \
    init.target.rc \
    ueventd.qcom.rc

# Screen density
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG ?= xhdpi

# Seccomp
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/seccomp/mediacodec-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    $(COMMON_PATH)/configs/seccomp/mediaextractor-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy

# Sensors
PRODUCT_PACKAGES += \
    android.frameworks.sensorservice@1.0.vendor \
    android.hardware.sensors@1.0-impl \
    android.hardware.sensors@1.0-service \
    libpower.vendor

PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

# Soong namespaces
QCOM_SOONG_NAMESPACE := \
    $(COMMON_PATH)/qcom-caf

PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/opensource/display

# Telephony
PRODUCT_PACKAGES += \
    extphonelib \
    extphonelib-product \
    extphonelib.xml \
    extphonelib_product.xml \
    ims-ext-common \
    ims_ext_common.xml \
    qti-telephony-hidl-wrapper \
    qti_telephony_hidl_wrapper.xml \
    qti-telephony-hidl-wrapper-prd \
    qti_telephony_hidl_wrapper_prd.xml \
    qti-telephony-utils \
    qti_telephony_utils.xml \
    telephony-ext

PRODUCT_BOOT_JARS += \
    telephony-ext

# Thermal
PRODUCT_PACKAGES += \
    android.hardware.thermal@2.0-service.asus_sdm660.qti

# Trust HAL
PRODUCT_PACKAGES += \
    vendor.lineage.trust@1.0-service

# Touch
PRODUCT_PACKAGES += \
    vendor.lineage.touch@1.0-service.asus_sdm660

# USB
PRODUCT_PACKAGES += \
    usb_compositions.conf \
    android.hardware.usb@1.0-service.basic \
    android.hardware.usb.gadget@1.2-service-qti

PRODUCT_VENDOR_PROPERTIES += \
    vendor.usb.controller=a800000.dwc3

PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/opensource/usb/etc

# Vibrator
$(call inherit-product, vendor/qcom/opensource/vibrator/vibrator-vendor-product.mk)

# FIXME: master: compat for libprotobuf
# See https://android-review.googlesource.com/c/platform/prebuilts/vndk/v28/+/1109518
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full-vendorcompat \
    libprotobuf-cpp-lite-vendorcompat

# Wifi
PRODUCT_PACKAGES += \
    android.hardware.wifi.hostapd@1.3.vendor \
    android.hardware.wifi@1.0-service \
    android.hardware.wifi@1.6.vendor \
    hostapd \
    hostapd_cli \
    libwifi-hal-qcom \
    wificond \
    libwpa_client \
    WifiOverlay \
    wpa_supplicant \
    wpa_supplicant.conf

PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/wifi/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf \
    $(COMMON_PATH)/configs/wifi/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(COMMON_PATH)/configs/wifi/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/WCNSS_qcom_cfg.ini

# QTI Perf - build from source
TARGET_PERF_DIR := sdm660

PRODUCT_PACKAGES += \
    vendor.qti.hardware.perf@2.2-service \
    libqti-perfd \
    libqti-perfd-client \
    libqti-util \
    libperfconfig \
    libperfgluelayer \
    libperfioctl

# Include perf configs
-include vendor/qcom/proprietary/android-perf/profiles.mk

# QTI PerfService (Binder service for Framework)
PRODUCT_PACKAGES += \
    perfservice

# QTI PerfService (Binder service for Framework)
PRODUCT_PACKAGES += \
    perfservice \
    libqti-perfd-client_system

# QPerformance Java library
PRODUCT_PACKAGES += \
    QPerformance \
    com.qualcomm.qti.Performance.xml

# Boot JAR
PRODUCT_BOOT_JARS += \
    QPerformance

# UxPerformance Java library
PRODUCT_PACKAGES += \
    UxPerformance \
    com.qualcomm.qti.UxPerformance.xml

# Activity Trigger
PRODUCT_PACKAGES += \
    libqti-at

# QTI Performance
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=libqti-perfd-client.so \
    ro.vendor.perf-hal.ver=2.2 \
    ro.vendor.perf.scroll_opt=true \
    vendor.perf.gestureflingboost.enable=true
