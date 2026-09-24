// DWARF declaration skeleton for game/shared/sharedvar.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/sharedvar.h:122 (declaration)
void SharedVar_SaveUtlVector<BlobDataPerUpdate_t>( ISave *pSave, CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > *pValue, int iCount );

// game/shared/sharedvar.h:131 (declaration)
void SharedVar_RestoreUtlVector<BlobDataPerUpdate_t>( IRestore *pRestore, CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > *pValue );

// game/shared/sharedvar.h:147 (declaration)
void SharedVar_MakeEmptyUtlVector<BlobDataPerUpdate_t>( CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > *pValue, int iCount );

// game/shared/sharedvar.h:156 sizeof=0x4 (i386)
struct ISharedVarBase
{
public:
	int (**_vptr$ISharedVarBase)(); // +0x0  // line 0
private:
	virtual void _Save( ISave * );  // line 158
	virtual void _Restore( IRestore * );  // line 159
	virtual bool _IsEmpty();  // line 160
	virtual void _MakeEmpty();  // line 161
};

// game/shared/sharedvar.h:170 sizeof=0xc (i386)
struct CSharedVarBase<CUtlVector<BlobDataPerUpdate_t, CUtlMemory<BlobDataPerUpdate_t, int> >,CPaintStream::SharedVar_m_sharedBlobData> : public ISharedVarBase
{
public:
	void CSharedVarBase();  // line 914
	~CSharedVarBase();  // line 920
protected:
	virtual void Shared_CUtlVector_Vars_Must_Use_CSharedUtlVector();  // line 931
public:
	ISPSharedMemory *m_pSharedMemory; // +0x4  // line 934
	CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > *m_pValue; // +0x8  // line 935
};

// game/shared/sharedvar.h:942 sizeof=0xc (i386)
struct CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData> : public CSharedVarBase<CUtlVector<BlobDataPerUpdate_t, CUtlMemory<BlobDataPerUpdate_t, int> >,CPaintStream::SharedVar_m_sharedBlobData>
{
public:
	void Initialize( char *, int );  // line 944
	CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > &operator CUtlVector<BlobDataPerUpdate_t, CUtlMemory<BlobDataPerUpdate_t, int> >&();  // line 955
	CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > &GetForModify();  // line 960
	CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > *operator->();  // line 965
	BlobDataPerUpdate_t &operator[]( int );  // line 970
	BlobDataPerUpdate_t &GetForModify( int );  // line 975
	void Set( int, const BlobDataPerUpdate_t & );  // line 981
	const CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > &operator const CUtlVector<BlobDataPerUpdate_t, CUtlMemory<BlobDataPerUpdate_t, int> >&() const;  // line 988
	const CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > &Get() const;  // line 993
	const CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> > *operator->() const;  // line 998
	const BlobDataPerUpdate_t &operator[]( int ) const;  // line 1003
	const BlobDataPerUpdate_t &Get( int ) const;  // line 1008
	const int Count() const;  // line 1014
protected:
	virtual void Shared_CUtlVector_Vars_Must_Use_CSharedUtlVector();  // line 1020
private:
	virtual void _Save( ISave * );  // line 1023
	virtual void _Restore( IRestore * );  // line 1028
	virtual bool _IsEmpty();  // line 1033
	virtual void _MakeEmpty();  // line 1038
};

// game/shared/sharedvar.h:942 (declaration)
void CSharedUtlVectorBase();

// game/shared/sharedvar.h:942 (declaration)
~CSharedUtlVectorBase();

// game/shared/sharedvar.h:1020 @0x73d2b0 _ZN20CSharedUtlVectorBaseI19BlobDataPerUpdate_tN12CPaintStream26SharedVar_m_sharedBlobDataEE48Shared_CUtlVector_Vars_Must_Use_CSharedUtlVectorEv
void CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::Shared_CUtlVector_Vars_Must_Use_CSharedUtlVector()
{
}

// game/shared/sharedvar.h:1023 @0x73d2c0 _ZN20CSharedUtlVectorBaseI19BlobDataPerUpdate_tN12CPaintStream26SharedVar_m_sharedBlobDataEE5_SaveEP5ISave
void CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::_Save( ISave *pSave )
{
	// inlined SharedVar_SaveUtlVector<BlobDataPerUpdate_t>() at line 1025
}

// game/shared/sharedvar.h:1028 @0x73dce0 _ZN20CSharedUtlVectorBaseI19BlobDataPerUpdate_tN12CPaintStream26SharedVar_m_sharedBlobDataEE8_RestoreEP8IRestore
void CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::_Restore( IRestore *pRestore )
{
	// inlined SharedVar_RestoreUtlVector<BlobDataPerUpdate_t>() at line 1030
}

// game/shared/sharedvar.h:1033 @0x73d320 _ZN20CSharedUtlVectorBaseI19BlobDataPerUpdate_tN12CPaintStream26SharedVar_m_sharedBlobDataEE8_IsEmptyEv
bool CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::_IsEmpty()
{
}

// game/shared/sharedvar.h:1038 @0x73dcc0 _ZN20CSharedUtlVectorBaseI19BlobDataPerUpdate_tN12CPaintStream26SharedVar_m_sharedBlobDataEE10_MakeEmptyEv
void CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::_MakeEmpty()
{
	// inlined SharedVar_MakeEmptyUtlVector<BlobDataPerUpdate_t>() at line 1040
}

// game/shared/sharedvar.h:1125 sizeof=0x4 (i386)
struct CSharedVarSaveDataOps : public CDefSaveRestoreOps
{
private:
	virtual void Save( const SaveRestoreFieldInfo_t &, ISave * );  // line 1127
	virtual void Restore( const SaveRestoreFieldInfo_t &, IRestore * );  // line 1134
	virtual bool IsEmpty( const SaveRestoreFieldInfo_t & );  // line 1141
	virtual void MakeEmpty( const SaveRestoreFieldInfo_t & );  // line 1147
};

// game/shared/sharedvar.h:1125 (declaration)
void CSharedVarSaveDataOps();

// game/shared/sharedvar.h:1154
static CSharedVarSaveDataOps g_SharedVarSaveDataOps;
