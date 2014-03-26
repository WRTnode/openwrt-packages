# Copyright (C) 2012 DreamBox Team.
# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# lintel<lintel.huang@gmail.com>
#

# If there is a src subdirectory, compile the driver from source (using
# any patches in the patches/ directory).  Otherwise, just use the
# precompiled .ko from files/lib/modules/...
#
# Note that this is for DreamBox use only, since the wifi driver
# source is made available by Ralink under a non-open license, which
# forbids redistribution of the sources.

USE_SRC=$(shell ls ./src/ 2>/dev/null >/dev/null && echo 1)

include $(TOPDIR)/rules.mk

PKG_NAME:=ralink-wifi
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/ralink-wifi
  SECTION:=ralink
  CATEGORY:=Ralink
  TITLE:=Ralink APSoC WiFi SoftAP driver
#  DEPENDS:=@TARGET_ralink
endef

define Package/ralink-wifi/description
	Ralink APSoC WiFi driver for DreamBox
endef

ifneq ($(USE_SRC),)
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(Build/Patch)
	$(if $(QUILT),touch $(PKG_BUILD_DIR)/.quilt_used)
endef

define Build/Compile
	cd $(LINUX_DIR); \
		ARCH=mips CC=$(TARGET_CC) \
		$(MAKE) M=$(PKG_BUILD_DIR)/rt2860v2_ap/ V=1 modules
endef

else

define Build/Prepare
	echo
endef

define Build/Compile
	echo
endef

endif

define Package/ralink-wifi/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION) $(1)/etc/Wireless/RT2860/
	$(CP) -rf ./files/* $(1)/
	$(CP) modules/* $(1)/lib/modules/$(LINUX_VERSION)

	# This overwrites the precompiled version from files/lib/modules

#	$(CP) $(LINUX_DIR)/drivers/net/wireless/rt2860v2_ap/rt2860v2_ap.ko $(1)/lib/modules/$(LINUX_VERSION)

endef

$(eval $(call BuildPackage,ralink-wifi))

