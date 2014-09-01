#
# Copyright (C) 2013-2014 WRTnode machine team 
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=FlashMAC
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/FlashMAC
	SECTION:=FlashMAC
	CATEGORY:=WRTnode
	TITLE:=WRTnode flash MAC by web
endef

define Package/FlashMAC/description
	This package provides a way that wrtnode flash MAC through web.
endef

define Build/Prepare
        echo
endef

define Build/Compile
        echo
endef

define Package/FlashMAC/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/www/
	$(CP) ./files/www/* $(1)/www/
	$(INSTALL_BIN) ./files/lib/libubox_s.so.7.0 $(1)/lib
endef

$(eval $(call BuildPackage,FlashMAC))

