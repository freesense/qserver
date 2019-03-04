
#ifndef __QUOTE_FEED_INTERFACE_H__
#define __QUOTE_FEED_INTERFACE_H__

#ifdef _DLL
#define FEEDAPI __declspec(dllexport)
#else
#define FEEDAPI __declspec(dllimport)
#endif

struct IFeedOwner
{
    virtual void Quote(const char *lpData, unsigned int nLength, void *lpParam) = 0;
    virtual void Report(const char *mn, int tid, const char *fn, int ln, const char *sr, int lv) = 0;
};

extern "C" bool FEEDAPI Initialize(int argc, char *argv[], IFeedOwner *pOwner);
extern "C" bool FEEDAPI Request(char *lpData, unsigned int nLength, void *lpParam);
extern "C" void FEEDAPI Release();
extern "C" void FEEDAPI GetModuleVer(char** pBuf);
#endif
