# Copyright (C) 2014 WRTnode machine team.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# noel<noel@wrtnode.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uixo_console
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/uixo_console
  SECTION:=uixo_console
  CATEGORY:=WRTnode
  TITLE:=UIxO console 
  DEPENDS =+libpthread 
endef
define Package/uixo_console/description
  UIxO console
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
     
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS)
endef

define Package/uixo_console/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uixo_console $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uixo_console_do.sh $(1)/bin/
endef

$(eval $(call BuildPackage,uixo_console))

