//Match Bip_01_Pelvis position to existing skeleton

	//cut children of pelvis temporarily
SelectObj("Shadow.Bip01_L_Thigh", null, null);
ToggleSelection("Shadow.Bip01_R_Thigh", null, null);
ToggleSelection("Shadow.Bip01_Spine", null, null);
CutObj(null);

	//create and place temp position goal
GetPrim("Sphere", "tempPelvisPos", null, null);
ApplyCns("TwoPoints", "tempPelvisPos", "Shadow.Bip01_L_Thigh,Shadow.Bip01_R_Thigh", null);
Refresh (null);

	//move pelvis deformer to goal
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 1);
MatchTransform("Shadow.Bip01_Pelvis", "tempPelvisPos", siTrn, null);
Refresh (null);
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);
	
	//cleanup, reparent children
DeleteObj("tempPelvisPos");
CopyPaste("Shadow.Bip01_L_Thigh", null, "Shadow.Bip01_Pelvis", 1);
CopyPaste("Shadow.Bip01_R_Thigh", null, "Shadow.Bip01_Pelvis", 1);
CopyPaste("Shadow.Bip01_Spine", null, "Shadow.Bip01_Pelvis", 1);
Refresh (null);

	//fixup clavicle hierarchy
CopyPaste("Shadow.Bip01_R_Clavicle", null, "Shadow.Bip01_Spine4", 1);
CopyPaste("Shadow.Bip01_L_Clavicle", null, "Shadow.Bip01_Spine4", 1);
	
	//remove extra unwanted and foot bones, fixup hierarchy 
CopyPaste("Shadow.Bip01_L_Toe0", null, "Shadow.Bip01_L_Foot", 1);	
DeleteObj("Shadow.LFootBone2");
CopyPaste("Shadow.Bip01_R_Toe0", null, "Shadow.Bip01_R_Foot", 1);
DeleteObj("Shadow.RFootBone2");

// Adjust Nulls' Axes so pivot matches (or is close to) existing skeleton
// assuming the original rig/skeleton is in the scene, named ValveBiped
	
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 1);	

//Select all shadow nulls, force scale to [1,1,1]
SelectObj("Shadow.Bip01_Pelvis", "BRANCH", true);
SelectChildNodes(null, null, null);	
Scale(null, 1, 1, 1, siAbsolute, siPivot, siObj, siX, null, null, null, null, null, null, null, 0);
Scale(null, 1, 1, 1, siAbsolute, siPivot, siObj, siY, null, null, null, null, null, null, null, 0);
Scale(null, 1, 1, 1, siAbsolute, siPivot, siObj, siZ, null, null, null, null, null, null, null, 0);
//DeselectAll();
Refresh (null);
SceneRefresh();

//align x,y,z rot of all bones to ValveBiped corresponding bones
//have to do each command 3x for it to work it seems, else the fingers dont all take
//first verify ValveBiped is in scene, if not then skip

oTest = ActiveSceneRoot.findChildren("ValveBiped");
if(oTest.count == 1)
{
SetUserPref("SI3D_NODETRANSFORM_CHILD_COMPENSATE", 1);
for (i=0; i < selection.count; i++){
	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.rotx", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.rotx", otemp, null);
	Refresh (null);	
	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.roty", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.roty", otemp, null);
	Refresh (null);	
	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.rotz", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.rotz", otemp, null);
	Refresh (null);	

	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.rotx", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.rotx", otemp, null);
	Refresh (null);	
	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.roty", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.roty", otemp, null);
	Refresh (null);	
	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.rotz", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.rotz", otemp, null);
	Refresh (null);	

	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.rotx", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.rotx", otemp, null);
	Refresh (null);	
	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.roty", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.roty", otemp, null);
	Refresh (null);	
	otemp = GetValue("ValveBiped." + selection(i).name + ".kine.global.rotz", null);
	SetValue("Shadow." + selection(i).name + ".kine.global.rotz", otemp, null);
	Refresh (null);	

	}	// end for loop
	SetValue("ValveBiped.Name", "ValveBiped_old", null);
}	// end if condition
DeselectAll();
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);
SetUserPref("SI3D_NODETRANSFORM_CHILD_COMPENSATE", 0);