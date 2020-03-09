obj-m += parser.o
obj-m += ../reader/reader.o
obj-m += ../analyser/analyser.o
obj-m += ../keylog/keylog.o

PARSER_PATH		:= ../parser
READER_PATH 	:= ../reader
ANALYSER_PATH 	:= ../analyser
KEYLOG_PATH 	:= ../keylog

parser-objs := ps_parser.o queue.o list.o HashTable.o

KROOT = /lib/modules/$(shell uname -r)/build

alltofit: modules

modules:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules
	gcc -o $(READER_PATH)/caller.o $(READER_PATH)/caller.c
modules_install:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules_install
clean:
	rm -rf Module.symvers modules.order
	rm -rf *.o *.mod.c *.ko *.cmd *.d
	cd $(READER_PATH)
	rm -rf *.o *.mod.c *.ko *.cmd *.d
	cd $(ANALYSER_PATH)
	rm -rf *.o *.mod.c *.ko *.cmd *.d
	cd $(KEYLOG_PATH)
	rm -rf modules.order Modules.symvers
	rm -rf *.o *.mod.c *.ko *.cmd *.d
	cd $(PARSER_PATH)
queue.o:
	gcc -o queue.o queue.c queue.h
list.o:
	gcc -o list.o list.c list.h
HashTable.o:
	gcc -o HashTable.o HashTable.c HashTable.h
