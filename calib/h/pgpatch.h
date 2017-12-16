/* $Header: /mnt/cc-x4/ccp/database/h/rcs/pgpatch.h,v 1.1 2003/02/03 15:11:17 tine Exp $ */

#ifndef PGPATCH_INCLUDED
# define PGPATCH_INCLUDED

# ifdef hpux
  /* Unter HP-UX gibt es diesen Bool-Typ noch nicht !!! */
  typedef enum {false, true} bool;
# endif

#endif
