/* $Header$ */

#ifndef BASENAME_INCLUDED
# define BASENAME_INCLUDED

# include <string.h>

/*
 * Nur der Name des Programms soll bei der Usage mit ausgegeben werden.
 */

# define basename(name,ptr) (((ptr = strrchr (name, '/')) != (char *) 0) \
                        ? ptr + 1 : name)

#endif
