#
# Regular cron jobs for the tcl7.3 package
#
0 4	* * *	root	[ -x /usr/bin/tcl7.3_maintenance ] && /usr/bin/tcl7.3_maintenance
