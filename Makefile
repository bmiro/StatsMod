obj-m += statsmod-wheat.o
#obj-m += statsmod-reaper.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm *~ *.ko *.mod.c *.mod.o *.o
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insmod: all
	sudo insmod statsmod-wheat.ko
#	sudo insmod statsmod-reaper.ko
