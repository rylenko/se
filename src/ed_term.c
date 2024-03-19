#include "ed.h"
#include "ed_sig.h"
#include "ed_term.h"
#include "term.h"

void
ed_term_deinit(void)
{
	term_disable_raw_mode();
	ed_sig_unreg();
}

void
ed_term_init(Ed *const ed, const int ifd, const int ofd)
{
	term_init(ifd, ofd);
	term_enable_raw_mode();
	term_get_win_size(&ed->win_size);
	ed_sig_reg(ed);
}
