#!/bin/sh

MESSAGE=

case $1 in
    "-m")
        MESSAGE="[1234:1:0:0:0:$3:$2:mkport]\n"
        ;;
    "-r")
        MESSAGE="[1234:1:0:0:0:0:$2:rmport]\n"
        ;;
    "-s")
        DATA=$3
        MESSAGE="[1234:${#DATA}:0:${DATA}:$4:0:$2:hlport]\n"
        ;;
    *)
        echo "Illegal parameters!"
        exit -1
        ;;
esac

PAYLOAD={\"objname\":\"uixo_console\",\"data\":\"${MESSAGE}\",\"len\":${#MESSAGE}}
ubus call uixo_manager wagent ${PAYLOAD}

echo 0
