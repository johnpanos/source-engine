//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//

#include "filesystem.h"
#include "dedicated.h"
#include <stdio.h>
#include <stdlib.h>
#include "interface.h"
#include <string.h>
#ifdef OSX
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tier0/dbg.h"
#include "../filesystem/basefilesystem.h"
#include "appframework/AppFramework.h"
#include "tier2/tier2.h"


extern IFileSystem *g_pFileSystem;
extern IBaseFileSystem *g_pBaseFileSystem;
