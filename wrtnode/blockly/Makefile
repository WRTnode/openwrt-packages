#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=blockly
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/blockly
	SECTION:=blockly
	CATEGORY:=WRTnode
	TITLE:=WRTnode controls the self-balanced car
endef

define Package/blockly/description
	This package is a bridge connected to the communication blockly and WRTnode 
endef

define Build/Prepare
        echo
endef

define Build/Compile
        echo
endef

define Package/blockly/install
	$(INSTALL_DIR) $(1)/www/
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(CP) ./files/www/blockly $(1)/www/ -d
	$(CP) ./files/www/cgi-bin/* $(1)/www/cgi-bin
endef

$(eval $(call BuildPackage,blockly))

