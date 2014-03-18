#
# Copyright (c) 2013 Arduino LLC. All right reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=yun-scripts
PKG_VERSION:=1.0.0
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/yun-scripts
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Custom Yún scripts
endef

define Build/Compile
	# NOOP
endef

define Package/yun-scripts/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_BIN) ./files/usr/bin/blink-start $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/blink-stop $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/update-file-available $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/run-sysupgrade $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/wifi-live-or-reset $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/wifi-reset-and-reboot $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/reset-to-factory-anyway $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/run-avrdude $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/merge-sketch-with-bootloader.lua $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/wifi-reset-button-pressed $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/wifi-reset-button-released $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/kill-bridge $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/run-bridge $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/reset-mcu $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/special-storage-available $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/pretty-wifi-info.lua $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/prod $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/cali $(1)/usr/bin

	$(INSTALL_BIN) ./files/etc/init.d/rename-wifi-if-access-point $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/generate_new_gpg_key $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/delete_uhttpd_cert $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/handle_wifi_reset $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/rngd-turn-off $(1)/etc/init.d
endef

$(eval $(call BuildPackage,yun-scripts))

