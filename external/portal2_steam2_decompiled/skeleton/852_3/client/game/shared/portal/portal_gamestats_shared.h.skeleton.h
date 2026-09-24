// DWARF declaration skeleton for game/shared/portal/portal_gamestats_shared.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/portal_gamestats_shared.h:80 sizeof=0x1 (i386)
struct UniqueStatID_t<SConfigStats>
{
public:
	static void Reset();  // line 87
private:
	static unsigned int s_nLastID;  // line 97
};

// game/shared/portal/portal_gamestats_shared.h:80 sizeof=0x1 (i386)
struct UniqueStatID_t<SSurveyData>
{
public:
	static void Reset();  // line 87
private:
	static unsigned int s_nLastID;  // line 97
};

// game/shared/portal/portal_gamestats_shared.h:110 sizeof=0x4 (i386)
struct IGameStatTracker
{
public:
	int (**_vptr$IGameStatTracker)(); // +0x0  // line 0
	void SubmitStat<SConfigStats>( SConfigStats & );  // line 118
	void SubmitStat<SConfigStats>( SConfigStats * );  // line 135
	void SubmitStat<SSurveyData>( SSurveyData * );  // line 135
	virtual void SubmitGameStats( KeyValues * );  // line 144
	void PrintGamestatMemoryUsage();  // line 149
	// game/shared/portal/portal_gamestats_shared.h:157 sizeof=0x4 (i386)
	struct IStatContainer
	{
	public:
		int (**_vptr$IStatContainer)(); // +0x0  // line 0
		virtual void SendData( KeyValues * );  // line 159
		virtual void Clear();  // line 160
		virtual void PrintMemoryUsage();  // line 161
	};  // line 157
protected:
	virtual CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> > *GetStatContainerList();  // line 170
private:
	CGameStatList<SConfigStats> *GetStatTable<SConfigStats>();  // line 228
	CGameStatList<SSurveyData> *GetStatTable<SSurveyData>();  // line 228
};

// game/shared/portal/portal_gamestats_shared.h:157 (declaration)
void IStatContainer();

// game/shared/portal/portal_gamestats_shared.h:180 sizeof=0x18 (i386)
struct CGameStatList<SSurveyData> : public IGameStatTracker::IStatContainer, public CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> >
{
public:
	virtual void SendData( KeyValues * );  // line 185
	virtual void Clear();  // line 204
	virtual void PrintMemoryUsage();  // line 212
};

// game/shared/portal/portal_gamestats_shared.h:180 sizeof=0x18 (i386)
struct CGameStatList<SConfigStats> : public IGameStatTracker::IStatContainer, public CUtlVector<SConfigStats*,CUtlMemory<SConfigStats*, int> >
{
public:
	virtual void SendData( KeyValues * );  // line 185
	virtual void Clear();  // line 204
	virtual void PrintMemoryUsage();  // line 212
};

// game/shared/portal/portal_gamestats_shared.h:180 (declaration)
void CGameStatList();

// game/shared/portal/portal_gamestats_shared.h:185 @0x4df8a0 _ZN16IGameStatTracker13CGameStatListI11SSurveyDataE8SendDataEP9KeyValues
void CGameStatList<SSurveyData>::SendData( KeyValues *pKV )
{
	// inlined UniqueStatID_t<SSurveyData>::Reset() at line 198
	{
		int i;  // line 190
		{
			KeyValues *pKVCopy;  // line 193
			// inlined SSurveyData::BuildGamestatDataTable() at line 194
		}
	}
}

// game/shared/portal/portal_gamestats_shared.h:185 @0x4dfa00 _ZN16IGameStatTracker13CGameStatListI12SConfigStatsE8SendDataEP9KeyValues
void CGameStatList<SConfigStats>::SendData( KeyValues *pKV )
{
	// inlined UniqueStatID_t<SConfigStats>::Reset() at line 198
	{
		int i;  // line 190
		{
			KeyValues *pKVCopy;  // line 193
			// inlined SConfigStats::BuildGamestatDataTable() at line 194
		}
	}
}

// game/shared/portal/portal_gamestats_shared.h:204 @0x4df810 _ZN16IGameStatTracker13CGameStatListI12SConfigStatsE5ClearEv
void CGameStatList<SConfigStats>::Clear()
{
	// inlined CUtlVector<SConfigStats*,CUtlMemory<SConfigStats*, int> >::PurgeAndDeleteElements() at line 206
}

// game/shared/portal/portal_gamestats_shared.h:204 @0x4e0360 _ZN16IGameStatTracker13CGameStatListI11SSurveyDataE5ClearEv
void CGameStatList<SSurveyData>::Clear()
{
	// inlined CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> >::PurgeAndDeleteElements() at line 206
}

// game/shared/portal/portal_gamestats_shared.h:212 @0x4dfc40 _ZN16IGameStatTracker13CGameStatListI11SSurveyDataE16PrintMemoryUsageEv
void CGameStatList<SSurveyData>::PrintMemoryUsage()
{
	unsigned int uMemoryUsed;  // line 218
}

// game/shared/portal/portal_gamestats_shared.h:212 @0x4dfcc0 _ZN16IGameStatTracker13CGameStatListI12SConfigStatsE16PrintMemoryUsageEv
void CGameStatList<SConfigStats>::PrintMemoryUsage()
{
	unsigned int uMemoryUsed;  // line 218
}

// game/shared/portal/portal_gamestats_shared.h:228 @0x4dfd40 _ZN16IGameStatTracker12GetStatTableI12SConfigStatsEEPNS_13CGameStatListIT_EEv
CGameStatList<SConfigStats> *IGameStatTracker::GetStatTable<SConfigStats>()
{
	// inlined CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> >::AddToTail() at line 234
	// inlined CGameStatList<SConfigStats>::CGameStatList() at line 233
}

// game/shared/portal/portal_gamestats_shared.h:228 @0x4e0050 _ZN16IGameStatTracker12GetStatTableI11SSurveyDataEEPNS_13CGameStatListIT_EEv
CGameStatList<SSurveyData> *IGameStatTracker::GetStatTable<SSurveyData>()
{
	// inlined CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> >::AddToTail() at line 234
	// inlined CGameStatList<SSurveyData>::CGameStatList() at line 233
}

// game/shared/portal/portal_gamestats_shared.h:242 sizeof=0xc (i386)
struct BaseStatData
{
public:
	BaseStatData();  // line 243
	bool m_bUseGlobalData; // +0x0  // line 249
	uint64 TimeSubmitted; // +0x4  // line 250
};
