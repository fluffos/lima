/* Do not remove the headers from this file! see /USAGE for more info. */

#if BODY_STATS == BODY_STATS_LIMA
#include "m_bodystats_lima.c"
#elif BODY_STATS == BODY_STATS_DND
#include "m_bodystats_dnd.c"
#elif BODY_STATS == BODY_STATS_RIFTS
#include "m_bodystats_rifts.c"
#endif
