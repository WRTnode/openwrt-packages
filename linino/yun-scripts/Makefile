#
# Copyright (c) 2013 Arduino LLC. All right reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=yun-scripts
PKG_VERSION:=1.0.0
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/yun-scripts
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Custom Yún scripts
endef

define Build/Compile
	# NOOP
endef

define Package/yun-scripts/install
	$(INSTALL_DIR) $(1)/usr/bin/

	$(INSTALL_BIN) ./files/usr/bin/run-bridge $(1)/usr/bin/
endef

$(eval $(call BuildPackage,yun-scripts))

