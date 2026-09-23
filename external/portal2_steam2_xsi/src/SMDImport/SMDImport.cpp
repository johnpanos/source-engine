// MapImport.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Commdlg.h"

#include <comdef.h>




#include "SMDImporter.h"
#include "QCParser.h"

void	XSILogMessage ( wchar_t *in_szMessage, XSI::siSeverityType in_eSeverity );

char ___gTexturePathOverride[MAX_PATH];
bool g_iImportMaterials = false;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{


	return TRUE;
}

HRESULT WINAPI XSIOnInit
( 
	HINSTANCE in_hInstModule
)
{


	return S_OK;
}

HRESULT WINAPI XSIOnTerminate()
{
	return S_OK;
}




namespace XSI
{
#ifdef unix
extern "C"
#endif

void SMDImport();

XSI::CStatus XSILoadPlugin( XSI::PluginRegistrar& in_reg )
{
	using namespace XSI;
	in_reg.PutAuthor( L"Softimage Corp" );
	in_reg.PutName( L"SMD Import Plug-in" );	in_reg.PutVersion( 1, 0 );

	// register the commands plugin item.
	in_reg.RegisterCommand( L"SMDImport", L"SMDImport" );
	// register property
	in_reg.RegisterProperty( L"SMDImportProperty" );	//
	// register the menu for the import 
//	in_reg.RegisterMenu(XSI::siMenuMainFileImportID, L"ImportSMD", false, false);
//	in_reg.RegisterMenu((XSI::siMenuAnchorPoints)1014, L"ImportSMD", false, false);

	return XSI::CStatus::OK;	
}

#ifdef unix
extern "C"
#endif
XSI::CStatus ImportSMD_Init( XSI::CRef& in_ref )
{
	Context ctxt = in_ref;
	Menu menu = ctxt.GetSource();

	CStatus st;
	MenuItem item;
	menu.AddCallbackItem(L"Import SMD...", L"OnSMDImportMenu", item);

	return CStatus::OK;	
}


#ifdef unix
extern "C"
#endif
XSI::CStatus OnSMDImportMenu( XSI::CRef& in_ref )
{
	SMDImport();
	return CStatus::OK;
}

#ifdef unix
extern "C"
#endif
XSI::CStatus XSIUnloadPlugin( const XSI::PluginRegistrar& in_reg )
{
	Application app;
#ifdef _DEBUG
	app.LogMessage( in_reg.GetName() + L" has been unloaded.");
#endif
	return XSI::CStatus::OK;
}

#ifdef unix
extern "C"
#endif
CStatus SMDImport_Init( const XSI::CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	cmd.EnableReturnValue ( true );


	ArgumentArray args = cmd.GetArguments();

	args.Add( L"arg0", (long)0 );
	
	return XSI::CStatus::OK;
}

void SMDImport();

#ifdef unix
extern "C"
#endif
CStatus SMDImport_Execute( XSI::CRef& in_context )
{
	SMDImport();
	return CStatus::OK;
}

#ifdef unix
extern "C"
#endif
CStatus SMDImportProperty_Define( const CRef & in_Ctx )
{
	Application app ;
	CustomProperty prop = Context(in_Ctx).GetSource() ;
	Parameter param ;

	// Default capabilities for most of these parameters
	int caps = siPersistable  ;
	CValue dft ;	// Used for arguments we don't want to set

	prop.AddParameter(	L"Filename",CValue::siString, caps, 
						L"Filename", L"", 
						dft, param ) ;
	
	prop.AddParameter(	L"TexturePath",CValue::siString, caps, 
						L"Texture path", L"", 
						dft, param ) ;	

	dft = true;

	prop.AddParameter(	L"UseMaterials",CValue::siBool, caps, 
						L"Import Materials", L"", 
						dft, param ) ;	

	return CStatus::OK;	
}

#ifdef unix
extern "C"
#endif
CStatus SMDImportProperty_DefineLayout( const CRef & in_Ctx )
{
	PPGLayout oLayout = Context( in_Ctx ).GetSource() ;
	
	PPGItem item ;
	
	oLayout.Clear() ;
	
	
	item = oLayout.AddItem( L"SMDImportProperty" ) ;
	
	oLayout.AddGroup(L"File") ;
	
	oLayout.AddRow() ;
	item = oLayout.AddItem( L"Filename",L"Import File",siControlFilePath ) ;
	item.PutAttribute( siUIFileMustExist, true ) ;
	item.PutAttribute( siUIOpenFile, true ) ;
	item.PutAttribute( siUIFileFilter, L"Valve SMD Files (*.smd)|*.smd|Vertex Animation Files (*.vta)|*.vta|Valve .QC file (*.qc)|*.qc|All Files (*.*)|*.*||" ) ;
	oLayout.EndRow() ;

	oLayout.AddRow() ;
	item = oLayout.AddItem( L"TexturePath",L"Texture Path",siControlFolder ) ;
	item.PutAttribute( siUIInitialDir, L"user" ) ;
	oLayout.EndRow() ;

	oLayout.EndGroup() ;
	
	oLayout.AddGroup( L"Options" ) ;
	oLayout.AddRow();
	oLayout.AddRow();
	item = oLayout.AddItem( L"UseMaterials" ) ;
	oLayout.EndRow();
	oLayout.EndGroup() ;

	return CStatus::OK;	
}

void SMDImport()
{

	Application app;

	CStatus	st;

	Property prop;
	prop = app.GetActiveSceneRoot().GetProperties().GetItem( L"SMDImportProperty" );
	if (!prop.IsValid())
		prop = app.GetActiveSceneRoot().AddProperty( L"SMDImportProperty" ) ;

	CValueArray args(5);
	args[0] = prop;
	args[1] = L"";
	args[2] = L"SMDImportProperty";
	args[3] = (long)4;
	args[4] = true;
	CValue ret;
	st = app.ExecuteCommand(L"InspectObj",args,ret);
	if ( CStatus::OK == st ) {
		//
		// FileName
		//
		char l_szFilename[MAX_PATH];
		memset ( l_szFilename,0,MAX_PATH );

		Parameter parm = prop.GetParameters().GetItem(L"Filename" );
		CString	str = parm.GetValue();
		const wchar_t	* p = str.GetWideString();
		wcstombs( l_szFilename, p, wcslen (p));

		parm = prop.GetParameters().GetItem(L"TexturePath" );
		str = parm.GetValue();
		p = str.GetWideString();
		wcstombs( ___gTexturePathOverride, p, wcslen (p));

		

		//
		// Bools
		//
		parm = prop.GetParameters().GetItem(L"UseMaterials");
		g_iImportMaterials = (bool) parm.GetValue();

		if (l_szFilename && strlen( l_szFilename ) > 0)
		{
		
			//
			// SMD or QC?
			//

			char ext[256];
			_splitpath ( l_szFilename, NULL, NULL, NULL, ext );

			if ( strstr ( ext, "smd"))
			{
				SMDImporter importer;
				importer.Import ( l_szFilename );
			} 

			if ( strstr ( ext, "vta"))
			{
				XSILogMessage ( L"didn't have time to implement support for VTAs yet... sorry ", XSI::siErrorMsg );
				//SMDImporter importer;
				//importer.Import ( l_szFilename );

			} 

			if ( strstr ( ext, "qc"))
			{
				QCParser parser;
				parser.Import ( l_szFilename );

				//
				// import include
				//

				for (int i=0;i<parser.m_includelist.GetUsed();i++)
				{
					char *pSzString = new char [ parser.m_includelist[i].Length() + 1 ];
					W2AHelper ( pSzString, parser.m_includelist[i].GetWideString() );
					
					QCParser parser2;
					parser2.Import ( pSzString );
			
					delete [] pSzString;
				}

			} 



		}
		else
		{
			XSILogMessage ( L"Error - Invalid file specified", XSI::siErrorMsg );

		}

	}
}
} // using namespace XSI


void	XSILogMessage ( wchar_t *in_szMessage, XSI::siSeverityType in_eSeverity )
{
	XSI::Application	app;


	app.LogMessage( in_szMessage, in_eSeverity ) ;


}
