// DWARF declaration skeleton for game/shared/portal2/paint_saverestore.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x15ea0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CPaintSaveRestoreBlockHandler::CPaintSaveRestoreBlockHandler() at line 211
}

// game/shared/portal2/paint_saverestore.cpp:25 sizeof=0x4 (i386)
struct CPaintSaveRestoreBlockHandler : public CDefSaveRestoreBlockHandler
{
private:
	virtual const char *GetBlockName();  // line 26
	virtual void PreSave( CSaveRestoreData * );  // line 28
	virtual void Save( ISave * );  // line 30
	virtual void WriteSaveHeaders( ISave * );  // line 110
	virtual void PostSave();  // line 112
	virtual void PreRestore();  // line 114
	virtual void ReadRestoreHeaders( IRestore * );  // line 121
	virtual void Restore( IRestore *, bool );  // line 123
	virtual void PostRestore();  // line 206
};

// game/shared/portal2/paint_saverestore.cpp:25 (declaration)
void CPaintSaveRestoreBlockHandler();

// game/shared/portal2/paint_saverestore.cpp:26 @0x1b46b0 _ZN29CPaintSaveRestoreBlockHandler12GetBlockNameEv
const char *CPaintSaveRestoreBlockHandler::GetBlockName()
{
}

// game/shared/portal2/paint_saverestore.cpp:28 @0x1b46c0 _ZN29CPaintSaveRestoreBlockHandler7PreSaveEP16CSaveRestoreData
void CPaintSaveRestoreBlockHandler::PreSave( CSaveRestoreData * )
{
}

// game/shared/portal2/paint_saverestore.cpp:30 @0x1b46d0 _ZN29CPaintSaveRestoreBlockHandler4SaveEP5ISave
void CPaintSaveRestoreBlockHandler::Save( ISave *pSave )
{
}

// game/shared/portal2/paint_saverestore.cpp:110 @0x1b46f0 _ZN29CPaintSaveRestoreBlockHandler16WriteSaveHeadersEP5ISave
void CPaintSaveRestoreBlockHandler::WriteSaveHeaders( ISave * )
{
}

// game/shared/portal2/paint_saverestore.cpp:112 @0x1b4700 _ZN29CPaintSaveRestoreBlockHandler8PostSaveEv
void CPaintSaveRestoreBlockHandler::PostSave()
{
}

// game/shared/portal2/paint_saverestore.cpp:114 @0x1b4710 _ZN29CPaintSaveRestoreBlockHandler10PreRestoreEv
void CPaintSaveRestoreBlockHandler::PreRestore()
{
}

// game/shared/portal2/paint_saverestore.cpp:121 @0x1b4720 _ZN29CPaintSaveRestoreBlockHandler18ReadRestoreHeadersEP8IRestore
void CPaintSaveRestoreBlockHandler::ReadRestoreHeaders( IRestore * )
{
}

// game/shared/portal2/paint_saverestore.cpp:123 @0x1b4730 _ZN29CPaintSaveRestoreBlockHandler7RestoreEP8IRestoreb
void CPaintSaveRestoreBlockHandler::Restore( IRestore *pRestore, bool fCreatePlayers )
{
}

// game/shared/portal2/paint_saverestore.cpp:206 @0x1b4750 _ZN29CPaintSaveRestoreBlockHandler11PostRestoreEv
void CPaintSaveRestoreBlockHandler::PostRestore()
{
}

// game/shared/portal2/paint_saverestore.cpp:211
CPaintSaveRestoreBlockHandler g_PaintSaveRestoreBlockHandler;

// game/shared/portal2/paint_saverestore.cpp:213 @0x1b46a0 _Z31GetPaintSaveRestoreBlockHandlerv
ISaveRestoreBlockHandler *GetPaintSaveRestoreBlockHandler()
{
}
