obj-m += statsmod-wheat.o
#obj-m += statsmod-reaper.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

jp:
	gcc jp-wheat.c -o jp-wheat

clean:
	rm *~ *.ko *.mod.c *.mod.o *.o jp-wheat file-*
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

runjp: all jp
	sudo ./jp-wheat

insmod: all
	sudo insmod statsmod-wheat.ko
#	sudo insmod statsmod-reaper.ko

rmmod:
	sudo rmmod statsmod_wheat

