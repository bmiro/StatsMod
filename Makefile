obj-m += statsmodwheat.o
obj-m += statsmodreaper.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm *~ *.ko
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
