# Copyright (C) 2014 WRTnode machine team.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# noel<noel@wrtnode.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=WRTnode2r-stm32
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/WRTnode2r-stm32
  SECTION:=wrtnode
  CATEGORY:=WRTnode
  TITLE:=WRTnode2r update stm32 
  DEPENDS:=+spi-bridge
endef

define Package/WRTnode2r-stm32/description
	WRTnode2r update stm32
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
     
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/WRTnode2r-stm32/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/WRTnode2r-stm32 $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/flash-stm32 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,WRTnode2r-stm32))

