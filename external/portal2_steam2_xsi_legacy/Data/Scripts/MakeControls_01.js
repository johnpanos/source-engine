/******************************************************************************************
*******************************************************************************************
	Build the Valve control rig ontop of Default xsi rig
*******************************************************************************************
******************************************************************************************/

//constrain shadow pelvis to hipPlate (vs Upperbody control), place upperbody control.
//currently we're placing the control right between the thigh joints
RemoveAllCns("Shadow.Bip01_Pelvis", null);
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 1);
SetUserPref("SI3D_NODETRANSFORM_CHILD_COMPENSATE", 1);
ApplyCns("Pose", "Shadow.Bip01_Pelvis", "Biped.HipPlate", true);
CutObj("Biped.HipPlate");
SetValue("Biped.UpperBody.kine.local.nrotx", 0, null);
MatchTransform("Biped.UpperBody", "Shadow.Bip01_Pelvis", siTrn, null);
SetNeutralPose(Array("Biped.UpperBody"), siSRT, false);
CopyPaste("Biped.HipPlate", null, "Biped.UpperBody", 1);
Refresh (null);
	//set upperbody rot order to XZY to fight gimball
SetValue("Biped.UpperBody.kine.local.rotorder", 1, null);

	//create pelvis-only control for playbabies
CreatePrim("Cube", "MeshSurface", "PelvisControl", "Biped.Upperbody");
MatchTransform("Biped.PelvisControl", "Biped.UpperBody", siTrn, null);
	//display property	
AddProp("Display Property", "Biped.PelvisControl", "", "", null);
SetValue("Biped.PelvisControl.display.staticsel", 0, null);
SetValue("Biped.PelvisControl.display.intsel", 0, null);
SetValue("Biped.PelvisControl.display.playbacksel", 0, null);
SetValue("Biped.PelvisControl.display.staticunselnear", 0, null);
SetValue("Biped.PelvisControl.display.intunselnear", 0, null);
SetValue("Biped.PelvisControl.display.playbackunselnear", 0, null);
SetValue("Biped.PelvisControl.display.staticunselfar", 0, null);
SetValue("Biped.PelvisControl.display.intunselfar", 0, null);
SetValue("Biped.PelvisControl.display.playbackunselfar", 0, null);
	//cut spine root so pelvis behaves like original ValveBiped
	//constrain hipPlate to PelvisControl
CopyPaste("Biped.SpineEff", null, "Biped.UpperBody", 1);
ApplyCns("Pose", "Biped.HipPlate", "Biped.PelvisControl", true);

//========================================================================================
	//Create Spine Controls
//========================================================================================
	//place them so they are zeroed out in reference pose,
	//aligned to the bone pivot, but compensated
	//so their axes are the same as Upperbody control (for similar FCurve meaning),
	//change rot order for desired behaviour
	
	//SpineControl
CreatePrim("Sphere", "NurbsSurface", "SpineControl", "Biped.Upperbody");	
SetValue("Biped.SpineControl.sphere.radius", 1.5, null);
SetValue("Biped.SpineControl.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.SpineControl.surfmsh.geom.subdivv", 2, null);
	//align to bone pivot
MatchTransform("Biped.SpineControl", "Biped.Lumbar1", siSRT, null);
	//then rotate so Rot axes match Upperbody (for FCurve consistency)
Rotate(null, 0, 0, -90, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Rotate(null, 0, -90, 0, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
	//then zero out (using neutral pose)
SetNeutralPose(Array("Biped.SpineControl"), siSRT, false);
	//Set rot order YZX
SetValue("Biped.SpineControl.kine.local.rotorder", 3, null);	
	//move control points back
SelectGeometryComponents("Biped.SpineControl.pnt[*]");
Translate(null, 0, 0, -12, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);	

	//SpineControl1
CreatePrim("Sphere", "NurbsSurface", "SpineControl1", "Biped.SpineControl");	
SetValue("Biped.SpineControl1.sphere.radius", 1.5, null);
SetValue("Biped.SpineControl1.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.SpineControl1.surfmsh.geom.subdivv", 2, null);
	//align to bone pivot
MatchTransform("Biped.SpineControl1", "Biped.Lumbar2", siSRT, null);
	//then rotate so Rot axes match Upperbody
Rotate(null, 0, 0, -90, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Rotate(null, 0, -90, 0, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
	//then zero out (using neutral pose)
SetNeutralPose(Array("Biped.SpineControl1"), siSRT, false);
	//Set rot order YZX
SetValue("Biped.SpineControl1.kine.local.rotorder", 3, null);	
	//move control points back
SelectGeometryComponents("Biped.SpineControl1.pnt[*]");
Translate(null, 0, 0, -12, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);	

	//SpineControl2
CreatePrim("Sphere", "NurbsSurface", "SpineControl2", "Biped.SpineControl1");	
SetValue("Biped.SpineControl2.sphere.radius", 1.5, null);
SetValue("Biped.SpineControl2.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.SpineControl2.surfmsh.geom.subdivv", 2, null);
	//align to bone pivot
MatchTransform("Biped.SpineControl2", "Biped.Lumbar3", siSRT, null);
	//then rotate so Rot axes match Upperbody
Rotate(null, 0, 0, -90, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Rotate(null, 0, -90, 0, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
	//then zero out (using neutral pose)
SetNeutralPose(Array("Biped.SpineControl2"), siSRT, false);
	//Set rot order YZX
SetValue("Biped.SpineControl2.kine.local.rotorder", 3, null);	
	//move control points back
SelectGeometryComponents("Biped.SpineControl2.pnt[*]");
Translate(null, 0, 0, -12, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);	

	//SpineControl3
CreatePrim("Sphere", "NurbsSurface", "SpineControl3", "Biped.SpineControl2");	
SetValue("Biped.SpineControl3.sphere.radius", 1.5, null);
SetValue("Biped.SpineControl3.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.SpineControl3.surfmsh.geom.subdivv", 2, null);
	//align to bone pivot
MatchTransform("Biped.SpineControl3", "Biped.Neck", siSRT, null);
	//then rotate so Rot axes match Upperbody
Rotate(null, 0, 0, -90, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Rotate(null, 0, -180, 0, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
	//then zero out (using neutral pose)
SetNeutralPose(Array("Biped.SpineControl3"), siSRT, false);
	//Set rot order YZX
SetValue("Biped.SpineControl3.kine.local.rotorder", 3, null);	
	//move control points back
SelectGeometryComponents("Biped.SpineControl3.pnt[*]");
Translate(null, 0, 0, -12, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);	

	//NeckControl
CreatePrim("Sphere", "NurbsSurface", "NeckControl", "Biped.SpineControl3");	
SetValue("Biped.NeckControl.sphere.radius", 1.5, null);
SetValue("Biped.NeckControl.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.NeckControl.surfmsh.geom.subdivv", 2, null);
	//align to bone pivot
MatchTransform("Biped.NeckControl", "Biped.Neck1", siSRT, null);
	//then rotate so Rot axes match Upperbody
Rotate(null, 0, 0, -90, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Rotate(null, 0, -180, 0, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
	//then zero out (using neutral pose)
SetNeutralPose(Array("Biped.NeckControl"), siSRT, false);
	//Set rot order YZX
SetValue("Biped.NeckControl.kine.local.rotorder", 3, null);	
	//move control points back
SelectGeometryComponents("Biped.NeckControl.pnt[*]");
Translate(null, 0, 0, -12, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);	

	//HeadControl
CreatePrim("Sphere", "NurbsSurface", "HeadControl", "Biped.NeckControl");	
SetValue("Biped.HeadControl.sphere.radius", 1.5, null);
SetValue("Biped.HeadControl.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.HeadControl.surfmsh.geom.subdivv", 2, null);
	//align to bone pivot
MatchTransform("Biped.HeadControl", "Biped.Head", siSRT, null);
	//then rotate so Rot axes match Upperbody
Rotate(null, 0, 0, -90, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Rotate(null, 0, -180, 0, siRelative, siLocal, siObj, siXYZ, null, null, null, null, null, null, null, 0);
	//then zero out (using neutral pose)
SetNeutralPose(Array("Biped.HeadControl"), siSRT, false);
	//Set rot order YZX
SetValue("Biped.HeadControl.kine.local.rotorder", 3, null);	
	//move control points back
SelectGeometryComponents("Biped.HeadControl.pnt[*]");
Translate(null, 0, 0, -12, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);	
DeselectAll();
Refresh(null);
	
	//Constrain XSI rig to new SpineControls
	// pukes a "PROBLEMATIC EVALUATION CYCLES..." prob sometimes (?), dont know why
	// it doesnt say that when i do this manually, or run these 1 line @ a time
ApplyCns("Pose", "Biped.Lumbar1", "Biped.SpineControl", true);	
ApplyCns("Pose", "Biped.Lumbar2", "Biped.SpineControl1", true);	
ApplyCns("Pose", "Biped.Lumbar3", "Biped.SpineControl2", true);
ApplyCns("Pose", "Biped.Neck", "Biped.SpineControl3", true);
ApplyCns("Pose", "Biped.Neck1", "Biped.NeckControl", true);
ApplyCns("Pose", "Biped.Head", "Biped.HeadControl", true);

//======================================================================================
//	MAKE ARM CONTROLS
//======================================================================================

	//ClavicleControlL
CreatePrim("Sphere", "NurbsSurface", "ClavicleControlL", "Biped.SpineControl3");	
SetValue("Biped.ClavicleControlL.sphere.radius", 2, null);
SetValue("Biped.ClavicleControlL.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.ClavicleControlL.surfmsh.geom.subdivv", 2, null);
MatchTransform("Biped.ClavicleControlL", "Biped.LShoulder", siSRT, null);
SetNeutralPose(Array("Biped.ClavicleControlL"), siSRT, false);
SelectGeometryComponents("Biped.ClavicleControlL.pnt[*]");
Translate(null, 12, 6, 0, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);
ApplyCns("Pose", "Biped.LShoulder", "Biped.ClavicleControlL", true);

	//ClavicleControlR
	//	Want clavicle controls to be Neg scaled with respect.  This is happening
	//	automatically now because the RShoulderRoot is neg scaled.
	//	The neg scale can be applied in Neutral pose too, i imagine
CreatePrim("Sphere", "NurbsSurface", "ClavicleControlR", "Biped.SpineControl3");	
SetValue("Biped.ClavicleControlR.sphere.radius", 2, null);
SetValue("Biped.ClavicleControlR.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.ClavicleControlR.surfmsh.geom.subdivv", 2, null);
MatchTransform("Biped.ClavicleControlR", "Biped.RShoulder", siSRT, null);
SetNeutralPose(Array("Biped.ClavicleControlR"), siSRT, false);
SelectGeometryComponents("Biped.ClavicleControlR.pnt[*]");
Translate(null, -12, 6, 0, siRelative, siGlobal, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);
ApplyCns("Pose", "Biped.RShoulder", "Biped.ClavicleControlR", true);

	//Left UpVector
	//	Break UpV hierarchy, introduce UpVParent, PosConstrain parent to shoulder root
	//	for desired UpV behaviour.  redefine UpV neutral pose
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);	
GetPrim("Null", "LArmUpV_Parent", "Biped", null);	
ApplyCns("Position", "Biped.LArmUpV_Parent", "Biped.LRoot", null);	
CopyPaste("Biped.LArmUpV", null, "Biped.LArmUpV_Parent", 1);	
SetNeutralPose(Array("Biped.LArmUpV"), siSRT, false);	
	
	//Right UpVector
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);	
GetPrim("Null", "RArmUpV_Parent", "Biped", null);	
ApplyCns("Position", "Biped.RArmUpV_Parent", "Biped.RRoot", null);	
CopyPaste("Biped.RArmUpV", null, "Biped.RArmUpV_Parent", 1);	
SetNeutralPose(Array("Biped.RArmUpV"), siSRT, false);	
	
	//Left Hand Pos Control
	//	Create and place Control, Zero control, Constrain arm eff, 
CreatePrim("Sphere", "NurbsSurface", "HandControlPosL", "Biped.LArmUpV_Parent");	
SetValue("Biped.HandControlPosL.sphere.radius", 3, null);
SetValue("Biped.HandControlPosL.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.HandControlPosL.surfmsh.geom.subdivv", 2, null);
MatchTransform("Biped.HandControlPosL", "Biped.LArmEff", siTrn, null);
SetNeutralPose(Array("Biped.HandControlPosL"), siSRT, false);	
ApplyCns("Position", "Biped.LArmEff", "Biped.HandControlPosL", null);	
Refresh(null);	
	
	//Right Hand Pos Control
CreatePrim("Sphere", "NurbsSurface", "HandControlPosR", "Biped.RArmUpV_Parent");	
SetValue("Biped.HandControlPosR.sphere.radius", 3, null);
SetValue("Biped.HandControlPosR.surfmsh.geom.subdivu", 4, null);
SetValue("Biped.HandControlPosR.surfmsh.geom.subdivv", 2, null);
MatchTransform("Biped.HandControlPosR", "Biped.RArmEff", siTrn, null);
SetNeutralPose(Array("Biped.HandControlPosR"), siSRT, false);	
ApplyCns("Position", "Biped.RArmEff", "Biped.HandControlPosR", null);	
Refresh(null);	
	
	//Left Hand Rot Control
	//  Want Rot control to be aligned to the Forearm bone -- twist happens 
	//	local to forearm.
GetPrim("Null", "HandControlRotParentL", "Biped.LArmUpV_Parent", null);	
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);
ApplyCns("Pose", "Biped.HandControlRotParentL", "Biped.LForearm", null);
CreatePrim("Cube", "MeshSurface", "HandControlRotL", "Biped.HandControlRotParentL");	
MatchTransform("Biped.HandControlRotL", "Biped.LHand", siTrn, null);
MatchTransform("Biped.HandControlRotL", "Biped.HandControlRotParentL", siRot, null);
SelectGeometryComponents("Biped.HandControlRotL.pnt[*]");
Scale(null, .55, .5, .3, siRelative, siParent, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Translate(null, 3.2, 0, 0, siRelative, siParent, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);
	//change rot order, zero out
SetValue("Biped.HandControlRotL.kine.local.rotorder", 5, null);	
SetNeutralPose(Array("Biped.HandControlRotL"), siSRT, false);

	//Right Hand Rot Control
GetPrim("Null", "HandControlRotParentR", "Biped.RArmUpV_Parent", null);	
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);
ApplyCns("Pose", "Biped.HandControlRotParentR", "Biped.RForearm", null);
CreatePrim("Cube", "MeshSurface", "HandControlRotR", "Biped.HandControlRotParentR");	
MatchTransform("Biped.HandControlRotR", "Biped.RHand", siTrn, null);
MatchTransform("Biped.HandControlRotR", "Biped.HandControlRotParentR", siRot, null);
SelectGeometryComponents("Biped.HandControlRotR.pnt[*]");
Scale(null, .55, .5, .3, siRelative, siParent, siObj, siXYZ, null, null, null, null, null, null, null, 0);
Translate(null, -3.2, 0, 0, siRelative, siParent, siObj, siXYZ, null, null, null, null, null, null, null, null, null, 0);
	//change rot order, zero out
SetValue("Biped.HandControlRotR.kine.local.rotorder", 5, null);	
SetNeutralPose(Array("Biped.HandControlRotR"), siSRT, false);

	//constrain bones to Hand Rot Controls
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 1);	
ApplyCns("Pose", "Biped.LHand", "Biped.HandControlRotL", true);
ApplyCns("Pose", "Biped.RHand", "Biped.HandControlRotR", true);

//Fixup finger controls a bit, align root to first bone
AlignRootToFirstBone("Biped.LThumb1,Biped.LIndex1,Biped.LMiddle1,Biped.LRing1,Biped.LPinky1,Biped.RThumb1,Biped.RPinky1,Biped.RRing1,Biped.RMiddle1,Biped.RIndex1");



//============================================================================
// LEG/UPV CONTROLS
//============================================================================

	//Left leg UpV
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);
GetPrim("Null", "LLegUpV_Parent", "Biped", null);	
ApplyCns("Position", "Biped.LLegUpV_Parent", "Biped.LRoot1", null);
CopyPaste("Biped.LlegUpV", null, "Biped.LLegUpV_Parent", 1);

	//Right leg UpV
GetPrim("Null", "RLegUpV_Parent", "Biped", null);	
ApplyCns("Position", "Biped.RLegUpV_Parent", "Biped.RRoot1", null);
CopyPaste("Biped.RlegUpV", null, "Biped.RLegUpV_Parent", 1);

	//Set footroll angles to effectively remove the unwanted 2nd foot bone
SetValue("Biped.LPivotBase.kine.local.ToeGuide.R1", 0, null);
SetValue("Biped.RPivotBase.kine.local.ToeGuide.R1", 0, null);	

SetUserPref("SI3D_NODETRANSFORM_CHILD_COMPENSATE", 0);
SetUserPref("SI3D_CONSTRAINT_COMPENSATION_MODE", 0);

