# Copyright (C) 2014 WRTnode machine team.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# noel<noel@wrtnode.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shine
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/shine
  SECTION:=wrtnode
  CATEGORY:=WRTnode
  SUBMENU :=shine
  TITLE:=Revived shine fixed-point mp3 encoder 
endef

define Package/shine/description
	Revived shine fixed-point mp3 encoder.
	it can compress .wav to .mp3 with a ultra high speed.
	tens of speed faster than realtime encoding.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
     
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS) -fPIC"
endef

define Package/shine/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/shineenc-gen $(1)/usr/bin/
endef

$(eval $(call BuildPackage,shine))

