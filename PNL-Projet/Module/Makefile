.PHONY: all clean
#.SECONDARY:

ifneq ($(KERNELRELEASE),)

obj-m := exec_commande.o moduleTest.o
exec_commande-objs := commande.o list.o kill.o meminfo.o modinfo.o wait.o fg.o


else

  KERNELDIR ?= /home/faucon/Documents/M1/PNL/TP-03/linux-4.2.3/
  PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm -f  *.*~ *~ 

endif
