ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I../include -I$(top_srcdir)/include \
	-DHOST=\"`hostname`\" -DUSER=\"`whoami`\"


AM_CFLAGS = -Wall

bin_PROGRAMS = tip
tip_SOURCES = tip.c
tip_LDADD = -lpcap

install-exec-hook:
	sudo chown root:root  $(DESTDIR)$(bindir)/tip$(EXEEXT)
	sudo chmod u+s $(DESTDIR)$(bindir)/tip$(EXEEXT)

