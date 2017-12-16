ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I../include -I$(top_srcdir)/include

bin_PROGRAMS = download
download_SOURCES = download.c version.c

LDADD = ../libtools/libtools.la

install-exec-hook:
	sudo chown root:root  $(DESTDIR)$(bindir)/download$(EXEEXT)
	sudo chmod u+s $(DESTDIR)$(bindir)/download$(EXEEXT)
