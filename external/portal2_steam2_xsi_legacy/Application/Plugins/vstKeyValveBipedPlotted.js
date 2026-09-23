function XSILoadPlugin( in_reg )
{
	in_reg.Author = "wade";
	in_reg.Name = "vstKeyValveBipedPlotted";
	in_reg.Email = "";
	in_reg.URL = "";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("vstKeyValveBipedPlotted","vstKeyValveBipedPlotted");
	//RegistrationInsertionPoint - do not remove this line

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}

function vstKeyValveBipedPlotted_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	return true;
}

function vstKeyValveBipedPlotted_Execute(  )
{

	Application.LogMessage("vstKeyValveBipedPlotted_Execute called");
	
	var plotted = new ActiveXObject("XSI.Collection");
	plotted.Add( "ValveBiped.Plotted" );
	var members = new Enumerator( plotted.item(0).Members );
	for ( ; !members.atEnd(); members.moveNext() )
	{
//		logmessage( "\t" + members.item().Name );
		var keyThis = 	members.item().Name;
		SetKeyUsingMarkingSet("ValveBiped." + keyThis);
	}
	
 
	return true;
}



