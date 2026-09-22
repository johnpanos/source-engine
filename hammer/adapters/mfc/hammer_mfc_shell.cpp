//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Thin MFC UI sibling for the Hammer editor (RFC 0002,
//			hammer.adapters.mfc). This is the second of the two sibling shells the
//			architecture calls for, built new (NOT the legacy Hammer MFC shell,
//			which is entangled with the whole Source Windows engine). Like its GTK
//			sibling (hammer.adapters.gtk), it holds ONLY toolkit glue -- an MFC
//			CWinApp / CFrameWnd, a menu, and command handlers -- and delegates every
//			editor decision to the SAME reusable, engine-free libraries.
//
//			Editor core (no MFC): hammer::app::EditorDocument owns content, history
//			(undo/redo), and transactional save; hammer::formats serializes; the
//			document persists through hammer::ports::IFileStore, here the real
//			hammer::adapters::platform::DiskFileStore. No editor policy lives in this
//			file; each ON_COMMAND handler is a one- or two-line delegation, and the
//			view is refreshed from the document (the single authority). That is what
//			makes this shell "thin" and a true sibling of the GTK shell.
//
//			This target depends ONLY on MFC + the reusable libraries -- not on
//			tier0/vgui2/materialsystem -- so unlike the legacy shell it builds and
//			links with the real MSVC+MFC toolchain against the extracted libraries
//			alone. Build recipe + headless workflow oracle:
//			unittests/hammertest/adapters/build_mfc_shell.sh and test_mfc_workflow.cpp.
//
//=============================================================================//

#include <afxwin.h>

#include "hammer/app/editor_document.h"
#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/formats/keyvalues.h"

#include <string>

namespace
{
// Custom command IDs (avoid a resource script; the shell builds menus in code).
enum
{
	kCmdNew = 0x8001,
	kCmdSave,
	kCmdOpen,
	kCmdUndo,
	kCmdRedo,
	kCmdApplyEdit,
};

const char *const kSeedDocument = "world\n{\n\t\"classname\" \"worldspawn\"\n}\n";
const char *const kDocumentPath = "untitled.vmf";
} // namespace

//-----------------------------------------------------------------------------
// The main frame. Owns the reusable editor core (document + real file store) and
// a read-only edit control that renders it. Every command delegates to the core.
//-----------------------------------------------------------------------------
class CHammerFrame : public CFrameWnd
{
public:
	CHammerFrame()
	{
		std::string error;
		m_document.LoadFromText( kSeedDocument, error ); // core decides; UI just calls
	}

	afx_msg int OnCreate( LPCREATESTRUCT create )
	{
		if ( CFrameWnd::OnCreate( create ) == -1 )
		{
			return -1;
		}

		CMenu menu;
		menu.CreateMenu();
		CMenu file;
		file.CreatePopupMenu();
		file.AppendMenu( MF_STRING, kCmdNew, _T( "&New" ) );
		file.AppendMenu( MF_STRING, kCmdOpen, _T( "&Open" ) );
		file.AppendMenu( MF_STRING, kCmdSave, _T( "&Save" ) );
		CMenu edit;
		edit.CreatePopupMenu();
		edit.AppendMenu( MF_STRING, kCmdApplyEdit, _T( "Set &classname=func_detail" ) );
		edit.AppendMenu( MF_STRING, kCmdUndo, _T( "&Undo" ) );
		edit.AppendMenu( MF_STRING, kCmdRedo, _T( "&Redo" ) );
		menu.AppendMenu( MF_POPUP, reinterpret_cast<UINT_PTR>( file.Detach() ), _T( "&File" ) );
		menu.AppendMenu( MF_POPUP, reinterpret_cast<UINT_PTR>( edit.Detach() ), _T( "&Edit" ) );
		SetMenu( &menu );
		menu.Detach();

		m_view.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
					   CRect( 0, 0, 0, 0 ), this, 0x1000 );
		Refresh();
		return 0;
	}

	afx_msg void OnSize( UINT type, int cx, int cy )
	{
		CFrameWnd::OnSize( type, cx, cy );
		if ( m_view.GetSafeHwnd() != nullptr )
		{
			m_view.MoveWindow( 0, 0, cx, cy );
		}
	}

	// --- Command handlers: each delegates to the reusable editor core ---
	afx_msg void OnNew()
	{
		std::string error;
		m_document.LoadFromText( kSeedDocument, error );
		Refresh();
	}
	afx_msg void OnApplyEdit()
	{
		m_document.SetFirstBlockKey( "classname", "func_detail" );
		Refresh();
	}
	afx_msg void OnSave()
	{
		m_document.Save( m_store, kDocumentPath );
		Refresh();
	}
	afx_msg void OnOpen()
	{
		std::string error;
		m_document.Load( m_store, kDocumentPath, error );
		Refresh();
	}
	afx_msg void OnUndo()
	{
		m_document.Undo();
		Refresh();
	}
	afx_msg void OnRedo()
	{
		m_document.Redo();
		Refresh();
	}

private:
	// Render the document + status into the widgets. The only place the UI reads
	// the core, through public accessors.
	void Refresh()
	{
		const std::string text = hammer::formats::WriteKeyValues( m_document.Content() );
		std::string title = kDocumentPath;
		title += m_document.IsModified() ? " * - Hammer (MFC sibling)" : " - Hammer (MFC sibling)";
		::SetWindowTextA( GetSafeHwnd(), title.c_str() );
		if ( m_view.GetSafeHwnd() != nullptr )
		{
			::SetWindowTextA( m_view.GetSafeHwnd(), text.c_str() );
		}
	}

	hammer::app::EditorDocument m_document;
	hammer::adapters::platform::DiskFileStore m_store;
	CEdit m_view;

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP( CHammerFrame, CFrameWnd )
ON_WM_CREATE()
ON_WM_SIZE()
ON_COMMAND( kCmdNew, OnNew )
ON_COMMAND( kCmdApplyEdit, OnApplyEdit )
ON_COMMAND( kCmdSave, OnSave )
ON_COMMAND( kCmdOpen, OnOpen )
ON_COMMAND( kCmdUndo, OnUndo )
ON_COMMAND( kCmdRedo, OnRedo )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// The application object. Creates the frame; MFC provides WinMain.
//-----------------------------------------------------------------------------
class CHammerApp : public CWinApp
{
public:
	BOOL InitInstance() override
	{
		auto *frame = new CHammerFrame();
		m_pMainWnd = frame;
		frame->Create( nullptr, _T( "Hammer (MFC sibling)" ), WS_OVERLAPPEDWINDOW,
					   CRect( 0, 0, 720, 540 ) );
		frame->ShowWindow( SW_SHOW );
		frame->UpdateWindow();
		return TRUE;
	}
};

CHammerApp theApp;
