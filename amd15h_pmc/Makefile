obj-m	+= amd15h_pmc.o
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc -g -W -Wall read-pmc.c -o read-pmc

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f modules.order amd15h_pmc.tar.gz read-pmc

dist:
	make clean
	tar -C.. -cvzf amd15h_pmc.tar.gz		\
	    --exclude=\.git --exclude=\.gitignore	\
	    --exclude=amd15h_pmc.tar.gz			\
	    $(shell basename `pwd`)
