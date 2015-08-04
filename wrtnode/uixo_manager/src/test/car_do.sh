#!/bin/sh

echo "car do - begin"
/bin/uixo_console_do.sh -m /dev/ttyS0 115200

echo "test command"
/bin/uixo_console_do.sh -s /dev/ttyS0 "\$M<0uu" 0

sleep 3

/bin/uixo_console_do.sh -s /dev/ttyS0 "\$M<0hh" 0

/bin/uixo_console_do.sh -r /dev/ttyS0
echo "car do - end"
