// DWARF declaration skeleton for game/shared/IGameSystem.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/IGameSystem.h:30 sizeof=0x4 (i386)
struct IGameSystem
{
public:
	int (**_vptr$IGameSystem)(); // +0x0  // line 0
	virtual const char *Name();  // line 33
	virtual bool Init();  // line 37
	virtual void PostInit();  // line 38
	virtual void Shutdown();  // line 39
	virtual void LevelInitPreEntity();  // line 42
	virtual void LevelInitPostEntity();  // line 44
	virtual void LevelShutdownPreEntity();  // line 46
	virtual void LevelShutdownPostEntity();  // line 48
	virtual void OnSave();  // line 52
	virtual void OnRestore();  // line 55
	virtual void SafeRemoveIfDesired();  // line 58
	virtual bool IsPerFrame();  // line 60
	virtual ~IGameSystem();  // line 63
	static const char *MapName();  // line 66
	static void Add( IGameSystem * );  // line 71
	static void Remove( IGameSystem * );  // line 72
	static void RemoveAll();  // line 73
	static bool InitAllSystems();  // line 76
	static void PostInitAllSystems();  // line 77
	static void ShutdownAllSystems();  // line 78
	static void LevelInitPreEntityAllSystems( const char * );  // line 79
	static void LevelInitPostEntityAllSystems();  // line 80
	static void LevelShutdownPreEntityAllSystems();  // line 81
	static void LevelShutdownPostEntityAllSystems();  // line 82
	static void OnSaveAllSystems();  // line 84
	static void OnRestoreAllSystems();  // line 85
	static void SafeRemoveIfDesiredAllSystems();  // line 87
	static void PreRenderAllSystems();  // line 90
	static void UpdateAllSystems( float );  // line 91
	static void PostRenderAllSystems();  // line 92
};

// game/shared/IGameSystem.h:107 sizeof=0x4 (i386)
struct IGameSystemPerFrame : public IGameSystem
{
public:
	virtual ~IGameSystemPerFrame();  // line 110
	virtual void PreRender();  // line 114
	virtual void Update( float );  // line 117
	virtual void PostRender();  // line 120
};

// game/shared/IGameSystem.h:133 sizeof=0x4 (i386)
struct CBaseGameSystem : public IGameSystem
{
public:
	virtual const char *Name();  // line 136
	virtual bool Init();  // line 140
	virtual void PostInit();  // line 141
	virtual void Shutdown();  // line 142
	virtual void LevelInitPreEntity();  // line 145
	virtual void LevelInitPostEntity();  // line 146
	virtual void LevelShutdownPreEntity();  // line 147
	virtual void LevelShutdownPostEntity();  // line 148
	virtual void OnSave();  // line 150
	virtual void OnRestore();  // line 151
	virtual void SafeRemoveIfDesired();  // line 152
	virtual bool IsPerFrame();  // line 154
private:
	virtual void PreRender();  // line 161
	virtual void Update( float );  // line 164
	virtual void PostRender();  // line 167
};

// game/shared/IGameSystem.h:133 (declaration)
~CBaseGameSystem();

// game/shared/IGameSystem.h:180 sizeof=0x4 (i386)
struct CBaseGameSystemPerFrame : public IGameSystemPerFrame
{
public:
	virtual const char *Name();  // line 182
	virtual bool Init();  // line 186
	virtual void PostInit();  // line 187
	virtual void Shutdown();  // line 188
	virtual void LevelInitPreEntity();  // line 191
	virtual void LevelInitPostEntity();  // line 192
	virtual void LevelShutdownPreEntity();  // line 193
	virtual void LevelShutdownPostEntity();  // line 194
	virtual void OnSave();  // line 196
	virtual void OnRestore();  // line 197
	virtual void SafeRemoveIfDesired();  // line 198
	virtual bool IsPerFrame();  // line 200
	virtual void PreRender();  // line 204
	virtual void Update( float );  // line 207
	virtual void PostRender();  // line 210
};

// game/shared/IGameSystem.h:223 sizeof=0xc (i386)
struct CAutoGameSystem : public CBaseGameSystem
{
public:
	CAutoGameSystem( const char * );  // line 225
	CAutoGameSystem *m_pNext; // +0x4  // line 226
	virtual const char *Name();  // line 228
private:
	const char *m_pszName; // +0x8  // line 231
};

// game/shared/IGameSystem.h:223 (declaration)
~CAutoGameSystem();

// game/shared/IGameSystem.h:238 sizeof=0xc (i386)
struct CAutoGameSystemPerFrame : public CBaseGameSystemPerFrame
{
public:
	CAutoGameSystemPerFrame( const char * );  // line 240
	CAutoGameSystemPerFrame *m_pNext; // +0x4  // line 241
	virtual const char *Name();  // line 243
private:
	const char *m_pszName; // +0x8  // line 246
};
