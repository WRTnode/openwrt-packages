#
# Copyright (C) 2014-2015 wrtnode.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spi-bridge
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/spi-bridge
  SECTION:=spi-bridge
  CATEGORY:=WRTnode
  TITLE:=WRTnode2r bridge between 7688 and stm32.
endef

define Package/spi-bridge/description
  This package is a bridge connected between 7688 and stm32.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/spi-bridge/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spi-bridge $(1)/usr/bin/
endef

$(eval $(call BuildPackage,spi-bridge))
