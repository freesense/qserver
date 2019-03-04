//VariantTrans.h
#ifndef _VARIANTTRANS_H_
#define _VARIANTTRANS_H_
#include <comutil.h>
//////////////////////////////////////////////////////////////////////////
class CVariantTrans
{
public:
	static bool TransVar(const _variant_t& var, bool& value);
	static bool TransVar(const _variant_t& var, unsigned char& value);
	static bool TransVar(const _variant_t& var, short& value);
};
//////////////////////////////////////////////////////////////////////////
#endif