##############################################
# wrtbox is a Swiss Army knife for WRTnode  
# WRTnode's busybox
# This file is part of wrtbox.
# Author: 39514004@qq.com (huamanlou,alais name intel inside)
#
# This library is free software; under the terms of the GPL
#
##############################################
include $(TOPDIR)/rules.mk
# Nameand release number of this package

PKG_NAME:=wrtbox
PKG_VERSION:=0.1
PKG_RELEASE:=1
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
define Package/wrtbox
    CATEGORY:=WRTnode
    TITLE:=wrtbox-- The Swiss Army Knife of WRTnode
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/wrtbox/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wrtbox $(1)/bin/
	ln -fs wrtbox $(1)/bin/dnsclient
	ln -fs wrtbox $(1)/bin/get_ip
	ln -fs wrtbox $(1)/bin/is_intf_up
	ln -fs wrtbox $(1)/bin/wait_intf_up
	ln -fs wrtbox $(1)/bin/hellowrt
endef

define Package/$(PKG_NAME)/postinst
	#!/bin/sh
	# check if we are on real system
	if [ -z "$${IPKG_INSTROOT}" ]; then
		echo "ln -fs command!"
		ln -fs /bin/wrtbox /bin/dnsclient
		ln -fs /bin/wrtbox /bin/get_ip
		ln -fs /bin/wrtbox /bin/is_intf_up
		ln -fs /bin/wrtbox /bin/wait_intf_up
		ln -fs /bin/wrtbox /bin/hellowrt
    fi
	exit 0
endef

$(eval $(call BuildPackage,wrtbox))
