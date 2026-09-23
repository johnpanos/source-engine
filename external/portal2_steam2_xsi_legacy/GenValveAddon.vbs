'GenValveAddon.vbs
'askowron, July 2004 - 102970
'
'Script to generate the ValveSource addon.  You don't need to have any
'of the content installed yet in your user directory, however you need
'to have access to a clearcase view and you may need to change the g_ViewDrive content

option explicit

'Update this based on your configuration (e.g. it might be S: or P: instead)
const g_RealViewDrive = "S:"

'!(*&@*& Hack - XSI doesn't let you package things from the factory into an addon.
'But it considers everything in your source directory to be the factory.
'We hack around this with a temporary subst drive.
const g_ViewDrive = "X:"


const g_AddonName = "ValveSource"

dim g_ValveSrc : g_ValveSrc = g_ViewDrive & "\ThirdParty\ClientTools\Addons\ValveSource\"

dim g_Spdls, g_OtherFiles, g_Plugins

				'Src path						'Filename no Extension
g_Spdls = Array(		"Application\spdl\", 				"HelperBoneOp", _
				"Application\spdl\", 				"AxisInterpOp", _
				"",								"" )

				'Relative Path To g_ValveSrc	   		'File Name							'Plugin Name (set on load)
g_Plugins = Array( 	_
				"Application\Plugins\"	,			"SMDImport.dll",					"", _
				"Application\Plugins\"	,			"SMDExport2.dll",					"", _
				"Application\Plugins\"	,			"ValveSourceBackComp.vbs",			"", _
				"Application\Plugins\"	,			"ValveSourceBoneSymmetryMapTool.js",	"", _
				"Application\Plugins\"	,			"ValveSourceCharacterRigTools.js",		"", _
				"Application\Plugins\"	,			"ValveSourceCharacterRigTools.vbs",	"", _
				"Application\Plugins\"	,			"ValveSourceDebugTool.vbs",			"", _
				"Application\Plugins\"	,			"ValveSourceHelperBoneTool.js",		"", _
				"Application\Plugins\"	,			"ValveSourceHelperBoneTool.vbs",		"", _
				"Application\Plugins\"	,			"ValveSourceShapeTools.js",			"", _
				"Application\Plugins\"	,			"ValveSourceShapeTools2.vbs",		"", _
				"Application\Plugins\"	,			"ValveSourceTools.js",				"", _
				"Application\Plugins\"	,			"ValveSourceVMFTools.vbs",			"", _
				"Application\Plugins\"	,			"ValveSourceHelperBoneObject.js",	"", _
				"Application\Plugins\"	,			"VMFExport.dll",					"", _
				"Application\Plugins\"	,			"VMFImport.dll",					"", _
				"Application\Plugins\"	,			"WeightExport.dll",					"", _
				"Application\Plugins\"	,			"WeightImport.dll",					"", _
				""						,			"",									"" )	
				
'				"Application\Plugins\"	,			"SMDExport.dll",					"", _


				  'Source file						'Destination path
			  								'(Empty means root of addon)
g_OtherFiles = Array( 	_
				"Doc\ValveSourceHelperBoneTool.htm", 	"Doc", _
				"Doc\ValveSourceShapeTools.htm", 	"Doc", _
				"Doc\weighttool_help.htm", 	"Doc", _
				"Doc\characterrigstools_help.htm", 	"Doc", _
				"Doc\smdtools_help.htm", 	"Doc", _
				"Doc\vmftools_help.htm", 	"Doc", _
				"Doc\xsi.css",				 	"Doc", _
				"Doc\b1.gif",				 	"Doc", _
				"Doc\b3.gif",				 	"Doc", _
				"Doc\MakeDoorResult.gif",		"Doc", _
				"Doc\MakeDoor.gif",		"Doc", _
				"Doc\silogo.gif",				 	"Doc", _
				"Doc\avidlogo.gif",			 	"Doc", _
				"Doc\axisiinterptoolbar.jpg", 			"Doc", _
				"Doc\axisinterpolator_mainpage.jpg", 	"Doc", _
				"Doc\axisinterpolator_ppg3.jpg", 		"Doc", _
				"Doc\axisinterptool_help.htm", 		"Doc", _
				"Doc\axisinterptool_ppg1.jpg", 		"Doc", _
				"Doc\axisinterptool_ppg2.jpg", 		"Doc", _
				"netview_valvesource.htm", 			"", _
				"netview_valvesource.JPG", 			"", _					
				"netview_valvesource.txt",			"", _
				"Data\ConfigFiles\ValveBiped.vrd",		"Data\ConfigFiles",_
				"Application\toolbars\ValveSourceHelperBoneTool.xsitb",	"Application\toolbars", _
				"Application\toolbars\ValveSourceShapeTools.xsitb",		"Application\toolbars", _
				"Data\Models\Male\ValveBiped.emdl",	"Data\Models\Male\", _
				"Data\Models\Female\ValveBiped.emdl",	"Data\Models\Female\", _		
				"Data\Models\CharacterStudioGuide.emdl","Data\Models",_
				"Data\Models\ValveBiped.emdl",		"Data\Models",_
				"Data\Models\ValveCharacterGuide.emdl",	"Data\Models",_
				"Data\Models\ValveGuide.emdl",		"Data\Models",_
				"Data\Scripts\BipedFromGuide_03_NullShadow.js",	"Data\Scripts",_
				"Data\Scripts\MakeControls_01.js",		"Data\Scripts",_
				"Data\Scripts\MatchShadowNulls2OriginalSkeleton_02b.js",	"Data\Scripts",_
				"Data\Scripts\Organize01.js",			"Data\Scripts",_
				"Data\Scripts\Rename_ShadowNulls_01.js",		"Data\Scripts",_
				"Application\bin\nt-x86\HelperBoneOp.dll",		"Application\bin\nt-x86",_
				"Application\bin\nt-x86\AxisInterpOp.dll",		"Application\bin\nt-x86",_
				""				,			"" )	

const g_slash = "\"

dim g_oFSO
set g_oFSO = CreateObject( "Scripting.FileSystemObject" )


system( "subst " & g_ViewDrive & " " & g_RealViewDrive & "\" )
InstallAddonComponents
VerifyExpectedContent
RepackageAddon
RemoveLocalComponents
system( "subst " & g_ViewDrive & "/d" )


function InstallAddonComponents
	logmessage "*********** Installing Valve Addon Components **************************" 

	'Uninstall the addon if is it already installed
	on error resume next
	UnInstallAddon Application.InstallationPath( siUserAddonPath ) & "\InstalledAddons\" & g_AddonName & ".xsiaddon"
	on error goto 0


	'Most items need to be installed locally before you can package them
	'into an add-on.  This requirement causes most of the complexity of this
	'script
		
	'Regenerate signature file (for EXPERIENCE version)
	
	dim strCmd : strCmd = g_ViewDrive & " & " & g_ViewDrive & "\Utils\Scripts\expsigner.pl -in " & g_ValveSrc & "exp.filelist -out " & g_ValveSrc & "Application\exp.sgn"	
	system( strCmd )

	dim i

	for  i = 0 to ubound( g_Plugins, 1 ) 
		if ( g_Plugins(i+1) <> "" ) then
			Application.UnloadPlugin g_ValveSrc & g_Plugins(i) & g_Plugins(i+1), true
			
			dim oPlugin
			set oPlugin = Application.LoadPlugin( g_ValveSrc & g_Plugins(i) & g_Plugins(i+1) )
			
			'Remember the actual "name" of the plug-in, because that is what is needed 
			'when building the addon
			g_Plugins(i+2) = oPlugin.Name
			logmessage "Installed plugin: " & oPlugin.Name
		end if
		
		i = i + 2
	next 

	for  i = 0 to ubound( g_Spdls, 1 ) 
		if ( g_Spdls(i+1)<>"") then
			dim spdlname
			spdlname = g_Spdls( i + 1 )
				
			dim spdldest
			spdldest = Application.InstallationPath( siUserPath ) & g_slash _
				 & "Application" & g_slash & "spdl" & g_slash & spdlname & ".spdl"
				
			on error resume next
			g_oFSO.DeleteFile spdldest, true
			on error goto 0
				
			g_oFSO.CopyFile g_ValveSrc & g_Spdls(i) & spdlname & ".spdl", spdldest, true	
	
			'Only works on windows but the read-only attribute in clear case shouldn't be transfered to the 
			'temp copy in the user directory
			system( "attrib -r " & spdldest )
	
			XSIUtils.RegisterSpdl spdldest, true
		end if		
		i = i + 1
	next 
	
end function

function RepackageAddon

	logmessage "*********** Repackaging Valve Addon **************************" 

	dim oAddon
	set oAddon = Application.CreateAddon 

	oAddon.SubDirectory = g_AddonName
	oAddOn.DefaultInstallationPath = siFactoryPath
	
	'REVIEW: TODO other addon attributes need to be exposed in object model, e.g. for description,
	'addon version etc

	dim i

	for  i = 0 to ubound( g_Plugins, 1 ) 
		if (g_Plugins(i+2) <> "") then
			logmessage "Packaging plugin : " & g_Plugins(i+2)
			oAddon.AddItem  siPluginAddonItemType, g_Plugins(i+2)
		end if	
		i = i + 2
	next 

	for  i = 0 to ubound( g_OtherFiles, 1 ) 
		if (g_OtherFiles(i) <> "") then
			logmessage "Packaging otherfile : " & g_OtherFiles(i)	
			oAddon.AddOtherItem g_ValveSrc & g_OtherFiles(i), g_OtherFiles( i+1 )
		end if	
		i = i + 1
	next
		
	dim generatedAddonPath 
	generatedAddonPath = g_ValveSrc & g_AddonName & ".xsiaddon"

	'This can fail if you didn't checkout the addon
	on error resume next
	oAddon.Save generatedAddonPath
	if err <> 0 then
		Logmessage "Failed to save " & generatedAddonPath & "... Make sure the file is checked out"
	else
		Logmessage "Addon file been saved " & generatedAddonPath
	end if
	on error goto 0

	RepackageAddon = generatedAddonPath
end function

function RemoveLocalComponents

	'
	' Remove plug-in items so they don't conflict with addon contents
	'
	
	dim i

	for  i = 0 to ubound( g_Plugins, 1 ) 
		if (g_Plugins(i)<>"") then
			'logmessage "Unloading " & g_ValveSrc & g_Plugins(i) & g_Plugins(i+1)	
			Application.UnloadPlugin g_ValveSrc & g_Plugins(i) & g_Plugins(i+1), true		
		end if	
		i = i + 2
	next 

	for  i = 0 to ubound( g_Spdls, 1 ) 
		if (g_Spdls(i+1)<>"") then
			dim spdlname : spdlname = g_Spdls( i + 1 )
				
			dim spdldest
			spdldest = Application.InstallationPath( siUserPath ) & g_slash _
				 & "Application" & g_slash & "spdl" & g_slash & spdlname & ".spdl"
	
			XSIUtils.UnRegisterSPDL spdldest, true, true
		end if		
		i = i + 1
	next
	
end function

function VerifyExpectedContent
	'Verification of contents of the plugins

	if ( typename( Application.Commands( "ValveCopyShapeKey" ) ) <> "Command" ) then
		logmessage "Command ValveCopyShapeKey not found even after plugin loaded", siError
	end if

	if ( typename( Application.Commands( "ApplyHelperBoneChanges" ) ) <> "Command" ) then
		logmessage "Command ApplyHelperBoneChangesnot found even after plugin loaded", siError
	end if

	

end function


