// DWARF declaration skeleton for game/client/portal2/radialmenu_taunt.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x81190 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientMenuManagerTaunt::ClientMenuManagerTaunt() at line 32
}

// game/client/portal2/radialmenu_taunt.cpp:29
char g_szPositions[8][16];

// game/client/portal2/radialmenu_taunt.cpp:32
ClientMenuManagerTaunt TheClientMenuManagerTaunt;

// game/client/portal2/radialmenu_taunt.cpp:34 (declaration)
void ClearTauntStatusData();

// game/client/portal2/radialmenu_taunt.cpp:34 @0x620b00 _ZN22ClientMenuManagerTaunt20ClearTauntStatusDataEv
void ClientMenuManagerTaunt::ClearTauntStatusData()
{
	{
		int i;  // line 36
		{
			TauntStatusData *pNew;  // line 38
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 38
			// inlined vgui::surface() at line 41
		}
	}
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::RemoveAll() at line 46
}

// game/client/portal2/radialmenu_taunt.cpp:49 @0x621930 _ZN22ClientMenuManagerTaunt8FindMenuEPKc
KeyValues *ClientMenuManagerTaunt::FindMenu( const char *menuName )
{
}

// game/client/portal2/radialmenu_taunt.cpp:56 @0x620f50 _ZN22ClientMenuManagerTaunt5FlushEv
void ClientMenuManagerTaunt::Flush()
{
	KeyValues *pKVData;  // line 63
	// inlined ClientMenuManager::Reset() at line 58
	// inlined ClientMenuManagerTaunt::ClearTauntStatusData() at line 61
	{
		int nNew;  // line 66
		TauntStatusData *pNew;  // line 67
		// inlined vgui::surface() at line 73
		// inlined vgui::surface() at line 72
		// inlined KeyValues::GetBool() at line 71
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 67
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::AddToTail() at line 66
	}
}

// game/client/portal2/radialmenu_taunt.cpp:85 @0x620d20 _ZN22ClientMenuManagerTaunt15KeyValueBuilderEP9KeyValues
void ClientMenuManagerTaunt::KeyValueBuilder( KeyValues *pKeyValues )
{
	{
		int i;  // line 87
		{
			TauntStatusData *pTauntData;  // line 89
			KeyValues *pNewTaunt;  // line 91
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 89
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:102 @0x620ca0 _ZN22ClientMenuManagerTaunt18GetNextTauntToEarnEv
const char *ClientMenuManagerTaunt::GetNextTauntToEarn()
{
	TauntStatusData *pNext;  // line 104
	{
		int i;  // line 105
		{
			TauntStatusData *pTauntData;  // line 107
		}
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 105
	}
}

// game/client/portal2/radialmenu_taunt.cpp:127 @0x621c40 _ZN22ClientMenuManagerTaunt11IsTauntTeamEPKc
bool ClientMenuManagerTaunt::IsTauntTeam( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 129
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 129
}

// game/client/portal2/radialmenu_taunt.cpp:138 @0x621a90 _ZN22ClientMenuManagerTaunt16SetTauntUnlockedEPKc
void ClientMenuManagerTaunt::SetTauntUnlocked( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 140
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 140
	{
		int nLargestDelay;  // line 153
		{
			int j;  // line 154
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 154
			{
				TauntStatusData *pOtherTauntData;  // line 156
			}
		}
	}
	// inlined ClientMenuManagerTaunt::GetNumTauntsPending() at line 149
}

// game/client/portal2/radialmenu_taunt.cpp:173 (declaration)
void SetTauntOwned( const char *pchTaunt );

// game/client/portal2/radialmenu_taunt.cpp:173 @0x621a00 _ZN22ClientMenuManagerTaunt13SetTauntOwnedEPKc
void ClientMenuManagerTaunt::SetTauntOwned( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 175
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 175
}

// game/client/portal2/radialmenu_taunt.cpp:184 @0x621960 _ZN22ClientMenuManagerTaunt12SetTauntUsedEPKc
void ClientMenuManagerTaunt::SetTauntUsed( const char *pchTaunt )
{
	TauntStatusData *pTauntData;  // line 186
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 186
}

// game/client/portal2/radialmenu_taunt.cpp:196 @0x621ea0 _ZN22ClientMenuManagerTaunt13SetTauntDelayEPKci
void ClientMenuManagerTaunt::SetTauntDelay( const char *pchTaunt, int nDelay )
{
	TauntStatusData *pTauntData;  // line 198
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 198
}

// game/client/portal2/radialmenu_taunt.cpp:207 @0x621cb0 _ZN22ClientMenuManagerTaunt16SetTauntPositionEPKcS1_
void ClientMenuManagerTaunt::SetTauntPosition( const char *pchTaunt, const char *pchPosition )
{
	TauntStatusData *pTauntData;  // line 209
	bool bMoveToEmpty;  // line 213
	{
		int nPos;  // line 219
		{
			bool bNotTaken;  // line 222
			{
				int j;  // line 223
			}
		}
	}
	{
		int j;  // line 250
	}
	// inlined ClientMenuManagerTaunt::GetTauntDataByName() at line 209
}

// game/client/portal2/radialmenu_taunt.cpp:266 @0x621f30 _ZN22ClientMenuManagerTaunt23DecreaseOwnershipDelaysEv
void ClientMenuManagerTaunt::DecreaseOwnershipDelays()
{
	bool bChange;  // line 268
	bool bPending;  // line 269
	// inlined ClientMenuManagerTaunt::GetNumTauntsPending() at line 298
	{
		int i;  // line 271
		{
			TauntStatusData *pTauntData;  // line 273
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 273
			// inlined ClientMenuManagerTaunt::SetTauntOwned() at line 287
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:305 @0x620f10 _ZN22ClientMenuManagerTaunt17GetNumTauntsOwnedEv
int ClientMenuManagerTaunt::GetNumTauntsOwned()
{
	int nNumTauntsOwned;  // line 307
	{
		int i;  // line 309
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 309
		{
			TauntStatusData *pTauntData;  // line 311
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:321 (declaration)
void GetNumTauntsPending();

// game/client/portal2/radialmenu_taunt.cpp:321 @0x620b90 _ZN22ClientMenuManagerTaunt19GetNumTauntsPendingEv
int ClientMenuManagerTaunt::GetNumTauntsPending()
{
	int nNumTauntsPending;  // line 323
	{
		int i;  // line 325
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 325
		{
			TauntStatusData *pTauntData;  // line 327
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:337 @0x620ec0 _ZN22ClientMenuManagerTaunt18GetNumTauntsUnusedEv
int ClientMenuManagerTaunt::GetNumTauntsUnused()
{
	int nNumTauntsUnused;  // line 339
	{
		int i;  // line 341
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 341
		{
			TauntStatusData *pTauntData;  // line 343
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:353 @0x620e70 _ZN22ClientMenuManagerTaunt14GetUnusedTauntEi
const TauntStatusData *ClientMenuManagerTaunt::GetUnusedTaunt( int nIndex )
{
	{
		int i;  // line 355
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 355
		{
			TauntStatusData *pTauntData;  // line 357
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:372 @0x81120 _ZN22ClientMenuManagerTaunt19ClearCustomMenuKeysEv
void ClientMenuManagerTaunt::ClearCustomMenuKeys()
{
}

// game/client/portal2/radialmenu_taunt.cpp:383 (declaration)
void AddCustomMenuKey( KeyValues *pKeys );

// game/client/portal2/radialmenu_taunt.cpp:383 @0x620c50 _ZN22ClientMenuManagerTaunt16AddCustomMenuKeyEP9KeyValues
void ClientMenuManagerTaunt::AddCustomMenuKey( KeyValues *pKeys )
{
	KeyValues *pDefaultKey;  // line 385
}

// game/client/portal2/radialmenu_taunt.cpp:391 @0x620be0 _ZN22ClientMenuManagerTaunt15GetAllTauntKeysEv
KeyValues *ClientMenuManagerTaunt::GetAllTauntKeys()
{
}

// game/client/portal2/radialmenu_taunt.cpp:396 @0x621280 _ZN22ClientMenuManagerTaunt19UpdateDefaultTauntsEv
void ClientMenuManagerTaunt::UpdateDefaultTaunts()
{
	C_Portal_Player *pPlayer;  // line 398
	C_Portal_Player *pPartnerPlayer;  // line 399
	bool bTeamAccept;  // line 400
	bool bPositionsFilled[8];  // line 443
	{
		int nPosition;  // line 484
		{
			KeyValues *pNewTaunt;  // line 489
			// inlined ClientMenuManagerTaunt::AddCustomMenuKey() at line 494
			// inlined KeyValues::SetBool() at line 492
		}
	}
	{
		int i;  // line 446
		{
			TauntStatusData *pTauntData;  // line 448
			bool bIsReadyTeamTaunt;  // line 449
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 448
			{
				KeyValues *pNewTaunt;  // line 453
				// inlined ClientMenuManagerTaunt::AddCustomMenuKey() at line 468
				{
					int nPosition;  // line 470
				}
			}
		}
	}
	// inlined C_Portal_Player::HasTauntPartnerInRange() at line 399
	// inlined C_Portal_Player::GetLocalPortalPlayer() at line 398
	{
		int i;  // line 417
		{
			TauntStatusData *pTauntData;  // line 419
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 419
		}
	}
	{
		int i;  // line 427
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::Count() at line 427
		{
			TauntStatusData *pTauntData;  // line 429
			// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 429
			// inlined ClientMenuManagerTaunt::GetNumTauntsPending() at line 435
		}
	}
}

// game/client/portal2/radialmenu_taunt.cpp:500 (declaration)
void GetTauntDataByName( const char *pchTaunt );

// game/client/portal2/radialmenu_taunt.cpp:500 @0x620bf0 _ZN22ClientMenuManagerTaunt18GetTauntDataByNameEPKc
TauntStatusData *ClientMenuManagerTaunt::GetTauntDataByName( const char *pchTaunt )
{
	{
		int i;  // line 502
		// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::operator[]() at line 506
	}
}

// public/tier1/utlmemory.h:707 @0x622140 _ZN10CUtlMemoryI15TauntStatusDataiE4GrowEi
void CUtlMemory<TauntStatusData,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined MemAlloc_Alloc() at line 761
	// inlined CUtlMemory<TauntStatusData,int>::IsExternallyAllocated() at line 711
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
}
