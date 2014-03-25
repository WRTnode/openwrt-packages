#!/bin/sh

cd /root/uARM_driver/uARM_driver/uARM 

#fd=3
#tty=/dev/ttyACM0

#exec ${fd} < ${tty}

#for x in $(seq -180 10 180) 
#do
#	for y in $(seq 0 1 200) 
#	do
#		sudo ./uARM_driver ${x} ${y} -U
#		sleep 1
#	done
#done

for y in $(seq 0 1 200) 
do
	./uARM_driver 0 ${y} -U
	usleep 1000
done
./uARM_driver 0 0 -U  
sleep 1
for x in $(seq 0 1 360) 
do
	t=180
	i=$((x-t))
	./uARM_driver ${i} 0 -U
	usleep 1000
done


