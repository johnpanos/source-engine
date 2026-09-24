// DWARF declaration skeleton for game/server/vscript_server.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/vscript_server.h:39 sizeof=0x4 (i386)
struct CScriptKeyValues
{
public:
	CScriptKeyValues( KeyValues * );  // line 41
	~CScriptKeyValues();  // line 42
	HSCRIPT ScriptFindKey( const char * );  // line 44
	HSCRIPT ScriptGetFirstSubKey();  // line 45
	HSCRIPT ScriptGetNextKey();  // line 46
	int ScriptGetKeyValueInt( const char * );  // line 47
	float ScriptGetKeyValueFloat( const char * );  // line 48
	const char *ScriptGetKeyValueString( const char * );  // line 49
	bool ScriptIsKeyValueEmpty( const char * );  // line 50
	bool ScriptGetKeyValueBool( const char * );  // line 51
	void ScriptReleaseKeyValues();  // line 52
	KeyValues *m_pKeyValues; // +0x0  // line 54
};
