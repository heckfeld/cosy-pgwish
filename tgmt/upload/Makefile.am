ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I../include -I$(top_srcdir)/include

bin_PROGRAMS = upload
upload_SOURCES = upload.c version.c

LDADD = ../libtools/libtools.la

install-exec-hook:
	sudo chown root:root  $(DESTDIR)$(bindir)/upload$(EXEEXT)
	sudo chmod u+s $(DESTDIR)$(bindir)/upload$(EXEEXT)
