obj-m += statsmod-wheat.o
#obj-m += statsmod-reaper.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

mkjp:
	gcc jp-wheat.c -o jp-wheat

clean:
	rm *~ *.ko *.mod.c *.mod.o *.o
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

jp: all mkjp
	sudo ./jp-wheat

insmod: all
	sudo insmod statsmod-wheat.ko
#	sudo insmod statsmod-reaper.ko
