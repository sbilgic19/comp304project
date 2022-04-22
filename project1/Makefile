gcc:
	gcc -o shellfyre.o shellfyre.c
shell:
	gcc -o shellfyre.o shellfyre.c && ./shellfyre.o
testo: 
	gcc -o test.o test.c && ./test.o

obj-m	:= my_module.o
 
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)
 
all: default
 
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
 
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions .cache.mk
