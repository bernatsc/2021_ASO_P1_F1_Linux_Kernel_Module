EXECUTABLE= buttons

obj-m := $(EXECUTABLE).o

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

install:
	sudo insmod $(EXECUTABLE).ko

delete:
	sudo rmmod $(EXECUTABLE).ko

clean: 
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean