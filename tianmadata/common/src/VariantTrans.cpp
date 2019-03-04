//VariantTrans.cpp
#include "stdafx.h"
#include "VariantTrans.h"
/////////////////////////////////////////////////////////////////////////
bool CVariantTrans::TransVar(const _variant_t& var, bool& value)
{
	switch (var.vt)
	{
	case VT_BOOL:
		value = var.boolVal ? true : false;
	case VT_EMPTY:
	case VT_NULL:
		break;
	default:
		return false;
	}
	return true;
}

bool CVariantTrans::TransVar(const _variant_t& var, unsigned char& value)
{
	switch (var.vt)
	{
	case VT_I1:
	case VT_UI1:
		value = var.bVal;
		break;
	case VT_NULL:
	case VT_EMPTY:
		value = 0;
		break;
	default:
		return false;
	}	
	return true;
}

bool CVariantTrans::TransVar(const _variant_t& var, short& value)
{
	switch (var.vt)
	{
	case VT_BOOL:
		value = var.boolVal;
		break;
	case VT_UI1:
	case VT_I1:
		value = var.bVal;
		break;
	case VT_I2:
	case VT_UI2:
		value = var.iVal;
		break;
	case VT_NULL:
	case VT_EMPTY:
		value = 0;
		break;
	default:
		return false;
	}	
	return true;
}