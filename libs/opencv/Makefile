# 
# Copyright (C) 2013-2014 wrtnode.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=opencv
PKG_VERSION:=2.4.8
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:=http://downloads.sourceforge.net/project/opencvlibrary/opencv-unix/2.4.8/
#PKG_MD5SUM:=3d8e01537e747d7997993c70f2e108db

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

#define Package/polarssl/Default
 # SUBMENU:=SSL
  #TITLE:=Embedded SSL
  #URL:=http://polarssl.org/
#endef

define Package/polarssl/Default/description
The aim of the PolarSSL project is to provide a quality, open-source
cryptographic library written in C and targeted at embedded systems.
endef

define Package/opencv
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpthread +librt +libstdcpp +zlib
  TITLE:= opencv-2.4.8
endef

#define Package/libpolarssl/description
#$(call Package/polarssl/Default/description)
#This package contains the PolarSSL library.
#endef

#define Package/polarssl-progs/description
#$(call Package/polarssl/Default/description)
#This package contains the PolarSSL programs.
#endef

PKG_INSTALL:=1

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="Release" \
	-DUSE_SHARED_POLARSSL_LIBRARY:Bool=ON \

define Package/opencv/install
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopencv* $(1)/usr/lib/

endef

$(eval $(call BuildPackage,opencv))
