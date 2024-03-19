#ifndef _ED_SIG_H
#define _ED_SIG_H

#include "ed.h"

/* Registers useful signals to handle. */
void ed_sig_reg(Ed *);

/* Unregsters signal handlers. */
void ed_sig_unreg(void);

#endif /* _ED_SIG_H */
