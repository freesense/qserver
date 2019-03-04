//QuoteFeedService.cpp
#include "stdafx.h"
#include "QuoteFeedService.h"

extern int InitFeed();
extern void UninitFeed();
///////////////////////////////////////////////////////////////////////////////
CQuoteFeedService::CQuoteFeedService()
{
}

CQuoteFeedService::~CQuoteFeedService()
{
}

void CQuoteFeedService::Run()
{
	InitFeed();
	CNTService::Run();
}

void CQuoteFeedService::Stop()
{
	CNTService::Stop();
	UninitFeed();
}