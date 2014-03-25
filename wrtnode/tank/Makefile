# Copyright (C) 2014 WRTnode machine team.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# noel<noel@wrtnode.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tank
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/tank
  SECTION:=wrtnode
  CATEGORY:=WRTnode
  SUBMENU :=demos
  TITLE:=Tank SPI controller 
endef

define Package/tank/description
	WRTnode tank demo application throgh SPI interface 
endef

define Build/Prepare
	echo
endef

define Build/Compile
	echo
endef

define Package/tank/install
	#$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	$(CP) -rf ./files/* $(1)
endef

$(eval $(call BuildPackage,tank))

