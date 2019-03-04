
#ifndef __KVISITOR_PUBLIC_H__
#define __KVISITOR_PUBLIC_H__

#include <string>
#include "data_struct.h"
#include "commx/commxapi.h"

using std::string;

void COMMXAPI kdb_create();
string COMMXAPI kdb_addkline(dayk &kline);
bool COMMXAPI kdb_addayk(const char *mkt, string &sql);
bool COMMXAPI kdb_update_history(const char *lpmkt);
bool COMMXAPI kdb_update_weight();
unsigned int COMMXAPI kdb_getdayk(dayk *pdk, unsigned int num, const char *lpSymbol, unsigned int dayfrom);
unsigned int COMMXAPI kdb_get5dayvol(const char *lpSymbol, unsigned int today);
int COMMXAPI kdb_getlastestday(const char *lpSymbol);

#endif
