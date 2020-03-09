obj-m += ../parser/parser.o
obj-m += reader.o

reader-objs := ps_reader.o queue.o
parser-objs := ../parser/ps_parser.o ../parser/queue.o ../parser/list.o

VPATH += ../parser/ ../reader

export KVERSION = $(shell uname -r)/build

alltofit: modules
modules:
	cd ../parser && $(MAKE) && cd ../reader && \
	make -C ../reader -C /lib/modules/$(KVERSION) M=$(PWD) modules
	#cd ../reader
	#make -C /lib/modules/$(KVERSION) M=$(PWD) modules
	gcc -o caller.o caller.c
clean:
	make -C /lib/modules/$(KVERSION) M=$(PWD) clean
#../parser/parser.o: ../parser/ps_parser.c ../parser/queue.c ../parser/queue.h \
#			../parser/list.c ../parser/list.h
#	cd ../parser/
#	make -C /lib/modules/$(KVERSION) M=$(PWD) modules
#	cd ../reader
	
#queue.o:
#	gcc -o queue.o queue.c queue.h

parser:
	cd ../parser && \
	make -C /lib/modules/$(KVERSION) M=$(PWD) modules && \
	cd ../reader
reader:
	make -C /lib /modules/$(KVERSION) M=$(PWD) modules
