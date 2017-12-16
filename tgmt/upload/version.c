
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

static char Version[BUFSIZ];
char *printVersion() {
	int nprint = sprintf( Version, "%s compiled on %s by %s", PACKAGE_STRING, __DATE__, PACKAGE_BUGREPORT);
	return Version;
}

