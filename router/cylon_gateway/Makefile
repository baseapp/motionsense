include $(TOPDIR)/rules.mk

PKG_NAME:=cylon_gateway
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS := +lua

include $(INCLUDE_DIR)/package.mk

define Package/cylon_gateway
	CATEGORY:=cylon
	DEFAULT:=y
	TITLE:=cylon_gateway
	DEPENDS:= +lua +liblua +libc +luabitop +luaposix +luasocket
endef

define Package/cylon_gateway/description
Recieves packets from cylon nodes and forwards them to server
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/src
endef

define Build/Compile
endef

define Package/cylon_gateway/install
	$(INSTALL_DIR) $(1)/usr/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/bin/cylon
	$(INSTALL_BIN) src/dkjson.lua $(1)/usr/bin/cylon
	$(INSTALL_BIN) src/gateway-serial.sh $(1)/usr/bin/cylon
	$(INSTALL_BIN) src/gateway-serial.lua $(1)/usr/bin/cylon
	$(INSTALL_BIN) src/procutil.lua $(1)/usr/bin/cylon
	$(INSTALL_BIN) src/request.lua $(1)/usr/bin/cylon
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/uci-defaults/
	$(INSTALL_BIN) src/cylon_gateway.init $(1)/etc/init.d/cylon_gateway
	$(INSTALL_BIN) src/cylon.defaults $(1)/etc/uci-defaults/40_cylon
endef

$(eval $(call BuildPackage,cylon_gateway))
