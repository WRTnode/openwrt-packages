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
PKG_MD5SUM:=50cc1433b3654074206f5b3dbfd49848

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk


define Package/opencv/Default/description
the Open Source Computer Vision Library for linux
endef

define Package/opencv
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:= opencv-2.4.8
  DEPENDS:=+libpthread +librt +libstdcpp +zlib +libjpeg 
endef


PKG_INSTALL:=1

CMAKE_OPTIONS += -DBUILD_opencv_gpu:BOOL=OFF -DWITH_LIBV4L:BOOL=OFF \
	 -DWITH_1394:BOOL=OFF -DBUILD_opencv_stitching:BOOL=OFF \
	 -DBUILD_opencv_superres:BOOL=OFF -DBUILD_opencv_ts:BOOL=OFF 

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/opencv $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/opencv2 $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopencv* $(1)/usr/lib/
endef

define Package/opencv/install
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopencv* $(1)/usr/lib/

endef

$(eval $(call BuildPackage,opencv))
