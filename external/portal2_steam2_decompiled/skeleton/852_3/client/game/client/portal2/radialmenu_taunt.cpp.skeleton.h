// DWARF declaration skeleton for game/client/portal2/radialmenu_taunt.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/radialmenu_taunt.cpp:0 @0x614e80 _ZN22ClientMenuManagerTaunt5FlushEv
void ClientMenuManagerTaunt::Flush()
{
	KeyValues *pKVData;  // line 59
	// inlined ClientMenuManager::Reset() at line 54
	// inlined ClientMenuManagerTaunt::ClearTauntStatusData() at line 57
	{
		int nNew;  // line 62
		TauntStatusData *pNew;  // line 63
		// inlined vgui::surface() at line 69
		// inlined vgui::surface() at line 68
		// inlined KeyValues::GetBool() at line 67
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 63
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::AddToTail() at line 62
	}
}

// None:0 @0x82890 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ClientMenuManagerTaunt::ClientMenuManagerTaunt() at line 28
}

// game/client/portal2/radialmenu_taunt.cpp:25
char g_szPositions[8][16];

// game/client/portal2/radialmenu_taunt.cpp:28
ClientMenuManagerTaunt TheClientMenuManagerTaunt;

// game/client/portal2/radialmenu_taunt.cpp:30 (declaration)
void ClearTauntStatusData();

// game/client/portal2/radialmenu_taunt.cpp:30 @0x614ad0 _ZN22ClientMenuManagerTaunt20ClearTauntStatusDataEv
void ClientMenuManagerTaunt::ClearTauntStatusData()
{
	{
		int i;  // line 32
		{
			TauntStatusData *pNew;  // line 34
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 34
			// inlined vgui::surface() at line 37
		}
	}
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::RemoveAll() at line 42
}

// game/client/portal2/radialmenu_taunt.cpp:45 @0x615720 _ZN22ClientMenuManagerTaunt8FindMenuEPKc
KeyValues *ClientMenuManagerTaunt::FindMenu( const char *menuName )
{
}

// game/client/portal2/radialmenu_taunt.cpp:79 @0x614c70 _ZN22ClientMenuManagerTaunt15KeyValueBuilderEP9KeyValues
void ClientMenuManagerTaunt::KeyValueBuilder( KeyValues *pKeyValues )
{
	{
		int i;  // line 81
		{
			TauntStatusData *pTauntData;  // line 83
			KeyValues *pNewTaunt;  // line 85
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 83
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:96 @0x614dc0 _ZN22ClientMenuManagerTaunt18GetNextTauntToEarnEv
const char *ClientMenuManagerTaunt::GetNextTauntToEarn()
{
	TauntStatusData *pNext;  // line 98
	{
		int i;  // line 99
		{
			TauntStatusData *pTauntData;  // line 101
		}
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 99
	}
}

// game/client/portal2/radialmenu_taunt.cpp:121 @0x615cb0 _ZN22ClientMenuManagerTaunt11IsTauntTeamEPKc
bool ClientMenuManagerTaunt::IsTauntTeam( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 123
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 123
}

// game/client/portal2/radialmenu_taunt.cpp:132 @0x615b00 _ZN22ClientMenuManagerTaunt16SetTauntUnlockedEPKc
void ClientMenuManagerTaunt::SetTauntUnlocked( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 134
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 134
	{
		int nLargestDelay;  // line 147
		{
			int j;  // line 148
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 148
			{
				TauntStatusData *pOtherTauntData;  // line 150
			}
		}
	}
	// inlined ClientMenuManagerTaunt::GetNumTauntsPending() at line 143
}

// game/client/portal2/radialmenu_taunt.cpp:167 (declaration)
void SetTauntOwned( const char *pchTaunt );

// game/client/portal2/radialmenu_taunt.cpp:167 @0x615a70 _ZN22ClientMenuManagerTaunt13SetTauntOwnedEPKc
void ClientMenuManagerTaunt::SetTauntOwned( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 169
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 169
}

// game/client/portal2/radialmenu_taunt.cpp:178 @0x6159d0 _ZN22ClientMenuManagerTaunt12SetTauntUsedEPKc
void ClientMenuManagerTaunt::SetTauntUsed( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 180
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 180
}

// game/client/portal2/radialmenu_taunt.cpp:190 @0x615940 _ZN22ClientMenuManagerTaunt13SetTauntDelayEPKci
void ClientMenuManagerTaunt::SetTauntDelay( const char *pchTaunt, int nDelay )
{
	TauntStatusData *pTauntData;  // line 192
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 192
}

// game/client/portal2/radialmenu_taunt.cpp:201 @0x615750 _ZN22ClientMenuManagerTaunt16SetTauntPositionEPKcS1_
void ClientMenuManagerTaunt::SetTauntPosition( const char *pchTaunt, const char *pchPosition )
{
	TauntStatusData *pTauntData;  // line 203
	bool bMoveToEmpty;  // line 207
	{
		int nPos;  // line 213
		{
			bool bNotTaken;  // line 216
			{
				int j;  // line 217
			}
		}
	}
	{
		int j;  // line 244
	}
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 203
}

// game/client/portal2/radialmenu_taunt.cpp:260 @0x615d20 _ZN22ClientMenuManagerTaunt23DecreaseOwnershipDelaysEv
void ClientMenuManagerTaunt::DecreaseOwnershipDelays()
{
	bool bChange;  // line 262
	bool bPending;  // line 263
	// inlined ClientMenuManagerTaunt::GetNumTauntsPending() at line 292
	{
		int i;  // line 265
		{
			TauntStatusData *pTauntData;  // line 267
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 267
			// inlined ClientMenuManagerTaunt::SetTauntOwned() at line 281
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:299 @0x614e40 _ZN22ClientMenuManagerTaunt17GetNumTauntsOwnedEv
int ClientMenuManagerTaunt::GetNumTauntsOwned()
{
	int nNumTauntsOwned;  // line 301
	{
		int i;  // line 303
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 303
		{
			TauntStatusData *pTauntData;  // line 305
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:315 (declaration)
void GetNumTauntsPending();

// game/client/portal2/radialmenu_taunt.cpp:315 @0x614b60 _ZN22ClientMenuManagerTaunt19GetNumTauntsPendingEv
int ClientMenuManagerTaunt::GetNumTauntsPending()
{
	int nNumTauntsPending;  // line 317
	{
		int i;  // line 319
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 319
		{
			TauntStatusData *pTauntData;  // line 321
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:331 @0x82820 _ZN22ClientMenuManagerTaunt19ClearCustomMenuKeysEv
void ClientMenuManagerTaunt::ClearCustomMenuKeys()
{
}

// game/client/portal2/radialmenu_taunt.cpp:342 (declaration)
void AddCustomMenuKey( KeyValues *pKeys );

// game/client/portal2/radialmenu_taunt.cpp:342 @0x614c20 _ZN22ClientMenuManagerTaunt16AddCustomMenuKeyEP9KeyValues
void ClientMenuManagerTaunt::AddCustomMenuKey( KeyValues *pKeys )
{
	KeyValues *pDefaultKey;  // line 344
}

// game/client/portal2/radialmenu_taunt.cpp:350 @0x614bb0 _ZN22ClientMenuManagerTaunt15GetAllTauntKeysEv
KeyValues *ClientMenuManagerTaunt::GetAllTauntKeys()
{
}

// game/client/portal2/radialmenu_taunt.cpp:355 @0x6151b0 _ZN22ClientMenuManagerTaunt19UpdateDefaultTauntsEv
void ClientMenuManagerTaunt::UpdateDefaultTaunts()
{
	bool bPositionsFilled[8];  // line 391
	{
		int i;  // line 394
		{
			TauntStatusData *pTauntData;  // line 396
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 396
			{
				KeyValues *pNewTaunt;  // line 399
				// inlined KeyValues::SetBool() at line 402
				// inlined ClientMenuManagerTaunt::AddCustomMenuKey() at line 404
				{
					int nPosition;  // line 406
				}
			}
		}
	}
	{
		int nPosition;  // line 420
		{
			KeyValues *pNewTaunt;  // line 425
			// inlined KeyValues::SetBool() at line 428
			// inlined ClientMenuManagerTaunt::AddCustomMenuKey() at line 430
		}
	}
	{
		int i;  // line 365
		{
			TauntStatusData *pTauntData;  // line 367
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 367
		}
	}
	{
		int i;  // line 375
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 375
		{
			TauntStatusData *pTauntData;  // line 377
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 377
			// inlined ClientMenuManagerTaunt::GetNumTauntsPending() at line 383
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:436 (declaration)
void GetTauntDataByName( const char *pchTaunt );

// game/client/portal2/radialmenu_taunt.cpp:436 @0x614bc0 _ZN22ClientMenuManagerTaunt18GetTauntDataByNameEPKc
TauntStatusData *ClientMenuManagerTaunt::GetTauntDataByName( const char *pchTaunt )
{
	{
		int i;  // line 438
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 442
	}
}

// public/tier1/utlmemory.h:707 @0x615f30 _ZN10CUtlMemoryI15TauntStatusDataiE4GrowEi
void CUtlMemory<TauntStatusData,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined MemAlloc_Alloc() at line 761
	// inlined CUtlMemory<TauntStatusData,int>::IsExternallyAllocated() at line 711
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
}
