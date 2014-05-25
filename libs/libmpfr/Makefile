#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_MAINTAINER:=Wen Peng Xiao <xiaowp@gmail.com>

PKG_NAME:=mpfr
PKG_VERSION:=3.1.2
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://www.mpfr.org/mpfr-3.1.2/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=

#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmpfr
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=MPFR library
  DEPENDS:=+libgmp
endef

define Package/libmpfr/description
  The MPFR library is a C library for multiple-precision floating-point computations with correct rounding. 
endef

CONFIGURE_ARGS = \
		--host=$(REAL_GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--enable-shared

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/mpfr* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpfr.{a,so*} $(1)/usr/lib/
endef

define Package/libmpfr/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpfr.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmpfr))
