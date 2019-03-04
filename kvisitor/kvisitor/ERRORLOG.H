//---------------------------------------------------------------------------
#ifndef errorlogH
#define errorlogH

#ifdef __BORLANDC__
#include <dir.h>
#else
#define MAXPATH   _MAX_PATH
#define MAXDRIVE  _MAX_DRIVE
#define MAXDIR    _MAX_DIR
#define MAXFILE   _MAX_FNAME
#define MAXEXT    _MAX_EXT
#define fnsplit   _splitpath
#define fnmerge   _makepath
#define access    _access
#endif

const char * GetErrorLogFileName();

//---------------------------------------------------------------------------
#endif
