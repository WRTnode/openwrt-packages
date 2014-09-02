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
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
     
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/FlashMAC/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/www/
	$(CP) ./files/www/* $(1)/www/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libubox_s.so.7.0 $(1)/lib
endef

$(eval $(call BuildPackage,FlashMAC))

