#
# Copyright (C) 2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

ifneq ($(filter X00TD X01BD,$(TARGET_DEVICE)),)
  
  # Находим все Android.mk в подпапках
  subdir_makefiles=$(call first-makefiles-under,$(LOCAL_PATH))
  
  # Подключаем их
  $(foreach mk,$(subdir_makefiles),$(info including $(mk) ...)$(eval include $(mk)))

# --- 1. Исправление для msadp ---
# Определяем переменную для симлинка
MSADP_SYMLINK := $(TARGET_OUT_VENDOR)/firmware/msadp

# Создаем правило для создания симлинка
$(MSADP_SYMLINK):
	@echo "Creating msadp symlink: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /dev/block/bootdevice/by-name/msadp $@

# Добавляем его в список устанавливаемых модулей
ALL_DEFAULT_INSTALLED_MODULES += $(MSADP_SYMLINK)


# --- 2. Исправление для Mount Points ---

FIRMWARE_MOUNT_POINT := $(TARGET_OUT_VENDOR)/firmware_mnt
BT_FIRMWARE_MOUNT_POINT := $(TARGET_OUT_VENDOR)/bt_firmware
DSP_MOUNT_POINT := $(TARGET_OUT_VENDOR)/dsp

$(FIRMWARE_MOUNT_POINT):
	@echo "Creating directory: $@"
	@mkdir -p $@

$(BT_FIRMWARE_MOUNT_POINT):
	@echo "Creating directory: $@"
	@mkdir -p $@

$(DSP_MOUNT_POINT):
	@echo "Creating directory: $@"
	@mkdir -p $@

# ВАЖНО: Добавляем эти папки в список модулей, чтобы make их создал!
ALL_DEFAULT_INSTALLED_MODULES += $(FIRMWARE_MOUNT_POINT) $(BT_FIRMWARE_MOUNT_POINT) $(DSP_MOUNT_POINT)

IMS_LIBS := libimscamera_jni.so libimsmedia_jni.so
IMS_SYMLINKS := $(addprefix $(TARGET_OUT_SYSTEM_EXT_APPS_PRIVILEGED)/ims/lib/arm64/,$(notdir $(IMS_LIBS)))
$(IMS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "IMS lib link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /system_ext/lib64/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(IMS_SYMLINKS)

# --- RFS Section (Optimized) ---

# Определяем макрос для создания структуры
# $(1): Имя подсистемы (adsp, cdsp, mpss, slpi)
# $(2): Имя папки в tombstones (обычно совпадает, но для mpss это modem)
define create-rfs-rule
RFS_$(1)_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/msm/$(1)
$$(RFS_$(1)_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MSM $(1) folder structure: $$@"
	@rm -rf $$@ && mkdir -p $$@/readonly/vendor
	$(hide) ln -sf /data/vendor/tombstones/rfs/$(2) $$@/ramdumps
	$(hide) ln -sf /mnt/vendor/persist/rfs/msm/$(1) $$@/readwrite
	$(hide) ln -sf /mnt/vendor/persist/rfs/shared $$@/shared
	$(hide) ln -sf /mnt/vendor/persist/hlos_rfs/shared $$@/hlos
	$(hide) ln -sf /vendor/firmware_mnt $$@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $$@/readonly/vendor/firmware

ALL_DEFAULT_INSTALLED_MODULES += $$(RFS_$(1)_SYMLINKS)
endef

# Вызываем макрос для каждой подсистемы
$(eval $(call create-rfs-rule,adsp,lpass))
$(eval $(call create-rfs-rule,cdsp,cdsp))
$(eval $(call create-rfs-rule,mpss,modem))
$(eval $(call create-rfs-rule,slpi,slpi))

WCNSS_INI_SYMLINK := $(TARGET_OUT_VENDOR)/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini
$(WCNSS_INI_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@echo "WCNSS config ini link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /vendor/etc/wifi/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(WCNSS_INI_SYMLINK)

EGL_SYMLINK := $(TARGET_OUT_VENDOR)/lib/libGLESv2_adreno.so
$(EGL_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@mkdir -p $(dir $@)
	@rm -f $@
	$(hide) ln -sf egl/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(EGL_SYMLINK)

endif
