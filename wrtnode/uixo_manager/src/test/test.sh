#!/bin/sh
ubusd &
./uixo_manager/Debug/uixo_manager &
ubus call uixo_manager mkobj '{"config file":"/home/schumy/git/gitlab/openwrt-packages/wrtnode/uixo_manager/src/uixo_console.conf"}'
