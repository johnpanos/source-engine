//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dedicated host behavior independent of the preserved mod ABI.
//
//=============================================================================

#ifndef DEDICATED_EXPORTS_SERVICE_H
#define DEDICATED_EXPORTS_SERVICE_H

class IDedicatedExportsService
{
public:
	virtual ~IDedicatedExportsService() = default;
	virtual void Print( char *text ) = 0;
	virtual void RunServer() = 0;
};

using DedicatedExportsServiceFactory = IDedicatedExportsService *(*)();
using DedicatedExportsServiceBinder = void ( * )( IDedicatedExportsService * );

IDedicatedExportsService *Dedicated_CreateExportsService();
void Dedicated_BindExportsService( IDedicatedExportsService *service );

#endif // DEDICATED_EXPORTS_SERVICE_H
