#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=avrdude
PKG_VER:=5.11.1
PKG_REV:=1138
PKG_VERSION:=$(PKG_VER)-r$(PKG_REV)
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://svn.savannah.nongnu.org/svn/avrdude/trunk/avrdude
PKG_SOURCE_SUBDIR:=avrdude-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_PROTO:=svn

include $(INCLUDE_DIR)/package.mk

define Package/avrdude
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=AVR Downloader/UploaDEr
  URL:=http://www.bsdhome.com/avrdude/
  DEPENDS:=+libncurses +libreadline +libusb-compat +libc +libelf1 +lm $(CXX_DEPENDS)
endef

define Package/avrdude/description
 AVRDUDE is a full featured program for programming Atmel's AVR CPU's.
endef

define Build/Configure
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	chmod -R u+w $(PKG_BUILD_DIR)
	(cd $(PKG_BUILD_DIR);./bootstrap)
	$(call Build/Configure/Default)
endef

CONFIGURE_ARGS+= \
	--enable-shared \
	--disable-static \
	--disable-parport \
	--enable-linuxgpio \

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all 
endef

define Package/avrdude/conffiles
/etc/avrdude.conf
endef

define Package/avrdude/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/avrdude.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/avrdude $(1)/usr/bin/
endef

$(eval $(call BuildPackage,avrdude))
