# Copyright (C) 2014 WRTnode machine team.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# noel<noel@wrtnode.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=coin_location
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/coin_location
  SECTION:=wrtnode
  CATEGORY:=WRTnode
  SUBMENU :=demos
  TITLE:=coin_location demo app 
#  DEPENDS:=+uARM
endef

define Package/coin_location/description
	WRTnode coin_location demo application 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
     
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/coin_location/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/coin_location $(1)/usr/bin/
endef

$(eval $(call BuildPackage,coin_location))

