# Copyright (C) 2014 WRTnode machine team.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# noel<noel@wrtnode.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uixo_manager
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/uixo_manager
  SECTION:=uixo_manager
  CATEGORY:=WRTnode
  TITLE:=UIxO manager
  DEPENDS:=+libubox +libubus +libblobmsg-json
endef

define Package/uixo_manager/description
  UIxO manager
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/uixo_manager/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uixo_manager $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uixo_manager_fork_object $(1)/bin/
endef

$(eval $(call BuildPackage,uixo_manager))

