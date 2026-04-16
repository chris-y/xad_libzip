#ifndef XAD_ZIP_H
#define XAD_ZIP_H

#include <exec/types.h>
#include <proto/zip.h>

#ifdef __amigaos4__
#define PACKED  __attribute__((__packed__))
#else
#define PACKED  
#endif

#ifndef MEMF_PRIVATE
#define MEMF_PRIVATE MEMF_ANY
#endif

struct xadclientprivate {
	zip_source_t *zipsrc;
	zip_t *zarc;
	zip_stat_t *sb;
	UBYTE *inbuffer;
};

#endif
