#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_MAINTAINER:=Wen Peng Xiao <xiaowp@gmail.com>

PKG_NAME:=mpc
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

PKG_SOURCE_URL:=ftp://ftp.gnu.org/gnu/mpc/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmpc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=MPC library
  DEPENDS:=+libgmp +libmpfr
endef

define Package/libmpc/description
  The Mpc is a C library for the arithmetic of complex numbers with arbitrarily high precision and correct rounding of the result.
endef

CONFIGURE_ARGS = \
		--host=$(REAL_GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--enable-shared

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/mpc* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpc.{a,so*} $(1)/usr/lib/
endef

define Package/libmpc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpc.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmpc))
