/******************************************************************************************
*******************************************************************************************
	Organization.  Groups, display, marking sets, cleanup
*******************************************************************************************
******************************************************************************************/

//Display Properties for Controls
	
	// 	Left 
var LeftCnlsArray = new Array("Biped.HandControlPosL", "Biped.HandControlRotL", "Biped.LeftFoot", "Biped.ClavicleControlL", "Biped.LeftRoll", "Biped.LlegUpV", "Biped.LArmUpV");
for( i=0; i<LeftCnlsArray.length; i++){
	AddProp("Display Property", LeftCnlsArray[i], "", "", null);
	SetValue(LeftCnlsArray[i] +".display.staticsel", 0, null);
	SetValue(LeftCnlsArray[i] +".display.intsel", 0, null);
	SetValue(LeftCnlsArray[i] +".display.playbacksel", 0, null);
	SetValue(LeftCnlsArray[i] +".display.staticunselnear", 0, null);
	SetValue(LeftCnlsArray[i] +".display.intunselnear", 0, null);
	SetValue(LeftCnlsArray[i] +".display.playbackunselnear", 0, null);
	SetValue(LeftCnlsArray[i] +".display.staticunselfar", 0, null);
	SetValue(LeftCnlsArray[i] +".display.intunselfar", 0, null);
	SetValue(LeftCnlsArray[i] +".display.playbackunselfar", 0, null);
	SetValue(LeftCnlsArray[i] +".display.wirecol", 367, null);
}

	//Right
var RightCnlsArray = new Array("Biped.HandControlPosR", "Biped.HandControlRotR", "Biped.RightFoot", "Biped.ClavicleControlR", "Biped.RightRoll", "Biped.RlegUpV", "Biped.RArmUpV");
for( i=0; i<RightCnlsArray.length; i++){
	AddProp("Display Property", RightCnlsArray[i], "", "", null);
	SetValue(RightCnlsArray[i] +".display.staticsel", 0, null);
	SetValue(RightCnlsArray[i] +".display.intsel", 0, null);
	SetValue(RightCnlsArray[i] +".display.playbacksel", 0, null);
	SetValue(RightCnlsArray[i] +".display.staticunselnear", 0, null);
	SetValue(RightCnlsArray[i] +".display.intunselnear", 0, null);
	SetValue(RightCnlsArray[i] +".display.playbackunselnear", 0, null);
	SetValue(RightCnlsArray[i] +".display.staticunselfar", 0, null);
	SetValue(RightCnlsArray[i] +".display.intunselfar", 0, null);
	SetValue(RightCnlsArray[i] +".display.playbackunselfar", 0, null);
	SetValue(RightCnlsArray[i] +".display.wirecol", 13, null);
}

	//Spine, Neck, Head
var SpineCnlsArray = new Array("Biped.SpineControl", "Biped.SpineControl1", "Biped.SpineControl2", "Biped.SpineControl3", "Biped.NeckControl", "Biped.HeadControl");
for( i=0; i<SpineCnlsArray.length; i++){
	AddProp("Display Property", SpineCnlsArray[i], "", "", null);
	SetValue(SpineCnlsArray[i] +".display.staticsel", 0, null);
	SetValue(SpineCnlsArray[i] +".display.intsel", 0, null);
	SetValue(SpineCnlsArray[i] +".display.playbacksel", 0, null);
	SetValue(SpineCnlsArray[i] +".display.staticunselnear", 0, null);
	SetValue(SpineCnlsArray[i] +".display.intunselnear", 0, null);
	SetValue(SpineCnlsArray[i] +".display.playbackunselnear", 0, null);
	SetValue(SpineCnlsArray[i] +".display.staticunselfar", 0, null);
	SetValue(SpineCnlsArray[i] +".display.intunselfar", 0, null);
	SetValue(SpineCnlsArray[i] +".display.playbackunselfar", 0, null);
	SetValue(SpineCnlsArray[i] +".display.wirecol", 433, null);
}

//Clean up explorer, Create/manage groups
	
	//Move Shadow skeleton into Biped model, delete Shadow model
CopyPaste("Shadow.Bip01_Pelvis", null, "Biped", 1);
DeleteObj("B:Shadow");
	
	//move UpVParents under GlobalSRT for cleanliness' sake
CopyPaste("Biped.LArmUpV_Parent", null, "Biped.GlobalSRT", 1);
CopyPaste("Biped.LLegUpV_Parent", null, "Biped.GlobalSRT", 1);
CopyPaste("Biped.RArmUpV_Parent", null, "Biped.GlobalSRT", 1);
CopyPaste("Biped.RLegUpV_Parent", null, "Biped.GlobalSRT", 1);

	//Delete old Envelope_group, create new one
DeleteObj("Biped.envelope_group");
SelectObj("Biped.Bip01_Pelvis", "BRANCH", null);
SelectChildNodes(null, null, null);
CreateGroup("Envelope", null, null);
CopyPaste("Envelope", null, "Biped", 1);

	//add more to Hidden group
SIAddToGroup("Biped.hidden", "Biped.LArmUpV_Parent,Biped.HandControlRotParentL,Biped.LLegUpV_Parent,Biped.RArmUpV_Parent,Biped.HandControlRotParentR", null);
SIAddToGroup("Biped.hidden", "Biped.LArmEff,Biped.RArmEff", null);

	//Create Plotted group (all anim controls)
SelectObj("Biped.RThumb1,Biped.RThumb2,Biped.RThumb3,Biped.HandControlPosR,Biped.HandControlRotR", null, true);
AddToSelection("Biped.RPinky1,Biped.RPinky2,Biped.RPinky3,Biped.RRing1,Biped.RRing2,Biped.RMiddle1,Biped.RIndex1,Biped.HandControlRotR", null, true);
AddToSelection("Biped.RRing3,Biped.RMiddle2,Biped.RMiddle3,Biped.RIndex2,Biped.RIndex3,Biped.HandControlRotR", null, true);
AddToSelection("Biped.UpperBody", null, true);
AddToSelection("Biped.PelvisControl", null, true);
AddToSelection("Biped.LeftFoot", null, true);
AddToSelection("Biped.LeftRoll", null, true);
AddToSelection("Biped.LeftTip,Biped.LToeOffset2", null, true);
AddToSelection("Biped.RightTip", null, true);
AddToSelection("Biped.RToeOffset2", null, true);
AddToSelection("Biped.RightFoot", null, true);
AddToSelection("Biped.RightRoll", null, true);
AddToSelection("Biped.LToeOffset1", null, true);
AddToSelection("Biped.RToeOffset1", null, true);
AddToSelection("Biped.LThumb1,Biped.LThumb2,Biped.LThumb3,Biped.HandControlPosL,Biped.HandControlRotL", null, true);
AddToSelection("Biped.LPinky1,Biped.LPinky2,Biped.LPinky3,Biped.LRing1,Biped.LRing2,Biped.LRing3,Biped.LMiddle1,Biped.LMiddle2,Biped.LMiddle3,Biped.LIndex1,Biped.LIndex2,Biped.LIndex3,Biped.HandControlRotL", null, true);
AddToSelection("Biped.SpineControl,Biped.SpineControl1,Biped.SpineControl2,Biped.LlegUpV,Biped.RlegUpV", null, true);
AddToSelection("Biped.SpineControl3,Biped.NeckControl,Biped.HeadControl,Biped.ClavicleControlL,Biped.LArmUpV", null, true);
AddToSelection("Biped.ClavicleControlR,Biped.RArmUpV", null, true);
CreateGroup("Plotted", null, null);
CopyPaste("Plotted", null, "Biped", 1);

//Define Marking Sets
	
	//Pos only controls
SelectObj("Biped.HandControlPosL", null, true);	
SetMarking("kine.local.pos.posx");
AddToMarking("kine.local.pos.posy");
AddToMarking("kine.local.pos.posz");
CreateMarkingSet("Biped.RArmUpV,Biped.LArmUpV,Biped.LlegUpV,Biped.RlegUpV,Biped.HandControlPosL,Biped.HandControlPosR", false);	
	
	//Rot only controls
	//	not redefining fingers now (they have marking sets), i'm lazy!
SelectObj("Biped.HandControlRotL", null, true);	
SetMarking("kine.local.ori.euler");
CreateMarkingSet("Biped.SpineControl,Biped.SpineControl1,Biped.SpineControl2,Biped.SpineControl3,Biped.NeckControl,Biped.HeadControl,Biped.ClavicleControlR,Biped.ClavicleControlL,Biped.HandControlRotL,Biped.HandControlRotR,Biped.RightRoll,Biped.LeftRoll,Biped.RToeOffset1,Biped.RToeOffset2,Biped.LToeOffset1,Biped.LToeOffset2,Biped.PelvisControl", false);	
	
	//Pos/Rot controls	
SelectObj("Biped.UpperBody", null, true);	
SetMarking("kine.local.pos.posx");
AddToMarking("kine.local.pos.posy");
AddToMarking("kine.local.pos.posz");
AddToMarking("kine.local.ori.euler.rotx");
AddToMarking("kine.local.ori.euler.roty");
AddToMarking("kine.local.ori.euler.rotz");
CreateMarkingSet("Biped.UpperBody,Biped.LeftFoot,Biped.RightFoot,Biped.RightTip,Biped.LeftTip", false);	
	
//Rename Biped model to ValveBiped	
SetValue("Biped.Name", "ValveBiped", null);
	
	
	
	
	