obj-m += keylog.o
#obj-m += ../analyser/analyser.o
#keylog-objs := keylog_test.o queue.o

export KROOT=/lib/modules/$(shell uname -r)/build

allofit:  modules

modules:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules
modules_install:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules_install
kernel_clean:
	@$(MAKE) -C $(KROOT) M=$(PWD) clean
clean: kernel_clean
	rm -rf   Module.symvers modules.order
queue.o:
	gcc -o queue.o queue.c queue.h
