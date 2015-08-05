#!/bin/sh

MESSAGE=

case $1 in
    "-m")
        MESSAGE="[1234:1:0:0:0:$3:$2:mkport] 0"
        /bin/client_console 127.0.0.1 ${MESSAGE}
        ;;
    "-r")
        MESSAGE="[1234:1:0:0:0:0:$2:rmport] 0"
        ;;
    "-s")
        DATA=$3
        MESSAGE="[1234:${#DATA}:0:${DATA}:$4:0:$2:hlport] 0"
        ;;
    *)
        echo "Illegal parameters!"
        exit -1
        ;;
esac

#echo {\"objname\":\"uixo_console\",\"data\":\"${MESSAGE}\",\"len\":${#MESSAGE}}
#ubus call uixo_manager wagent {\"objname\":\"uixo_console\",\"data\":\"${MESSAGE}\",\"len\":${#MESSAGE}}
#ubus call uixo_manager ragent {\"objname\":\"uixo_console\",\"len\":2}


echo 0
