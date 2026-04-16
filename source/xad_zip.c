/* ZipX client for xadmaster.library
 * chris@unsatisfactorysoftware.co.uk
 */

#ifndef XADMASTER_LIB_C
#define XADMASTER_LIB_C

#include <proto/exec.h>

#ifdef __amigaos4__
struct ExecIFace *IExec;
struct Library *newlibbase;
struct Interface *INewlib;

#include "sys/types.h"
#else
#include <exec/types.h>
#endif

#include <proto/xadmaster.h>
#include <proto/zip.h>
#include <dos/dos.h>
#include "SDI_compiler.h"
#include "xad_zip.h"
#include "Zip_rev.h"

#ifndef XADMASTERFILE
#define xad_zip_Client		FirstClient
#define NEXTCLIENT		0
#ifdef __amigaos4__
#define XADMASTERVERSION	13
#else
#define XADMASTERVERSION	12
#endif
UBYTE *version = VERSTAG;
#endif
#define XADZIP_VERSION	VERSION
#define XADZIP_REVISION	REVISION

struct Library *ZipBase;
#ifdef __amigaos4__
struct ZipIFace *IZip;
struct ExecIFace *IExec;
#endif

void xad_close_zip(void)
{
#ifdef __amigaos4__
	if(IZip)
	{
		DropInterface((struct Interface *)IZip);
		IZip=NULL;
	}
#endif
	if(ZipBase)
	{
		CloseLibrary(ZipBase);
		ZipBase=NULL;
	}

#ifdef __amigaos4__
/*
    DropInterface(INewlib);
    CloseLibrary(newlibbase);
	INewlib = NULL;
	newlibbase = NULL;
*/
#endif
}

BOOL xad_open_zip(void)
{
#ifdef __amigaos4__
    IExec = (struct ExecIFace *)(*(struct ExecBase **)4)->MainInterface;

    newlibbase = OpenLibrary("newlib.library", 52);
    if(newlibbase)
        INewlib = GetInterface(newlibbase, "main", 1, NULL);
#endif

	if(ZipBase = OpenLibrary("zip.library",54))
	{
#ifdef __amigaos4__
		if(IZip = (struct ZipIFace *)GetInterface(ZipBase,"main",1,NULL))
		{
#endif
			return TRUE;
#ifdef __amigaos4__
		}
#endif
	}
	xad_close_zip();
	return FALSE;
}

#ifdef __amigaos4__
BOOL xad_zip_RecogData(ULONG size, STRPTR data,
struct XadMasterIFace *IXadMaster)
#else
ASM(BOOL) sz_RecogData(REG(d0, ULONG size), REG(a0, STRPTR data),
REG(a6, struct xadMasterBase *xadMasterBase))
#endif
{
  if(data[0]=='P' & data[1]=='K' & data[2]==0x03 & data[3]==0x04)
    return 1; /* known file */
  else
    return 0; /* unknown file */
}

#ifdef __amigaos4__
LONG xad_zip_GetInfo(struct xadArchiveInfo *ai,
struct XadMasterIFace *IXadMaster)
#else
ASM(LONG) xad_zip_GetInfo(REG(a0, struct xadArchiveInfo *ai),
REG(a6, struct xadMasterBase *xadMasterBase))
#endif
{
	struct xadFileInfo *fi;
	zip_error_t *zerr = 0;
	long err=XADERR_OK;
	if (!xad_open_zip()) return XADERR_RESOURCE;

	ai->xai_PrivateClient = xadAllocVec(sizeof(struct xadclientprivate),MEMF_PRIVATE | MEMF_CLEAR);
	struct xadclientprivate *xadzip = ai->xai_PrivateClient;


	xadzip->inbuffer = xadAllocVec(ai->xai_InSize, MEMF_CLEAR);
	xadHookAccess(XADAC_READ, ai->xai_InSize, xadzip->inbuffer, ai);

	zip_source_t *zipsrc = zip_source_buffer_create(xadzip->inbuffer, ai->xai_InSize, 0, &zerr);
	
	if(zipsrc) {
		zip_t *zarc = zip_open_from_source(zipsrc, ZIP_RDONLY, &zerr);

		if(zarc) {
// loop

			xadzip->sb = xadAllocVec(sizeof(zip_stat_t), MEMF_PRIVATE | MEMF_CLEAR);

			zip_stat_init(xadzip->sb);

			for( int i = 0; i < zip_get_num_entries(zarc, 0); i++) {
				zip_uint32_t clen = 0;
				
// on error ai->xai_Flags & XADAIF_FILECORRUPT
				fi = (struct xadFileInfo *) xadAllocObjectA(XADOBJ_FILEINFO, NULL);
				if (!fi) return(XADERR_NOMEMORY);

				fi->xfi_DataPos = i;
				if(zip_stat_index(zarc, i, 0, xadzip->sb) == 0) {

					if (xadzip->sb->valid & ZIP_STAT_NAME) fi->xfi_FileName = xadzip->sb->name; // TODO copy this?
					if (xadzip->sb->valid & ZIP_STAT_SIZE) fi->xfi_Size = xadzip->sb->size;
					if (xadzip->sb->valid & ZIP_STAT_COMP_SIZE) fi->xfi_CrunchSize = xadzip->sb->comp_size;
				}

				fi->xfi_Comment = zip_file_get_comment(zarc, i, &clen, 0);
				fi->xfi_Flags = XADFIF_NODATE; // date can be got from sb->mtime

				if ((err = xadAddFileEntryA(fi, ai, NULL))) return(XADERR_NOMEMORY);
// loop end
			}
			
			xadFreeObject(xadzip->sb, NULL);
			xadzip->sb = NULL;
			
		} else {
			return XADERR_NOMEMORY;
		}
	} else {
		return XADERR_NOMEMORY;
	}
	
	return(err);
}

#ifdef __amigaos4__
LONG xad_zip_UnArchive(struct xadArchiveInfo *ai,
struct XadMasterIFace *IXadMaster)
#else
ASM(LONG) xad_zip_UnArchive(REG(a0, struct xadArchiveInfo *ai),
REG(a6, struct xadMasterBase *xadMasterBase))
#endif
{
	struct xadclientprivate *xadzip = ai->xai_PrivateClient;
	struct xadFileInfo *fi = ai->xai_CurFile;
	UBYTE *outbuffer;
	long err=XADERR_OK, ret;

	if (!xad_open_zip()) return XADERR_RESOURCE;

	outbuffer = xadAllocVec(1024, MEMF_CLEAR);

	zip_file_t *zipf = zip_fopen_index(xadzip->zarc, fi->xfi_DataPos, 0);

	if(zipf == NULL) return XADERR_UNKNOWN;

	while(ret = zip_fread(zipf, outbuffer, 1024) > 0) {
		err = xadHookAccess(XADAC_WRITE, ret, outbuffer, ai);
	};

	if(ret == -1) err = XADERR_UNKNOWN;

	xadFreeObjectA(outbuffer, NULL);

	return err;
}

#ifdef __amigaos4__
void xad_zip_Free(struct xadArchiveInfo *ai,
struct XadMasterIFace *IXadMaster)
#else
ASM(void) xad_zip_Free(REG(a0, struct xadArchiveInfo *ai),
REG(a6, struct xadMasterBase *xadMasterBase))
#endif
{
  /* This function needs to free all the stuff allocated in info or
  unarchive function. It may be called multiple times, so clear freed
  entries!
  */

	struct xadclientprivate *xadzip = ai->xai_PrivateClient;
	if(xadzip->zarc) zip_discard(xadzip->zarc);
	xadzip->zarc = NULL;
	xadFreeObject(xadzip->inbuffer, NULL);
	xadFreeObjectA(ai->xai_PrivateClient, NULL);
	ai->xai_PrivateClient = NULL;
	xad_close_zip();
}

/* You need to complete following structure! */
const struct xadClient xad_zip_Client = {
NEXTCLIENT, XADCLIENT_VERSION, XADMASTERVERSION, VERSION, REVISION,
6, XADCF_FILEARCHIVER|XADCF_DATACRUNCHER|XADCF_FREEFILEINFO|XADCF_FREEXADSTRINGS,
XADCID_ZIP /* Type identifier. Normally should be zero */, "Zip",
(BOOL (*)()) xad_zip_RecogData, (LONG (*)()) xad_zip_GetInfo,
(LONG (*)()) xad_zip_UnArchive, (void (*)()) xad_zip_Free };

#ifndef __amigaos4__
void main(void)
{
}
#endif
#endif /* XADMASTER_ZIP_C */
