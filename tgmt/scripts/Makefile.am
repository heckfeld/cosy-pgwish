ACLOCAL_AMFLAGS = -I m4

dist_bin_SCRIPTS = tinfo tip tload troute xtload tboot treset tgtres

install-exec-hook:
	sudo chown root:root  $(DESTDIR)$(bindir)/tip
	sudo chmod u+s $(DESTDIR)$(bindir)/tip
