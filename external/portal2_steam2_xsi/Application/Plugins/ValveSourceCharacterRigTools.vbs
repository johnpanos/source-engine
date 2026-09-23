Const gToolName = "ValveSource CharacterRig Tools (VBScript)"
Const gDebug = false
dim gToolPath : gToolPath = ""

function XSILoadPlugin( in_reg )
	in_reg.Author = "ValveSource and Softimage"
	in_reg.Name = gToolName
	in_reg.Major = 1
	in_reg.Minor = 1

	call in_reg.RegisterCommand("vstCreateRigFromGuide", "vstCreateRigFromGuide")

	XSILoadPlugin = true
	
end function

'-----------------------------------
' function: Hierachy from Biped
'
' ValveBiped
'
' misner

'----------------------------------- 
sub hierachy_from_BipedProc( inModel )
	'
	' Disable Auto Inspect
	'
	PreviousAutoInspect = GetUserPref("AutoInspectEnabled")
	SetUserPref "AutoInspectEnabled", False
	'
	' Save Child Compensation
	'
	PreviousCompensation = GetUserPref("SI3D_NODETRANSFORM_CHILD_COMPENSATE")
	SetUserPref "SI3D_NODETRANSFORM_CHILD_COMPENSATE", 0
	dim hType, valid, nbVertebra 

	if inModel = "" then
		inModel = gToolInstallDir & "\Data\Models\Male\ValveBiped.emdl"
	end if
	'
	' Connect the Rig with the Guide
	'
 	attach_skeleton inModel
	'
	' Restore preferences
	'
	SetUserPref "AutoInspectEnabled", PreviousAutoInspect 			
	SetUserPref "SI3D_NODETRANSFORM_CHILD_COMPENSATE", PreviousCompensation
end sub

'*********************************************
'*********************************************
'*********************************************

' Attach Skeleton

'*********************************************
'*********************************************
'*********************************************
'---------------------------------------------

sub attach_skeleton( inModel )
	

	mpath = inModel

	set myModel = Application.ActiveProject.ActiveScene.Root.FindChild("ValveBiped")
	if typename ( myModel ) = "Nothing" then
		ImportModel mpath ,,,myModel
	end if

'	set oGuide = Application.ActiveProject.ActiveScene.Root.FindChild("ValveGuide")
	if typename( oGuide ) = "Nothing" then
		dim rtn : rtn = msgbox("Do you want to load the Valve Character Rig and continue?",vbYesNo,"Confirm Loading Guide")
		if rtn = vbNo then
			exit sub
		end if
		ImportModel gToolInstallDir & "\Data\Models\ValveGuide.emdl"
	end if
	
	AS_model_and_Bip01_match myModel, valid
	if valid = False then exit sub
	
	AS_Attach_hips myModel, valid
	
	AS_Attach_Bip01_Pelvis myModel, valid
	
	AS_Attach_Hands myModel, valid
	
	AS_attach_feet myModel, valid
	
	attach_feet myModel, valid
	
end sub
'---------------------------------------------
' Model and Global SRT Match
'---------------------------------------------
function AS_model_and_Bip01_match(in_rigModel, out_valid)

	dim oRoot, PModel, PgSRT, RiggSRT

	set oRoot = Application.ActiveProject.ActiveScene.Root
	
	set PModel = oRoot.FindChild("ValveGuide")
	set PgSRT= oRoot.FindChild("ValveGuide.GlobalSRT")
	set RiggSRT = oRoot.FindChild(in_rigModel & ".Bip01")
	
	

	'match up the models
	rt_match PModel, in_rigModel
	
	'match up the globalSRT's
	rt_match PgSRT, RiggSRT
	
	out_valid = True
	
end function
'---------------------------------------------
' Attach Hips
'---------------------------------------------
function AS_Attach_hips (in_rigModel, out_valid)

	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0
	SelectObj in_rigModel & ".Bip01_Pelvis", "BRANCH"
	MatchTransform , "ValveGuide.cog", siTrn
	
	
	'now match the top of the legs
	MatchTransform in_rigModel & ".LLegRoot", "ValveGuide.LLeg", siTrn
	MatchTransform in_rigModel & ".RLegRoot", "ValveGuide.RLeg", siTrn
		
	out_valid = True
		
end function
'---------------------------------------------
' Attach Upper Body
'---------------------------------------------
function AS_Attach_Bip01_Pelvis(in_rigModel, out_valid)
	
	MatchTransform in_rigModel & ".Bip01_Pelvis", "ValveGuide.SpineBase", siTrn
	MatchTransform in_rigModel & ".Spine_Root", "ValveGuide.SpineBase", siTrn

	'match the bone lengths
	dim s1, s2, s3, s4

	s1 = GetValue("ValveGuide.bone_3.bone.length")
	s2 = GetValue("ValveGuide.bone_2.bone.length")
	s3 = GetValue("ValveGuide.bone_1.bone.length")
	s4 = GetValue("ValveGuide.bone37.bone.length")

	SetValue in_rigModel & ".Bip01_Spine.bone.length", s1
	SetValue in_rigModel & ".Bip01_Spine1.bone.length", s2
	SetValue in_rigModel & ".Bip01_Spine2.bone.length", s3
	SetValue in_rigModel & ".Bip01_Spine4.bone.length", s4

	'match the positions
	MatchTransform in_rigModel & ".Bip01_Spine1", "ValveGuide.Spine1", siTrn
	MatchTransform in_rigModel & ".Bip01_Spine2", "ValveGuide.Spine2",siTrn
	MatchTransform in_rigModel & ".Bip01_Spine4", "ValveGuide.Spine3",siTrn
	MatchTransform in_rigModel & ".eff", "ValveGuide.RibCageEnd",siTrn

	refresh
	
	dim b1,b2,b3,b4,b5,b6,b7,b8
	
	'head
	b1 = GetValue("ValveGuide.bone35.bone.length")
	b2 = GetValue("ValveGuide.bone36.bone.length")

	SetValue in_rigModel & ".Bip01_Neck1.bone.length", b1
	SetValue in_rigModel & ".Bip01_Head1.bone.length", b2
	
	MatchTransform in_rigModel & ".Bip01_Neck1", "ValveGuide.RibCageEnd",siTrn	
	MatchTransform in_rigModel & ".Bip01_Head1", "ValveGuide.Neck",siTrn	
	MatchTransform in_rigModel & ".eff1", "ValveGuide.Head",siTrn	
	
	refresh
	
	'arms
	
	b3 = GetValue("ValveGuide.bone32.bone.length")
	b4 = GetValue("ValveGuide.bone33.bone.length")
	b5 = GetValue("ValveGuide.bone34.bone.length")
	b6 = GetValue("ValveGuide.bone31.bone.length")
	b7 = GetValue("ValveGuide.bone30.bone.length")	
	b8 = GetValue("ValveGuide.bone29.bone.length")
		
	
	SetValue in_rigModel & ".Bip01_R_Clavicle.bone.length", b3
	SetValue in_rigModel & ".Bip01_R_UpperArm.bone.length", b4
	SetValue in_rigModel & ".Bip01_R_Forearm.bone.length", b5
	SetValue in_rigModel & ".Bip01_L_Clavicle.bone.length", b6
	SetValue in_rigModel & ".Bip01_L_UpperArm.bone.length", b7
	SetValue in_rigModel & ".Bip01_L_Forearm.bone.length", b8
	
	
	
	MatchTransform in_rigModel & ".RRootShoulder", "ValveGuide.RShoulderStart",siTrn
	MatchTransform in_rigModel & ".RShoulderEff", "ValveGuide.RShoulder",siTrn
	MatchTransform in_rigModel & ".Bip01_R_Forearm", "ValveGuide.RElbow",siTrn
	MatchTransform in_rigModel & ".RArmEff", "ValveGuide.RHand",siTrn
	
	MatchTransform in_rigModel & ".LRootShoulder", "ValveGuide.LShoulderStart",siTrn
	MatchTransform in_rigModel & ".LShoulderEff", "ValveGuide.LShoulder",siTrn
	MatchTransform in_rigModel & ".Bip01_L_Forearm", "ValveGuide.LElbow",siTrn
	MatchTransform in_rigModel & ".LArmEff", "ValveGuide.LHand",siTrn
	
	refresh

	Align_Arms in_rigModel 

	out_valid = True
	
end function
'---------------------------------------------
' Attach Hands
'---------------------------------------------
function AS_Attach_Hands(in_rigModel, out_valid)



	out_valid = False
	dim c1
	
	'right hand

	'match the bone lengths
	dim b0, b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15
	
	b0 = GetValue("ValveGuide.bone62.bone.length")	
	
	b1 = GetValue("ValveGuide.bone46.bone.length")
	b2 = GetValue("ValveGuide.bone47.bone.length")
	b3 = GetValue("ValveGuide.bone48.bone.length")
	b4 = GetValue("ValveGuide.bone49.bone.length")
	b5 = GetValue("ValveGuide.bone50.bone.length")
	b6 = GetValue("ValveGuide.bone51.bone.length")
	b7 = GetValue("ValveGuide.bone52.bone.length")	
	b8 = GetValue("ValveGuide.bone53.bone.length")
	b9 = GetValue("ValveGuide.bone54.bone.length")
	b10 = GetValue("ValveGuide.bone55.bone.length")
	b11 = GetValue("ValveGuide.bone56.bone.length")
	b12 = GetValue("ValveGuide.bone57.bone.length")
	b13 = GetValue("ValveGuide.bone58.bone.length")
	b14 = GetValue("ValveGuide.bone59.bone.length")
	b15 = GetValue("ValveGuide.bone60.bone.length")
	
	
	SetValue in_rigModel & ".Bip01_R_Hand.bone.length", b0
	
	SetValue in_rigModel & ".Bip01_R_Finger0.bone.length", b1*.5
	SetValue in_rigModel & ".Bip01_R_Finger01.bone.length", b2*.5
	SetValue in_rigModel & ".Bip01_R_Finger02.bone.length", b3*.5
	SetValue in_rigModel & ".Bip01_R_Finger1.bone.length", b4*.5
	SetValue in_rigModel & ".Bip01_R_Finger11.bone.length", b5*.5
	SetValue in_rigModel & ".Bip01_R_Finger12.bone.length", b6*.5
	SetValue in_rigModel & ".Bip01_R_Finger2.bone.length", b7*.5
	SetValue in_rigModel & ".Bip01_R_Finger21.bone.length", b8*.5
	SetValue in_rigModel & ".Bip01_R_Finger22.bone.length", b9*.5
	SetValue in_rigModel & ".Bip01_R_Finger3.bone.length", b10*.5
	SetValue in_rigModel & ".Bip01_R_Finger31.bone.length", b11*.5
	SetValue in_rigModel & ".Bip01_R_Finger31.bone.length", b12*.5
	SetValue in_rigModel & ".Bip01_R_Finger4.bone.length", b13*.5
	SetValue in_rigModel & ".Bip01_R_Finger41.bone.length", b14*.5
	SetValue in_rigModel & ".Bip01_R_Finger42.bone.length", b15*.5
	

	
	Rotate in_rigModel & ".Bip01_R_Hand", 0, 0, 0, siAbsolute, 70, siObj, siXYZ
'	set c0 = ApplyCns ("Position", in_rigModel & ".RHandEffector", "ValveGuide.RHandEnd")	
	MatchTransform in_rigModel & ".RHandEffector", "ValveGuide.RHandEnd", siTrn
	
	MatchTransform in_rigModel & ".RThumbRoot", "ValveGuide.RThumb1", siTrn
	MatchTransform in_rigModel & ".RIndexRoot", "ValveGuide.RIndex1", siTrn
	MatchTransform in_rigModel & ".RMiddleRoot", "ValveGuide.RMiddle1", siTrn
	MatchTransform in_rigModel & ".RRingRoot", "ValveGuide.RRing1", siTrn
	MatchTransform in_rigModel & ".RPinkyRoot", "ValveGuide.RPinky1", siTrn	


	MatchTransform in_rigModel & ".Bip01_R_Finger01", "ValveGuide.RThumb2",siTrn
	MatchTransform in_rigModel & ".Bip01_R_Finger02", "ValveGuide.RThumb3",siTrn
	MatchTransform in_rigModel & ".RThumbEffector", "ValveGuide.RThumb4",siTrn
'
	MatchTransform in_rigModel & ".Bip01_R_Finger11", "ValveGuide.RIndex2",siTrn
	MatchTransform in_rigModel & ".Bip01_R_Finger12", "ValveGuide.RIndex3",siTrn
	MatchTransform in_rigModel & ".RIndexEffector", "ValveGuide.RIndex4",siTrn

	MatchTransform in_rigModel & ".Bip01_R_Finger21", "ValveGuide.RMiddle2",siTrn
	MatchTransform in_rigModel & ".Bip01_R_Finger22", "ValveGuide.RMiddle3",siTrn
	MatchTransform in_rigModel & ".RMiddleEffector", "ValveGuide.RMiddle4",siTrn


	MatchTransform in_rigModel & ".Bip01_R_Finger31", "ValveGuide.RRing2",siTrn
	MatchTransform in_rigModel & ".Bip01_R_Finger32", "ValveGuide.RRing3",siTrn
	MatchTransform in_rigModel & ".RRingEffector", "ValveGuide.RRing4",siTrn

	MatchTransform in_rigModel & ".Bip01_R_Finger41", "ValveGuide.RPinky2",siTrn
	MatchTransform in_rigModel & ".Bip01_R_Finger42", "ValveGuide.RPinky3",siTrn
	MatchTransform in_rigModel & ".RPinkyEffector", "ValveGuide.RPinky4",siTrn
	
	

	
	refresh

	'left hand

	'match the bone lengths
	
	b0 = GetValue("ValveGuide.bone62.bone.length")	
	
	b1 = GetValue("ValveGuide.bone.bone.length")
	b2 = GetValue("ValveGuide.bone15.bone.length")
	b3 = GetValue("ValveGuide.bone16.bone.length")
	b4 = GetValue("ValveGuide.bone19.bone.length")
	b5 = GetValue("ValveGuide.bone18.bone.length")
	b6 = GetValue("ValveGuide.bone17.bone.length")
	
	b7 = GetValue("ValveGuide.bone20.bone.length")	
	b8 = GetValue("ValveGuide.bone21.bone.length")
	b9 = GetValue("ValveGuide.bone22.bone.length")
	
	b10 = GetValue("ValveGuide.bone23.bone.length")
	b11 = GetValue("ValveGuide.bone24.bone.length")
	b12 = GetValue("ValveGuide.bone25.bone.length")
	
	b13 = GetValue("ValveGuide.bone26.bone.length")
	b14 = GetValue("ValveGuide.bone27.bone.length")
	b15 = GetValue("ValveGuide.bone28.bone.length")
	
	
	SetValue in_rigModel & ".Bip01_L_Hand.bone.length", b0
	
	SetValue in_rigModel & ".Bip01_L_Finger0.bone.length", b1*.5
	SetValue in_rigModel & ".Bip01_L_Finger01.bone.length", b2*.5
	SetValue in_rigModel & ".Bip01_L_Finger02.bone.length", b3*.5
	SetValue in_rigModel & ".Bip01_L_Finger1.bone.length", b4*.5
	SetValue in_rigModel & ".Bip01_L_Finger11.bone.length", b5*.5
	SetValue in_rigModel & ".Bip01_L_Finger12.bone.length", b6*.5
	SetValue in_rigModel & ".Bip01_L_Finger2.bone.length", b7*.5
	SetValue in_rigModel & ".Bip01_L_Finger21.bone.length", b8*.5
	SetValue in_rigModel & ".Bip01_L_Finger22.bone.length", b9*.5
	SetValue in_rigModel & ".Bip01_L_Finger3.bone.length", b10*.5
	SetValue in_rigModel & ".Bip01_L_Finger31.bone.length", b11*.5
	SetValue in_rigModel & ".Bip01_L_Finger31.bone.length", b12*.5
	SetValue in_rigModel & ".Bip01_L_Finger4.bone.length", b13*.5
	SetValue in_rigModel & ".Bip01_L_Finger41.bone.length", b14*.5
	SetValue in_rigModel & ".Bip01_L_Finger42.bone.length", b15*.5
	


'	set c0 = ApplyCns ("Position", in_rigModel & ".LHandEffector", "ValveGuide.LHandEnd")	
	matchTransform in_rigModel & ".LHandEffector", "ValveGuide.LHandEnd", siTrn
	refresh

'	DeleteObj c0

	MatchTransform in_rigModel & ".LThumbRoot", "ValveGuide.LThumb1", siTrn
	MatchTransform in_rigModel & ".LIndexRoot", "ValveGuide.LIndex1", siTrn
	MatchTransform in_rigModel & ".LMiddleRoot", "ValveGuide.LMiddle1", siTrn
	MatchTransform in_rigModel & ".LRingRoot", "ValveGuide.LRing1", siTrn
	MatchTransform in_rigModel & ".LPinkyRoot", "ValveGuide.LPinky1", siTrn	


		

	MatchTransform in_rigModel & ".Bip01_L_Finger01", "ValveGuide.LThumb2",siTrn
	MatchTransform in_rigModel & ".Bip01_L_Finger02", "ValveGuide.LThumb3",siTrn
	MatchTransform in_rigModel & ".LThumbEffector", "ValveGuide.LThumb4",siTrn

	MatchTransform in_rigModel & ".Bip01_L_Finger11", "ValveGuide.LIndex2",siTrn
	MatchTransform in_rigModel & ".Bip01_L_Finger12", "ValveGuide.LIndex3",siTrn
	MatchTransform in_rigModel & ".LIndexEffector", "ValveGuide.LIndex4",siTrn

	MatchTransform in_rigModel & ".Bip01_L_Finger21", "ValveGuide.LMiddle2",siTrn
	MatchTransform in_rigModel & ".Bip01_L_Finger22", "ValveGuide.LMiddle3",siTrn
	MatchTransform in_rigModel & ".LMiddleEffector", "ValveGuide.LMiddle4",siTrn

	MatchTransform in_rigModel & ".Bip01_L_Finger31", "ValveGuide.LRing2",siTrn
	MatchTransform in_rigModel & ".Bip01_L_Finger32", "ValveGuide.LRing3",siTrn
	MatchTransform in_rigModel & ".LRingEffector", "ValveGuide.LRing4",siTrn

	MatchTransform in_rigModel & ".Bip01_L_Finger41", "ValveGuide.LPinky2",siTrn
	MatchTransform in_rigModel & ".Bip01_L_Finger42", "ValveGuide.LPinky3",siTrn
	MatchTransform in_rigModel & ".LPinkyEffector", "ValveGuide.LPinky4",siTrn
	
	refresh

	out_valid = True
	
end function


'---------------------------------------------
' Attach Feet
'---------------------------------------------
function AS_attach_feet(in_rigModel, out_valid)

	out_valid = False

	'match the bone lengths
	dim b1,b2,b3,b4,b5,b6,b7,b8
	b1 = GetValue("ValveGuide.bone42.bone.length")
	b2 = GetValue("ValveGuide.bone43.bone.length")
	b3 = GetValue("ValveGuide.bone44.bone.length")
	b4 = GetValue("ValveGuide.bone45.bone.length")
	b5 = GetValue("ValveGuide.bone38.bone.length")
	b6 = GetValue("ValveGuide.bone39.bone.length")
	b7 = GetValue("ValveGuide.bone40.bone.length")	
	b8 = GetValue("ValveGuide.bone61.bone.length")
	
	SetValue in_rigModel & ".Bip01_R_Thigh.bone.length", b1
	SetValue in_rigModel & ".Bip01_R_Calf.bone.length", b2
	SetValue in_rigModel & ".Bip01_R_Foot.bone.length", b3
	SetValue in_rigModel & ".Bip01_R_Toe0.bone.length", b4
	SetValue in_rigModel & ".Bip01_L_Thigh.bone.length", b5
	SetValue in_rigModel & ".Bip01_L_Calf.bone.length", b6
	SetValue in_rigModel & ".Bip01_L_Foot.bone.length", b7
	SetValue in_rigModel & ".Bip01_L_Toe0.bone.length", b8
												
 	'match the foot postition
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0
	
	SelectObj in_rigModel & ".RLegRoot", "BRANCH"
	MatchTransform , "ValveGuide.RLeg", siTrn
	
	SelectObj in_rigModel & ".LLegRoot", "BRANCH"
	MatchTransform , "ValveGuide.LLeg", siTrn		
										
	MatchTransform in_rigModel & ".Bip01_R_Calf", "ValveGuide.RKnee",siTrn
	MatchTransform in_rigModel & ".RFootEff", "ValveGuide.RFoot",siTrn
	MatchTransform in_rigModel & ".Bip01_R_Toe0", "ValveGuide.RHeel",siTrn
	MatchTransform in_rigModel & ".RToeEff", "ValveGuide.RToe",siTrn
	
	MatchTransform in_rigModel & ".Bip01_L_Calf", "ValveGuide.LKnee",siTrn
	MatchTransform in_rigModel & ".LFootEff", "ValveGuide.LFoot",siTrn
	MatchTransform in_rigModel & ".Bip01_L_Toe0", "ValveGuide.LHeel",siTrn
	MatchTransform in_rigModel & ".LToeEff", "ValveGuide.LToe",siTrn
	refresh
	
	Align_Legs in_rigModel
		
end function
'---------------------------------------------
' FK Prep
'---------------------------------------------
function AS_fk_prep(in_model)
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 1
	
	root_Alignment in_model, "SpineRoot"
	root_Alignment in_model, "Spine"	
	
	'shoulders
	root_Alignment in_model, "Bip01_Spine4"
	root_Alignment in_model, "Bip01_L_Finger22"	
	
	'hands
	root_Alignment in_model, "Bip01_R_Clavicle"	
		
		
	root_Alignment in_model, "Bip01_R_Finger12"	
	root_Alignment in_model, "Bip01_R_Finger02"	
	root_Alignment in_model, "RThumbEffector"	
	root_Alignment in_model, "Bip01_L_Finger41"	
	root_Alignment in_model, "Bip01_R_UpperArm"						
			
	root_Alignment in_model, "Bip01_L_Clavicle"

	root_Alignment in_model, "Bip01_L_UpperArm"	
	root_Alignment in_model, "Bip01_Neck1"	
	root_Alignment in_model, "LThumbEffector"	
	root_Alignment in_model, "Bip01_L_Finger02"
	root_Alignment in_model, "Bip01_L_Finger12"	
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0	
	
end function
'---------------------------------------------
' Root Alignment
'---------------------------------------------
function root_Alignment(in_model, in_rootstring)

	in_root = in_model.FindChild(in_rootstring)

	dim mySkeleton, BoneIndex, rroot, rbone, zero, newrot
	set mySkeleton =  GetSkeleton(in_root, BoneIndex ) 	
	
	set boneRot = XSIMath.CreateRotation
	set zero = XSIMath.CreateRotation
		
	mySkeleton(1).Kinematics.local.Transform.GetRotation boneRot
	boneRot.GetXYZAnglesValues x,y,z

	Rotate mySkeleton(0), 0, 0, XSIMath.RadiansToDegrees(z), siRelative, siLocal, siObj, siXYZ
	Rotate mySkeleton(0), 0, XSIMath.RadiansToDegrees(y), 0, siRelative, siLocal, siObj, siXYZ
	Rotate mySkeleton(0), XSIMath.RadiansToDegrees(x), 0, 0, siRelative, siLocal, siObj, siXYZ

	set boneTrans = mySkeleton(1).Kinematics.local.Transform
	zero.SetFromXYZAnglesValues 0,0,0 	
	
	boneTrans.SetRotation zero 
	
	mySkeleton(1).Kinematics.local.Transform = boneTrans 
		
end function
'---------------------------------------------
' Is Component 
'---------------------------------------------
function IsComponent(in_string)
	
	dim oRoot
	IsComponent = True
	set oRoot = Application.ActiveProject.ActiveScene.Root
	if typename(oRoot.FindChild(in_string)) = "Nothing" then IsComponent = False
	
end function
'--------------------------------------
' RT Match
'--------------------------------------
function rt_match (in_source, in_target)

	dim sx, sy, sz
	dim rx, ry, rz	
	dim tx, ty, tz
	
	sx = GetValue(in_source & ".kine.global.sclx")
	sy = GetValue(in_source & ".kine.global.scly")	
	sz = GetValue(in_source & ".kine.global.sclz")
	
	rx = GetValue(in_source & ".kine.global.rotx")
	ry = GetValue(in_source & ".kine.global.roty")
	rz = GetValue(in_source & ".kine.global.rotz")
	
	tx = GetValue(in_source & ".kine.global.posx")
	ty = GetValue(in_source & ".kine.global.posy")
	tz = GetValue(in_source & ".kine.global.posz")
	
	SelectObj in_target, "BRANCH"

	Scale , sx, sy, sz, siAbsolute, siGlobal, siObj, siXYZ
	Rotate , rx, ry, rz, siAbsolute, siGlobal, siObj, siXYZ
	Translate , tx, ty, tz, siAbsolute, siGlobal, siObj, siXYZ
	
end function
'-----------------------------------------------
' Set Prefangles
'-----------------------------------------------
function set_prefangles(in_bone)
	 
	' set the rotations to pref angles. 
	
	dim pref1, pref2, pref3, chain, bIndex
	
	set chain = GetSkeleton(in_bone, bIndex)
	
	'if not the root or the first bone set the pref of one back
	
	if bIndex > 1 then
		set pref1 = createobject("Sumatra\Scripting\Math\SIVector3")
		GetVector chain(bIndex -1) & ".kine.local.rot", pref1
		SetValue chain(bIndex - 1) &".joint.prefrotx"	, pref1.x
		SetValue chain(bIndex - 1) &".joint.prefroty"	, pref1.y
		SetValue chain(bIndex - 1) &".joint.prefrotz"	, pref1.z			
	end if	

	
	' if in_bone is not an effector set itself.
	
	if bIndex < chain.count - 1 then
		set pref2 = createobject("Sumatra\Scripting\Math\SIVector3")
		GetVector chain(bIndex) & ".kine.local.rot", pref2
		SetValue chain(bIndex) &".joint.prefrotx"	, pref2.x
		SetValue chain(bIndex) &".joint.prefroty"	, pref2.y
		SetValue chain(bIndex) &".joint.prefrotz"	, pref2.z	
	end if
	
	
	' if bIndex + 1 is beyond the size of the chain or and effector set
	' the pref angles
		
	if bIndex < chain.count - 2 then
		set pref3 = createobject("Sumatra\Scripting\Math\SIVector3")  
		GetVector chain(bIndex + 1) & ".kine.local.rot", pref3
		SetValue chain(bIndex + 1) &".joint.prefrotx"	, pref3.x
		SetValue chain(bIndex + 1) &".joint.prefroty"	, pref3.y
		SetValue chain(bIndex + 1) &".joint.prefrotz"	, pref3.z	
	end if	
	
	
end function

'-----------------------------------------------
' Get Vector
'-----------------------------------------------
function GetVector(in_Path, out_vector)

	'Create a vector containing the x y z component of the path
	'for example  GetValueVector(obj & "kine.local.pos", frame)
	Dim V
	set V = createobject("Sumatra\Scripting\Math\SIVector3")
	V.Set GetValue(in_Path & "x"),_
	GetValue(in_Path & "y"),_
	GetValue(in_Path & "z")
	set out_vector = V
end function
'---------------------------------------------
' Attach Feet
'---------------------------------------------
function attach_feet(in_rigModel, out_valid)

	out_valid = False
	

 	'match the foot postition
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0
	
	SelectObj in_rigModel & ".LeftFootParent", "BRANCH"
	MatchTransform , "ValveGuide.LFoot", siTrn
	
	SelectObj in_rigModel & ".RightFootParent", "BRANCH"
	MatchTransform , "ValveGuide.RFoot", siTrn											
		

	set root = ActiveProject.ActiveScene.Root								
	dim lFoot, lHeel, rFoot, rHeel
	
	'match the left rig
	
	set lFoot = root.FindChild(in_rigModel & ".LeftFoot")
	set lHeel = root.FindChild("ValveGuide.LHeel")
	align_foot lFoot, lHeel 
	
	SelectObj in_rigModel & ".heel", "BRANCH", True
	MatchTransform , "ValveGuide.LHeelPivot", siTrn
	MatchTransform in_rigModel & ".toe", "ValveGuide.LToe", siTrn
	SelectObj in_rigModel & ".mid", "BRANCH", True									
	MatchTransform , "ValveGuide.LHeel", siTrn																		
	MatchTransform in_rigModel & ".ankle", "ValveGuide.LFoot", siTrn	
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 1
	refresh
	MatchTransform in_rigModel & ".LeftToe", "ValveGuide.LHeel", siTrn
	MatchTransform in_rigModel & ".LToeEff", "ValveGuide.LToe", siTrn	
	
	DeleteObj in_rigModel & ".LeftToeTarget.kine.posecns"
	MatchTransform in_rigModel & ".LeftToeTarget", "ValveGuide.LToe", siTrn
'	ApplyCns "Pose", in_rigModel & ".LeftToeTarget", in_rigModel & ".LeftToe", True	
	MatchTransform in_rigModel & ".LeftToeTarget", in_rigModel & ".LeftToe", siSRT
	
'	DeleteObj in_rigModel & ".LToeEff.kine.poscns"	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0	
'	ApplyCns "Position", in_rigModel & ".LToeEff", in_rigModel & ".LeftToeTarget"
	MatchTransform in_rigModel & ".LToeEff", in_rigModel & ".LeftToeTarget", siTrn

	'match the right rig
	set rFoot = root.FindChild(in_rigModel & ".RightFoot")
	set rHeel = root.FindChild("ValveGuide.RHeel")
	align_foot rFoot, rHeel 
	
	SelectObj in_rigModel & ".heel1", "BRANCH", True
	MatchTransform , "ValveGuide.RHeelPivot", siTrn
	MatchTransform in_rigModel & ".toe1", "ValveGuide.RToe", siTrn
	SelectObj in_rigModel & ".mid1", "BRANCH", True									
	MatchTransform , "ValveGuide.RHeel", siTrn																		
	MatchTransform in_rigModel & ".ankle1", "ValveGuide.RFoot", siTrn	
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 1
	refresh
	MatchTransform in_rigModel & ".RightToe", "ValveGuide.RHeel", siTrn
	MatchTransform in_rigModel & ".RToeEff", "ValveGuide.RToe", siTrn	
	
	DeleteObj in_rigModel & ".RightToeTarget.kine.posecns"
	MatchTransform in_rigModel & ".RightToeTarget", "ValveGuide.RToe", siTrn
'	ApplyCns "Pose", in_rigModel & ".RightToeTarget", in_rigModel & ".RightToe", True	
	MatchTransform in_rigModel & ".RightToeTarget", in_rigModel & ".RightToe", siSRT
	
'	DeleteObj in_rigModel & ".RToeEff.kine.poscns"	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0	
'	ApplyCns "Position", in_rigModel & ".RToeEff", in_rigModel & ".RightToeTarget"	
	MatchTransform in_rigModel & ".RToeEff", in_rigModel & ".RightToeTarget", siTrn

	out_valid = True
	
end function
'------------------------------------
' Align Foot
'------------------------------------
function align_foot(in_foot, in_heel)

	dim Pose1, dCnst, Pose2, yRot
	
	set Pose1 = in_foot.Kinematics.Global.Transform

	' Point the foot at the Toe
	set dCnst = in_foot.Kinematics.AddConstraint("Direction", in_heel, False)
	dCnst.dirx = 0
	dCnst.dirz = 1
	dCnst.upvct_active = True

	'now calculate the rotation to get there only rotating on the global y axis

	set Pose2 = in_foot.Kinematics.Global.Transform

	get_y_rotation Pose2, Pose1, yRot


	DeleteObj dCnst
	in_foot.Kinematics.Global.Transform = Pose1

	if yRot > 90 then yRot = yRot - 180
	if yRot < -90 then yRot = yRot + 180
	
	Rotate in_foot, 0, yRot, 0, siAbsolute, 65, siObj, siY

end function
'------------------------------------
' Get Y rotation
'------------------------------------
' We're not completly matching the rotations because 
' small variations of the foot angle up or down 
' shouldn't be used to rotate the body 
'....................................
function get_y_rotation(in_t1, in_t2, out_yRot)
	
	dim pushOut, gv1, gv2, gt1, gt2, cp ,angle, sroot
	
	set sRoot = Application.ActiveProject.ActiveScene.Root	
	
	set pushOut = xsimath.CreateVector3 
	pushOut.set 5,0,0
	
	'get the pushOut vector from local cordinates to a
	'global position
	
	set gv1 = XSIMath.MapObjectPositionToWorldSpace(in_t1,pushOut)  
	set gv2 = XSIMath.MapObjectPositionToWorldSpace(in_t2,pushOut) 
	
	'get the global translation to bring both vectors back to the origion
	set gt1 = xsimath.CreateVector3 
	set gt2 = xsimath.CreateVector3 
	in_t1.getTranslation gt1 
	in_t2.getTranslation gt2 

	gv1.SubInPlace gt1 
	gv2.SubInPlace gt2 
	
	'now project those vectors onto the xz plane	
	gv1.y = 0
	gv2.y = 0
	
	'now get the angle between them
	
	angle = gv2.Angle(gv1)  

	'get the cross product
	set cp = xsimath.CreateVector3 
	cp.cross gv1, gv2
	
	out_yRot = XSIMath.RadiansToDegrees(angle)

	if cp.y > 0 then out_yRot = out_yRot * -1 
	
end function
'-----------------------------------
' Handle Orientation
'-----------------------------------
function LegHandleOrientation(inSkeleton, inHandle, inDiagnostic)
	
	' get the normal for the Handle orientation plane
	' on this skeleton it should be the x+ axis of the 
	' up vector rotation handle
	
	set HandlePlaneStart = XSIMath.CreateVector3
	set HandleTransform = inHandle.parent.Kinematics.Global.Transform
	HandleTransform.GetTranslation HandlePlaneStart
	set ZAxis = XSIMath.CreateVector3
	ZAxis.set 0,-1,0
	HandleTransform.AddLocalTranslation ZAxis
	set HandlePlaneEnd = XSIMath.CreateVector3
	HandleTransform.GetTranslation HandlePlaneEnd 
	set HandlePlane = XSIMath.CreateVector3
	HandlePlane.sub HandlePlaneEnd, HandlePlaneStart



	'Also get the zero point for the HandleParent
	'in this case it's the -z axis (0,0,-1)
	
	set ZeroTrans = inHandle.parent.Kinematics.Global.Transform
	set ZeroOri = XSIMath.CreateVector3
	ZeroOri.set 0,0,-1
	ZeroTrans.AddLocalTranslation ZeroOri
	set ZeroEnd = XSIMath.CreateVector3
	ZeroTrans.GetTranslation ZeroEnd
	set ZeroVector = XSIMath.CreateVector3
	ZeroVector.Sub ZeroEnd, HandlePlaneStart
	

	'now get the plane for the upvector
	ProcessUpV inSkeleton, inDiagnostic, UpVPlane, FirstBoneYplus
	
	'now get the projection of the UpVectorPlane onto the HandlePlane
	set Projection = XSIMath.CreateVector3
	Projection.cross UpVPlane, HandlePlane


	' when taking the vector between the two planes you need to pick
	' the one that closest matches the Y plus vector coming off the
	' first bone.
	
	set NegProjection = XSIMath.CreateVector3
	NegProjection.Negate Projection
	
	ArmDot1 = Projection.Dot(FirstBoneYplus)
	ArmDot2 = NegProjection.Dot(FirstBoneYplus)
	

		
	if ArmDot1 < .0001 and ArmDot1 > -.0001 then exit function
	
	if ArmDot2 > ArmDot1 then
		set Projection = NegProjection
	end if
	
	
	'now get the angle between the Projection and the ZeroVector
	
	angle = ZeroVector.Angle(Projection)
	angle = XSIMath.RadiansToDegrees(angle)
	
	'find the sign of the angle
	
	set SignCp = xsimath.CreateVector3 
	SignCp.cross ZeroVector, Projection
	
	'the sign can be determined by the projection of SignCp onto the handlePlane
	
	if SignCp.Dot(HandlePlane) > 0 then
		sign = -1
	else	
		sign = 1
	end if
	
	SetValue inHandle & ".kine.local.roty", angle * sign
	
	
	if inDiagnostic = True then

		logmessage "DOT WITH PROJECTION: " & ArmDot1
		logmessage "DOT WITH NEG PROJECTION: " & ArmDot2

		set FirstBoneYplusDraw = XSIMath.CreateVector3
		FirstBoneYplus.AddInPlace HandlePlaneStart
		DrawLine2 HandlePlaneStart, FirstBoneYplus, "FirstBoneYplus"	

		HandlePlane.ScaleInPlace 10
		set HandlePlaneDraw = XSIMath.CreateVector3
		HandlePlaneDraw.add HandlePlaneStart, HandlePlane
		DrawLine2 HandlePlaneStart, HandlePlaneDraw, "HandlePlane"
		
		ZeroVector.ScaleInPlace 10
		set ZeroVectorDraw = XSIMath.CreateVector3
		ZeroVectorDraw.add HandlePlaneStart, ZeroVector
		DrawLine2 HandlePlaneStart, ZeroVectorDraw, "ZeroVector"
		
		set ProjectionDraw = XSIMath.CreateVector3
		ProjectionDraw.add HandlePlaneStart, Projection
		DrawLine2 HandlePlaneStart, ProjectionDraw, "Projection"					
		
	end if
	
	
end function
'-----------------------------------
' Process UpV
'-----------------------------------
function ProcessUpV(inSkel, inDiagnostic, outUpVectorPlane, outFirstBoneYplus)
	
	set inBone = inSkel(1)
	
	'first get the global transform
	
	set Trans = inBone.Kinematics.Global.Transform
	set startPoint = XSIMath.CreateVector3
	Trans.GetTranslation startPoint
	
	'get the bone length
	length = inBone.length.value
	
	'get the bone roll
	roll = GetValue(inBone & ".joint.roll")
	
	set EffPointAtTrans = RootToEffectorTrans(inSkel)
	

	'spin back the Trans as though it wasn't affected by the bone roll
	
	set Trans = RotationByReference(EffPointAtTrans, Trans , 0, -roll, 0)
	

	' get the first Bone y+ (used to figure out which side of the two plane
	' intesection is the right orientation).
 	set pos = XSIMath.CreateVector3
	pos.Set 0, 7, 0
	set Trans2 = XSIMath.CreateTransform
	Trans2.Copy Trans
	Trans2.AddLocalTranslation pos	
	set FirstBoneYplus= XSIMath.CreateVector3
	Trans2.GetTranslation FirstBoneYplus
	FirstBoneYplus.SubInPlace startPoint
	set outFirstBoneYplus = FirstBoneYplus

	' Get the position of the effector
	set UpVEnd = XSIMath.CreateVector3
	inSkel(inSkel.count - 1).Kinematics.Global.Transform.GetTranslation UpVEnd	

	'get the End the of first Bone Vector, in the space of the removed roll
	set Bpos = XSIMath.CreateVector3
	set BoneEnd = XSIMath.CreateVector3
	Bpos.Set length, 0, 0
	Trans.AddLocalTranslation Bpos	
	Trans.GetTranslation BoneEnd 

	'now get the two vectors for the plane of the bone
	set V1 = XSIMath.CreateVector3
	set V2 = XSIMath.CreateVector3
	
	V1.sub BoneEnd, startPoint
	V2.sub UpVEnd, startPoint
		
	set UpVectorPlane = XSIMath.CreateVector3
	UpVectorPlane.Cross V1, V2

	if inDiagnostic = True then
		
			DrawLine startPoint, BoneEnd, "BonePlane"
			DrawLine startPoint, UpVEnd, "UpVLine"	
			
			UpVectorPlane.NormalizeInPlace
			UpVectorPlane.ScaleInPlace 5
			set UpVectorPlaneOnArm = XSIMath.CreateVector3
			UpVectorPlaneOnArm.add startPoint, UpVectorPlane
			DrawLine startPoint, UpVectorPlaneOnArm, "UpVectorPlane"	
	end if
	
	set outUpVectorPlane = UpVectorPlane
	
end function
'-----------------------------------
' Root To Effector Trans
'-----------------------------------
function RootToEffectorTrans(inSkel)

	'Get a Transform Describing the root pointing towards the effector
	
	set inEff = inSkel(0)
	set inRoot = inSkel(inSkel.count - 1)
	
	set startAxis = XSIMath.CreateVector3	
	set endAxis = XSIMath.CreateVector3
	set Axis = XSIMath.CreateVector3

	inEff.Kinematics.Global.Transform.GetTranslation startAxis
	inRoot.Kinematics.Global.Transform.GetTranslation endAxis	
	Axis.sub startAxis, endAxis
		
	set PointAxis = XSIMath.CreateVector3	
	set cp  = XSIMath.CreateVector3	
	
	PointAxis.set 0,-1,0 
	angle = Axis.Angle(PointAxis)
	cp.cross PointAxis, Axis
	
	set Orot = XSIMath.CreateRotation	
	Orot.SetFromAxisAngle cp, angle
	
	set r1 = XSIMath.CreateVector3	
	Orot.GetXYZAngles r1
		
	'Create a Transform for the Return
	set returnTrans = XSIMath.CreateTransform
	returnTrans.SetTranslation endAxis
	returnTrans.SetRotation Orot

	set RootToEffectorTrans = returnTrans
	
end function
'-------------------------------------
' Rotation By Reference
'-------------------------------------
' inRef: the transform of Rotating Reference
' inObj: the transform to rotate in the space of inRef
' inX, inY, inZ: the rotation
'~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function RotationByReference(inRef, inObj, inX, inY, inZ)

	'Describe the Object in the Space of the Reference
	set RelativeObj = XSIMath.MapWorldPoseToObjectSpace(inRef, inObj)
	
	set rot = XSIMath.CreateRotation
	
	inX = XSIMath.DegreesToRadians(inX)
	inY = XSIMath.DegreesToRadians(inY)	
	inZ = XSIMath.DegreesToRadians(inZ)
		
	rot.SetFromXYZAnglesValues inX, inY, inZ
	
	RelativeObj.AddParentRotation rot
	
	'now put the position back into global space
	set returnTrans = XSIMath.MapObjectPoseToWorldSpace(inRef, RelativeObj)
	
	set RotationByReference = returnTrans 
	
end function
'-------------------------------------
' Draw Line
'-------------------------------------
function DrawLine(inStart, inEnd, inName)

	set myCurve = SICreateCurve ("crvlist", 1, 1)
	SISetCurvePoints myCurve, "("&inStart.x&","&inStart.y&","&inStart.z&"),("&inEnd.x&","&inEnd.y&","&inEnd.z&")", 0
	
	myCurve.name = inName
	myCurve.AddProperty("Display Property")
	myCurve.properties("display").wirecol.value = 507
	
end function
'-------------------------------------
' Draw Line 2
'-------------------------------------
function DrawLine2(inStart, inEnd, inName)

	set myCurve = SICreateCurve ("crvlist", 1, 1)
	SISetCurvePoints myCurve, "("&inStart.x&","&inStart.y&","&inStart.z&"),("&inEnd.x&","&inEnd.y&","&inEnd.z&")", 0
	
	myCurve.name = inName
	myCurve.AddProperty("Display Property")
	myCurve.properties("display").wirecol.value = 222
	
end function
'-------------------------------------
' Align Legs
'-------------------------------------
function Align_Legs(inModel)
	
	Diagnostic = False	
	
	set LThigh = inModel.FindChild("Bip01_L_Thigh")
	set LRoll = inModel.FindChild("LegRollL")
	
	set MySkeleton =  GetSkeleton(LThigh, BoneIndex ) 	
	LegHandleOrientation MySkeleton, LRoll, Diagnostic 
		
	set RThigh = inModel.FindChild("Bip01_R_Thigh")
	set RRoll = inModel.FindChild("LegRollR")
	
	set MySkeleton =  GetSkeleton(RThigh, BoneIndex ) 	
	LegHandleOrientation MySkeleton, RRoll, Diagnostic

end function
'-------------------------------------
' Align Arms
'-------------------------------------
function Align_Arms(inModel)
	
	Diagnostic = False	
	
	set LThigh = inModel.FindChild("Bip01_L_UpperArm")
	set LRoll = inModel.FindChild("LArmRoll")
	
	set MySkeleton =  GetSkeleton(LThigh, BoneIndex ) 	
	'ArmHandleOrientation MySkeleton, LRoll, Diagnostic 
		
	set RThigh = inModel.FindChild("Bip01_R_UpperArm")
	set RRoll = inModel.FindChild("RArmRoll")
	
	set MySkeleton =  GetSkeleton(RThigh, BoneIndex ) 	
	'ArmHandleOrientation MySkeleton, RRoll, Diagnostic

end function
'-----------------------------------
' Arm Handle Orientation
'-----------------------------------
function ArmHandleOrientation(inSkeleton, inHandle, inDiagnostic)
	
	' get the normal for the Handle orientation plane
	' on this skeleton it should be the x+ axis of the 
	' up vector rotation handle
	
	set HandlePlaneStart = XSIMath.CreateVector3
	set HandleTransform = inHandle.parent.Kinematics.Global.Transform
	HandleTransform.GetTranslation HandlePlaneStart
	set ZAxis = XSIMath.CreateVector3
	ZAxis.set 1,0,0
	HandleTransform.AddLocalTranslation ZAxis
	set HandlePlaneEnd = XSIMath.CreateVector3
	HandleTransform.GetTranslation HandlePlaneEnd 
	set HandlePlane = XSIMath.CreateVector3
	HandlePlane.sub HandlePlaneEnd, HandlePlaneStart



	'Also get the zero point for the HandleParent
	'in this case it's the -z axis (0,0,-1)
	
	set ZeroTrans = inHandle.parent.Kinematics.Global.Transform
	set ZeroOri = XSIMath.CreateVector3
	ZeroOri.set 0,0,-1
	ZeroTrans.AddLocalTranslation ZeroOri
	set ZeroEnd = XSIMath.CreateVector3
	ZeroTrans.GetTranslation ZeroEnd
	set ZeroVector = XSIMath.CreateVector3
	ZeroVector.Sub ZeroEnd, HandlePlaneStart
	

	'now get the plane for the upvector
	ProcessUpV inSkeleton, inDiagnostic, UpVPlane, FirstBoneYplus
	
	'now get the projection of the UpVectorPlane onto the HandlePlane
	set Projection = XSIMath.CreateVector3
	Projection.cross UpVPlane, HandlePlane


	' when taking the vector between the two planes you need to pick
	' the one that closest matches the Y plus vector coming off the
	' first bone.
	
	set NegProjection = XSIMath.CreateVector3
	NegProjection.Negate Projection
	
	ArmDot1 = Projection.Dot(FirstBoneYplus)
	ArmDot2 = NegProjection.Dot(FirstBoneYplus)
	

		
	if ArmDot1 < .0001 and ArmDot1 > -.0001 then exit function
	
	if ArmDot2 > ArmDot1 then
		set Projection = NegProjection
	end if
	
	
	'now get the angle between the Projection and the ZeroVector
	
	angle = ZeroVector.Angle(Projection)
	angle = XSIMath.RadiansToDegrees(angle)
	
	'find the sign of the angle
	
	set SignCp = xsimath.CreateVector3 
	SignCp.cross ZeroVector, Projection
	
	'the sign can be determined by the projection of SignCp onto the handlePlane
	
	if SignCp.Dot(HandlePlane) > 0 then
		sign = 1
	else	
		sign = -1
	end if
	
	SetValue inHandle & ".kine.local.rotx", angle * sign
	
	
	if inDiagnostic = True then

		logmessage "DOT WITH PROJECTION: " & ArmDot1
		logmessage "DOT WITH NEG PROJECTION: " & ArmDot2

		set FirstBoneYplusDraw = XSIMath.CreateVector3
		FirstBoneYplus.AddInPlace HandlePlaneStart
		DrawLine2 HandlePlaneStart, FirstBoneYplus, "FirstBoneYplus"	

		HandlePlane.ScaleInPlace 10
		set HandlePlaneDraw = XSIMath.CreateVector3
		HandlePlaneDraw.add HandlePlaneStart, HandlePlane
		DrawLine2 HandlePlaneStart, HandlePlaneDraw, "HandlePlane"
		
		ZeroVector.ScaleInPlace 10
		set ZeroVectorDraw = XSIMath.CreateVector3
		ZeroVectorDraw.add HandlePlaneStart, ZeroVector
		DrawLine2 HandlePlaneStart, ZeroVectorDraw, "ZeroVector"
		
		set ProjectionDraw = XSIMath.CreateVector3
		ProjectionDraw.add HandlePlaneStart, Projection
		DrawLine2 HandlePlaneStart, ProjectionDraw, "Projection"					
		
	end if
	
	
end function
'-----------------------------------



'
' Rigs rigs rigs
'
Sub ValveRigFromSoftimageRigProc
	PreviousAutoInspect = GetUserPref("AutoInspectEnabled")
	SetUserPref "AutoInspectEnabled", False
	'---- ValveRig from IsnerRig

	'#####  Setup Upperbody control, bip01 and pelvis, adjust hierarchy
	set oGuide = Application.ActiveProject.ActiveScene.Root.FindChild("ValveBiped")
	if typename( oGuide ) = "Nothing" then
		dim rtn : rtn = msgbox("Do you want to load the Biped Rig and continue?",vbYesNo,"Confirm Loading BipedRig")
		if rtn = vbNo then
			exit sub
		end if
		ImportModel gToolInstallDir & "\Data\Models\Male\ValveBiped.emdl"
	end if

	SelectObj "ValveBiped.Spine_Root", , True
	ToggleSelection "ValveBiped.LLegRoot", , True
	ToggleSelection "ValveBiped.RLegRoot", , True
	CutObj

	DeleteObj "ValveBiped.Bip01_Pelvis"

	ApplyCns "TwoPoints", "ValveBiped.Bip01", "ValveBiped.Bip01_R_Thigh,ValveBiped.Bip01_L_Thigh"
	SelectObj "ValveBiped.Bip01", , True
	Refresh
	RemoveAllCns

	MatchTransform "ValveBiped.UpperBody", "ValveBiped.Bip01", siSRT

		'-- create new pelvis, make child of bip01, align correctly 

	GetPrim "Cube"
	SetValue "cube.Name", "Bip01_Pelvis"
	MatchTransform "Bip01_Pelvis", "ValveBiped.Bip01", siSRT
	CopyPaste "Bip01_Pelvis", , "ValveBiped.Bip01", 1
	MatchTransform "ValveBiped.Bip01_Pelvis", "ValveBiped.Bip01", siRot


		'-- constrain bip01, setup hierarchy
		
	ApplyCns "Pose", "ValveBiped.Bip01", "ValveBiped.UpperBody"
	CopyPaste "ValveBiped.LLegRoot", , "ValveBiped.Bip01_Pelvis", 1
	CopyPaste "ValveBiped.RLegRoot", , "ValveBiped.Bip01_Pelvis", 1
	CopyPaste "ValveBiped.Spine_Root", , "ValveBiped.Bip01", 1


	'########  Create and Setup Spine, Neck, and Head controls 

		'-- SpineControl
		
			'-Create Objects
			
	CreatePrim "Sphere", "NurbsSurface"
	SetValue "sphere.surfmsh.geom.subdivu", 4
	SetValue "sphere.surfmsh.geom.subdivv", 2
	AddProp "Display Property"
	SetValue "sphere.display.staticsel", 0
	SetValue "sphere.display.intsel", 0
	SetValue "sphere.display.playbacksel", 0
	SetValue "sphere.display.staticunselnear", 0
	SetValue "sphere.display.intunselnear", 0
	SetValue "sphere.display.playbackunselnear", 0
	SetValue "sphere.display.staticunselfar", 0
	SetValue "sphere.display.intunselfar", 0
	SetValue "sphere.display.playbackunselfar", 0
	SetValue "sphere.Name", "SpineControl"

	GetPrim "Null"
	SetValue "null.Name", "SpineControlParent"

	GetPrim "Null"
	SetValue "null.Name", "SpineControlConstrainer"

			'-Setup Hierarchy, align objects, constrain Bone to control
			
	MatchTransform "SpineControlParent", "ValveBiped.Spine_Root", siSRT
	CopyPaste "SpineControlParent", , "ValveBiped.UpperBody", 1
	CopyPaste "SpineControl", , "ValveBiped.SpineControlParent", 1
	CopyPaste "SpineControlConstrainer", , "ValveBiped.SpineControl", 1
	MatchTransform "ValveBiped.SpineControl", "ValveBiped.SpineControlParent", siSRT
	SelectObj "ValveBiped.SpineControlParent", , True
	Rotate , 90, 0, 0, siAbsolute, siAdd, siObj, siX
	Rotate , 0, 0, 90, siAbsolute, siAdd, siObj, siZ
	MatchTransform "ValveBiped.SpineControlConstrainer", "ValveBiped.Bip01_Spine", siRot
	ApplyCns "Orientation", "ValveBiped.Bip01_Spine", "ValveBiped.SpineControlConstrainer"

	SelectObj "ValveBiped.SpineControl", , True
	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.SpineControl.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
	Translate , 0, 0, -10.120039710109, siRelative, siGlobal, siObj, siXYZ

		'-- SpineControl1
		
	Duplicate "ValveBiped.SpineControl", , 2, 1, 1, 0, 0, 1, 0, 1
	MatchTransform "ValveBiped.SpineControl1", "ValveBiped.Bip01_Spine1", siTrn
	CopyPaste "ValveBiped.SpineControl1", , "ValveBiped.SpineControl", 1
	Duplicate "ValveBiped.SpineControlConstrainer", , 2, 1, 1, 0, 0, 1, 0, 1
	CopyPaste "ValveBiped.SpineControlConstrainer1", , "ValveBiped.SpineControl1", 1
	MatchTransform "ValveBiped.SpineControlConstrainer1", "ValveBiped.Bip01_Spine1", siSRT
	ApplyCns "Orientation", "ValveBiped.Bip01_Spine1", "ValveBiped.SpineControlConstrainer1"
	SelectObj "ValveBiped.SpineControl1", , True
	ActivateVertexSelTool
	Translate , 0, 0, -1.04703723952745, siRelative, siGlobal, siObj, siXYZ
	ActivateObjectSelTool

		'-- SpineControl2
		
	Duplicate "ValveBiped.SpineControl1", , 2, 1, 1, 0, 0, 1, 0, 1
	Duplicate "ValveBiped.SpineControlConstrainer1", , 2, 1, 1, 0, 0, 1, 0, 1
	CopyPaste "ValveBiped.SpineControlConstrainer2", , "ValveBiped.SpineControl2", 1
	CopyPaste "ValveBiped.SpineControl2", , "ValveBiped.SpineControl1", 1
	MatchTransform "ValveBiped.SpineControl2", "ValveBiped.Bip01_Spine2", siTrn
	MatchTransform "ValveBiped.SpineControlConstrainer2", "ValveBiped.Bip01_Spine2", siSRT
	ApplyCns "Orientation", "ValveBiped.Bip01_Spine2", "ValveBiped.SpineControlConstrainer2"

		'-- SpineControl3
		
	Duplicate "ValveBiped.SpineControl2", , 2, 1, 1, 0, 0, 1, 0, 1
	Duplicate "ValveBiped.SpineControlConstrainer2", , 2, 1, 1, 0, 0, 1, 0, 1
	CopyPaste "ValveBiped.SpineControlConstrainer3", , "ValveBiped.SpineControl3", 1
	CopyPaste "ValveBiped.SpineControl3", , "ValveBiped.SpineControl2", 1
	MatchTransform "ValveBiped.SpineControl3", "ValveBiped.Bip01_Spine4", siTrn
	MatchTransform "ValveBiped.SpineControlConstrainer3", "ValveBiped.Bip01_Spine4", siSRT
	ApplyCns "Orientation", "ValveBiped.Bip01_Spine4", "ValveBiped.SpineControlConstrainer3"

		'-- NeckControl
		
	Duplicate "ValveBiped.SpineControl3", , 2, 1, 1, 0, 0, 1, 0, 1
	SetValue "ValveBiped.SpineControl4.Name", "NeckControl"
	Duplicate "ValveBiped.SpineControlConstrainer3", , 2, 1, 1, 0, 0, 1, 0, 1
	SetValue "ValveBiped.SpineControlConstrainer4.Name", "NeckControlConstrainer"
	CopyPaste "ValveBiped.NeckControlConstrainer", , "ValveBiped.NeckControl", 1
	CopyPaste "ValveBiped.NeckControl", , "ValveBiped.SpineControl3", 1
	MatchTransform "ValveBiped.NeckControl", "ValveBiped.Bip01_Neck1", siTrn
	MatchTransform "ValveBiped.NeckControlConstrainer", "ValveBiped.Bip01_Neck1", siSRT
	ApplyCns "Orientation", "ValveBiped.Bip01_Neck1", "ValveBiped.NeckControlConstrainer"
			
		'--HeadControl
		
	Duplicate "ValveBiped.NeckControl", , 2, 1, 1, 0, 0, 1, 0, 1
	SetValue "ValveBiped.NeckControl1.Name", "HeadControl"
	Duplicate "ValveBiped.NeckControlConstrainer", , 2, 1, 1, 0, 0, 1, 0, 1
	SetValue "ValveBiped.NeckControlConstrainer1.Name", "HeadControlConstrainer"
	CopyPaste "ValveBiped.HeadControl", , "ValveBiped.NeckControl", 1
	CopyPaste "ValveBiped.HeadControlConstrainer", , "ValveBiped.HeadControl", 1
	MatchTransform "ValveBiped.HeadControl", "ValveBiped.Bip01_Head1", siTrn
	MatchTransform "ValveBiped.HeadControlConstrainer", "ValveBiped.Bip01_Head1", siSRT
'	ApplyCns "Orientation", "ValveBiped.Bip01_Head1", "ValveBiped.HeadControlConstrainer"
	MatchTransform "ValveBiped.Bip01_Head1", "ValveBiped.HeadControlConstrainer", siOri

			'--Slight adjustments to control points for easier selecting	 (human specific results)
		
	SelectObj "ValveBiped.SpineControl2", , True
	ActivateVertexSelTool
	Translate , 6.79319096764948E-17, 3.32913340503841, -1.10944891743307, siRelative, siView, siObj, siXYZ
	ActivateObjectSelTool

	SelectObj "ValveBiped.NeckControl", , True
	ActivateVertexSelTool
	Translate , -9.18861341181465E-17, 4.11245891210626, 1.50101912358591, siRelative, siView, siObj, siXYZ
	ActivateObjectSelTool

	SelectObj "ValveBiped.HeadControl", , True
	ActivateVertexSelTool
	Translate , -1.03896214277265E-16, 11.097111350128, 1.69680422666234, siRelative, siView, siObj, siXYZ
	ActivateObjectSelTool


	'##########   ClavicleControl Setup

		'--ClavicleControlL
		
	GetPrim "Null"
	SetValue "null.Name", "ClavicleControlParentL"
	Duplicate "ValveBiped.HeadControl", , 2, 1, 1, 0, 0, 1, 0, 1
	SetValue "ValveBiped.HeadControl1.Name", "ClavicleControlL"
	CopyPaste "ClavicleControlParentL", , "ValveBiped.SpineControl3", 1
	CopyPaste "ValveBiped.ClavicleControlL", , "ValveBiped.ClavicleControlParentL", 1
	Refresh
	MatchTransform "ValveBiped.ClavicleControlParentL", "ValveBiped.Bip01_L_Clavicle", siSRT
	MatchTransform "ValveBiped.ClavicleControlL", "ValveBiped.Bip01_L_Clavicle", siSRT
	Refresh
	SelectObj "ValveBiped.ClavicleControlL", , True
	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.ClavicleControlL.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
	Translate , 3, 18.3075249631281, 0, siRelative, siGlobal, siObj, siXYZ
	Refresh
	ActivateObjectSelTool
	ApplyCns "Orientation", "ValveBiped.Bip01_L_Clavicle", "ValveBiped.ClavicleControlL"

		'ClavicleControlR
		
	GetPrim "Null"
	SetValue "null.Name", "ClavicleControlParentR"
	Duplicate "ValveBiped.HeadControl", , 2, 1, 1, 0, 0, 1, 0, 1
	SetValue "ValveBiped.HeadControl3.Name", "ClavicleControlR"
	CopyPaste "ClavicleControlParentR", , "ValveBiped.SpineControl3", 1
	CopyPaste "ValveBiped.ClavicleControlR", , "ValveBiped.ClavicleControlParentR", 1
	Refresh
	MatchTransform "ValveBiped.ClavicleControlParentR", "ValveBiped.Bip01_R_Clavicle", siSRT
	MatchTransform "ValveBiped.ClavicleControlR", "ValveBiped.Bip01_R_Clavicle", siSRT
	Refresh
	SelectObj "ValveBiped.ClavicleControlR", , True
	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.ClavicleControlR.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
	Translate , -4, 18.3075249631281, 0, siRelative, siGlobal, siObj, siXYZ
	Refresh
	ActivateObjectSelTool
	ApplyCns "Orientation", "ValveBiped.Bip01_R_Clavicle", "ValveBiped.ClavicleControlR"



	'######HandControl Setup

		'--LeftHand
	GetPrim "Null"
	SetValue "null.Name", "HandControlPosParentL"
	CopyPaste "HandControlPosParentL", , "ValveBiped", 1
	ApplyCns "Position", "ValveBiped.HandControlPosParentL", "ValveBiped.LShoulderEff"
		
	GetPrim "Null"
	SetValue "null.Name", "HandControlRotGrandmaL"
	CopyPaste "HandControlRotGrandmaL", , "ValveBiped", 1
	ApplyCns "Pose", "ValveBiped.HandControlRotGrandmaL", "ValveBiped.Bip01_L_Forearm"

			'--UpperArmControlL **NOPE**
			
	Refresh


			'--HandControlPosL
			
	CreatePrim "Sphere", "NurbsSurface"
	SetValue "sphere.surfmsh.geom.subdivu", 4
	SetValue "sphere.surfmsh.geom.subdivv", 2
	AddProp "Display Property"
	SetValue "sphere.display.staticsel", 0
	SetValue "sphere.display.intsel", 0
	SetValue "sphere.display.playbacksel", 0
	SetValue "sphere.display.staticunselnear", 0
	SetValue "sphere.display.intunselnear", 0
	SetValue "sphere.display.playbackunselnear", 0
	SetValue "sphere.display.staticunselfar", 0
	SetValue "sphere.display.intunselfar", 0
	SetValue "sphere.display.playbackunselfar", 0
	SetValue "sphere.Name", "HandControlPosL"
	SetValue "HandControlPosL.sphere.radius", 3
	MatchTransform "HandControlPosL", "ValveBiped.LArmEff", siTrn
	CopyPaste "HandControlPosL", , "ValveBiped.HandControlPosParentL", 1
	ApplyCns "Position", "ValveBiped.LArmEff", "ValveBiped.HandControlPosL"


			'--HandControlRotL
			
	GetPrim "Null"
	SetValue "null.Name", "HandControlRotParentL"
	MatchTransform "HandControlRotParentL", "ValveBiped.Bip01_L_Hand", siSRT
	CopyPaste "HandControlRotParentL", , "ValveBiped.HandControlRotGrandmaL", 1

	CreatePrim "Cube", "MeshSurface"
	SetValue "cube.Name", "HandControlRotL"
	MatchTransform "HandControlRotL", "ValveBiped.HandControlRotParentL", siSRT
	CopyPaste "HandControlRotL", , "ValveBiped.HandControlRotParentL", 1
	SetValue "ValveBiped.HandControlRotL.cube.length", 4
	Refresh

	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.HandControlRotL.pnt[*]"
	Translate , 1.96995415771412, 0, 0, siRelative, siParent, siObj, siXYZ
	Scale , 1, 0.585664335664336, 1, siRelative, siParent, siObj, siXYZ
	Scale , 1, 1, 1.18881118881119, siRelative, siParent, siObj, siXYZ
	ActivateObjectSelTool
	Refresh

	Duplicate "ValveBiped.HandControlRotL", , 2, 1, 1, 0, 0, 1, 0, 1
	CopyPaste "ValveBiped.HandControlRotL1", , "ValveBiped.HandControlRotL", 1
	SelectObj "ValveBiped.HandControlRotL1"
	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.HandControlRotL1.pnt[*]"
	Scale , 0.732517482517483, 0.732517482517483, 0.732517482517483, siRelative, siParent, siObj, siXYZ
	ActivateObjectSelTool
	Refresh

	GetPrim "Null"
	SetValue "null.Name", "HandControlRotConstrainerL"
	CopyPaste "HandControlRotConstrainerL", , "ValveBiped.HandControlRotL1", 1
	MatchTransform "ValveBiped.HandControlRotConstrainerL", "ValveBiped.Bip01_L_Hand", siSRT
	Refresh
	ApplyCns "Orientation", "ValveBiped.Bip01_L_Hand", "ValveBiped.HandControlRotConstrainerL"
	Refresh

		
		
		'--RightHand
		
	GetPrim "Null"
	SetValue "null.Name", "HandControlPosParentR"
	CopyPaste "HandControlPosParentR", , "ValveBiped", 1
	ApplyCns "Position", "ValveBiped.HandControlPosParentR", "ValveBiped.RShoulderEff"
		
	GetPrim "Null"
	SetValue "null.Name", "HandControlRotGrandmaR"
	CopyPaste "HandControlRotGrandmaR", , "ValveBiped", 1
	ApplyCns "Pose", "ValveBiped.HandControlRotGrandmaR", "ValveBiped.Bip01_R_Forearm"

			'--UpperArmControlR **NOPE**

			'--HandControlPosR
			
	CreatePrim "Sphere", "NurbsSurface"
	SetValue "sphere.surfmsh.geom.subdivu", 4
	SetValue "sphere.surfmsh.geom.subdivv", 2
	AddProp "Display Property"
	SetValue "sphere.display.staticsel", 0
	SetValue "sphere.display.intsel", 0
	SetValue "sphere.display.playbacksel", 0
	SetValue "sphere.display.staticunselnear", 0
	SetValue "sphere.display.intunselnear", 0
	SetValue "sphere.display.playbackunselnear", 0
	SetValue "sphere.display.staticunselfar", 0
	SetValue "sphere.display.intunselfar", 0
	SetValue "sphere.display.playbackunselfar", 0
	SetValue "sphere.Name", "HandControlPosR"
	SetValue "HandControlPosR.sphere.radius", 3
	MatchTransform "HandControlPosR", "ValveBiped.RArmEff", siTrn
	CopyPaste "HandControlPosR", , "ValveBiped.HandControlPosParentR", 1
	ApplyCns "Position", "ValveBiped.RArmEff", "ValveBiped.HandControlPosR"
	Refresh

			'--HandControlRotR
			
	GetPrim "Null"
	SetValue "null.Name", "HandControlRotParentR"
	MatchTransform "HandControlRotParentR", "ValveBiped.Bip01_R_Hand", siSRT
	CopyPaste "HandControlRotParentR", , "ValveBiped.HandControlRotGrandmaR", 1

	CreatePrim "Cube", "MeshSurface"
	SetValue "cube.Name", "HandControlRotR"
	MatchTransform "HandControlRotR", "ValveBiped.HandControlRotParentR", siSRT
	CopyPaste "HandControlRotR", , "ValveBiped.HandControlRotParentR", 1
	SetValue "ValveBiped.HandControlRotR.cube.length", 4
	Refresh

	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.HandControlRotR.pnt[*]"
	Translate , 1.96995415771412, 0, 0, siRelative, siParent, siObj, siXYZ
	Scale , 1, 0.585664335664336, 1, siRelative, siParent, siObj, siXYZ
	Scale , 1, 1, 1.18881118881119, siRelative, siParent, siObj, siXYZ
	ActivateObjectSelTool
	Refresh

	Duplicate "ValveBiped.HandControlRotR", , 2, 1, 1, 0, 0, 1, 0, 1
	CopyPaste "ValveBiped.HandControlRotR1", , "ValveBiped.HandControlRotR", 1
	SelectObj "ValveBiped.HandControlRotR1"
	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.HandControlRotR1.pnt[*]"
	Scale , 0.732517482517483, 0.732517482517483, 0.732517482517483, siRelative, siParent, siObj, siXYZ
	ActivateObjectSelTool
	Refresh

	GetPrim "Null"
	SetValue "null.Name", "HandControlRotConstrainerR"
	CopyPaste "HandControlRotConstrainerR", , "ValveBiped.HandControlRotR1", 1
	MatchTransform "ValveBiped.HandControlRotConstrainerR", "ValveBiped.Bip01_R_Hand", siSRT
	ApplyCns "Orientation", "ValveBiped.Bip01_R_Hand", "ValveBiped.HandControlRotConstrainerR"
	Refresh

	'--Kill Hidden Group

'	SelectObj "ValveBiped.hidden"
'	DeleteObj "ValveBiped.hidden"
'	SelectObj "ValveBiped.hidden_distractions"
'	DeleteObj "ValveBiped.hidden_distractions"

	'--New ArmRollL

	SelectObj "ValveBiped.lArmUpVParent"
	CutObj
	CutObj
	CreatePrim "Sphere", "NurbsSurface"
	SetValue "sphere.sphere.radius", 1
	SetValue "sphere.surfmsh.geom.subdivu", 4
	SetValue "sphere.surfmsh.geom.subdivv", 2
	MatchTransform "sphere", "ValveBiped.ArmRollL", siTrn
	SelectObj "ValveBiped.ArmRollL"
	DeleteObj "ValveBiped.ArmRollL"
	SelectObj "sphere"
	SetValue "sphere.Name", "ArmRollL"
	CopyPaste "ArmRollL", , "ValveBiped.HandControlPosParentL", 1
	SelectObj "lArmUpVParent"
	CopyPaste "lArmUpVParent", , "ValveBiped.ArmRollL", 1
	SelectObj "ValveBiped.ArmRollL"
	SetSelFilter "Point"
	SelectAllUsingFilter "Point", siCheckComponentVisibility
	CreateClusterCenterWithNull "ValveBiped.ArmRollL.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
	ActivateObjectSelTool
	DeselectAll
	SelectObj "ValveBiped.ArmRollL_Point_ClsCtr"
	MatchTransform "ValveBiped.ArmRollL_Point_ClsCtr", "ValveBiped.HandControlPosL", siTrn
	Translate , 0, 9, 0, siRelative, siView, siObj, siXYZ
	CutObj
	CutObj
	CopyPaste "ArmRollL_Point_ClsCtr", , "ValveBiped.HandControlPosL", 1
'	InspectObj "ValveBiped.ArmRollL_Point_ClsCtr.kine.local"
	SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.cnsori", False
	SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.cnsscl", False
	SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.affbyori", False
	SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.affbyscl", False

	'--New ArmRollR

	SelectObj "ValveBiped.lArmUpVParent1"
	CutObj
	CutObj
	CreatePrim "Sphere", "NurbsSurface"
	SetValue "sphere.sphere.radius", 1
	SetValue "sphere.surfmsh.geom.subdivu", 4
	SetValue "sphere.surfmsh.geom.subdivv", 2
	MatchTransform "sphere", "ValveBiped.ArmRollR", siTrn
	SelectObj "ValveBiped.ArmRollR"
	DeleteObj "ValveBiped.ArmRollR"
	SelectObj "sphere"
	SetValue "sphere.Name", "ArmRollR"
	CopyPaste "ArmRollR", , "ValveBiped.HandControlPosParentR", 1
	SelectObj "lArmUpVParent1"
	SelectObj "lArmUpVParent1"
	ParentObj "B:ValveBiped.ArmRollR", "lArmUpVParent1"
	SelectObj "B:ValveBiped.ArmRollR"
	SelectObj "ValveBiped.ArmRollR"
	SetSelFilter "Point"
	SelectAllUsingFilter "Point", siCheckComponentVisibility
	CreateClusterCenterWithNull "ValveBiped.ArmRollR.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
	ActivateObjectSelTool
	DeselectAll
	SelectObj "ValveBiped.ArmRollR_Point_ClsCtr"
	MatchTransform "ValveBiped.ArmRollR_Point_ClsCtr", "ValveBiped.HandControlPosR", siTrn
	Translate , 0, 9, 0, siRelative, siView, siObj, siXYZ
	CutObj
	CutObj
	CopyPaste "ArmRollR_Point_ClsCtr", , "ValveBiped.HandControlPosR", 1
'	InspectObj "ValveBiped.ArmRollR_Point_ClsCtr.kine.local"
	SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.cnsori", False
	SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.cnsscl", False
	SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.affbyori", False
	SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.affbyscl", False

	'-- New LLegRoll

	SelectObj "ValveBiped.LUpVParent"
	CutObj
	CutObj
	CreatePrim "Sphere", "NurbsSurface"
	SetValue "sphere.sphere.radius", 1
	SetValue "sphere.surfmsh.geom.subdivu", 4
	SetValue "sphere.surfmsh.geom.subdivv", 2
	MatchTransform "sphere", "ValveBiped.LegRollL", siTrn
	SelectObj "ValveBiped.LegRollL"
	DeleteObj "ValveBiped.LegRollL"
	SetValue "sphere.Name", "LegRollL"
	GetPrim "Null"
	SetValue "null.Name", "LegRollControlParent_L"
	MatchTransform "LegRollControlParent_L", "LegRollL", siTrn
	SelectObj "LegRollL"
	CopyPaste "LegRollL", , "LegRollControlParent_L", 1
	SelectObj "LUpVParent"
	CopyPaste "LUpVParent", , "LegRollL", 1
	SelectObj "LegRollL", , True
	SetSelFilter "Point"
	SelectAllUsingFilter "Point", siCheckComponentVisibility
	CreateClusterCenterWithNull "LegRollL.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
	ActivateObjectSelTool
	DeselectAll
	SelectObj "LegRollControlParent_L"
	CopyPaste "LegRollControlParent_L", , "ValveBiped", 1
	SelectObj "ValveBiped.LegRollL_Point_ClsCtr"
	MatchTransform "ValveBiped.LegRollL_Point_ClsCtr", "ValveBiped.LeftFoot", siTrn
	Translate , 8, 0, 0, siRelative, siLocal, siObj, siX
	CutObj
	CutObj
	CopyPaste "LegRollL_Point_ClsCtr", , "ValveBiped.LeftFoot", 1
'	InspectObj "ValveBiped.LegRollL_Point_ClsCtr.kine.local"
	SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.cnsori", False
	SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.cnsscl", False
	SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.affbyori", False
	SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.affbyscl", False

	'-- New RLegRoll

	SelectObj "ValveBiped.RUpVParent"
	CutObj
	CutObj
	CreatePrim "Sphere", "NurbsSurface"
	SetValue "sphere.sphere.radius", 1
	SetValue "sphere.surfmsh.geom.subdivu", 4
	SetValue "sphere.surfmsh.geom.subdivv", 2
	MatchTransform "sphere", "ValveBiped.LegRollR", siTrn
	SelectObj "ValveBiped.LegRollR"
	DeleteObj "ValveBiped.LegRollR"
	SetValue "sphere.Name", "LegRollR"
	GetPrim "Null"
	SetValue "null.Name", "LegRollControlParent_R"
	MatchTransform "LegRollControlParent_R", "LegRollR", siTrn
	SelectObj "LegRollR"
	CopyPaste "LegRollR", , "LegRollControlParent_R", 1
	SelectObj "RUpVParent"
	CopyPaste "RUpVParent", , "LegRollR", 1
	SelectObj "LegRollR", , True
	SetSelFilter "Point"
	SelectAllUsingFilter "Point", siCheckComponentVisibility
	CreateClusterCenterWithNull "LegRollR.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
	ActivateObjectSelTool
	DeselectAll
	SelectObj "LegRollControlParent_R"
	CopyPaste "LegRollControlParent_R", , "ValveBiped", 1
	SelectObj "ValveBiped.LegRollR_Point_ClsCtr"
	MatchTransform "ValveBiped.LegRollR_Point_ClsCtr", "ValveBiped.RightFoot", siTrn
	Translate , -8, 0, 0, siRelative, siRelative, siLocal, siObj, siX
	CutObj
	CutObj
	CopyPaste "LegRollR_Point_ClsCtr", , "ValveBiped.RightFoot", 1
'	InspectObj "ValveBiped.LegRollR_Point_ClsCtr.kine.local"
	SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.cnsori", False
	SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.cnsscl", False
	SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.affbyori", False
	SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.affbyscl", False
	DeselectAll

	'-- constrain LegRollControlParents to LegRoots

	SelectObj "ValveBiped.LegRollControlParent_L"
	ApplyCns "Position", "ValveBiped.LegRollControlParent_L", "ValveBiped.LLegRoot"
	SelectObj "ValveBiped.LegRollControlParent_R"
	ApplyCns "Position", "ValveBiped.LegRollControlParent_R", "ValveBiped.RLegRoot"
	DeselectAll


	'-- cosmetic changes

	SelectObj "ValveBiped.SpineControl", , True
	AddToSelection "ValveBiped.SpineControl,ValveBiped.SpineControl1", , True
	AddToSelection "ValveBiped.SpineControl2", , True
	AddToSelection "ValveBiped.SpineControl3", , True
	AddToSelection "ValveBiped.NeckControl", , True
	AddToSelection "ValveBiped.HeadControl", , True
	SetValue "ValveBiped.SpineControl.sphere.radius,ValveBiped.SpineControl1.sphere.radius,ValveBiped.SpineControl2.sphere.radius,ValveBiped.SpineControl3.sphere.radius,ValveBiped.NeckControl.sphere.radius,ValveBiped.HeadControl.sphere.radius", Array(1.5, 1.5, 1.5, 1.5, 1.5, 1.5)

	SelectObj "ValveBiped.ClavicleControlL", , True
	AddToSelection "ValveBiped.ClavicleControlR", , True
	SetValue "ValveBiped.ClavicleControlL.sphere.radius,ValveBiped.ClavicleControlR.sphere.radius", Array(2, 2)

	SelectObj "ValveBiped.HandControlRotL", , True
	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.HandControlRotL.pnt[1,3,5,LAST]"
	Translate , 1.87375999944803, 0, 0, siRelative, siParent, siObj, siXYZ
	ActivateObjectSelTool
	SelectObj "ValveBiped.HandControlRotR", , True
	ActivateVertexSelTool
	SelectGeometryComponents "ValveBiped.HandControlRotR.pnt[1,3,5,LAST]"
	Translate , 1.87375999944803, 0, 0, siRelative, siParent, siObj, siXYZ
	ActivateObjectSelTool
	DeselectAll

	'--rotation controls group **DELETED**


	'-- deforming bones group

	SelectObj "ValveBiped.Bip01", "BRANCH"
	SelectChildNodes
	CreateGroup
	SetValue "Group.Name", "Envelope_Deformers"
	CopyPaste "Envelope_Deformers", , "ValveBiped", 1

	DeselectAll

	'-- all animateable controls group **DELETED, REDUNDANT WITH PLOTTED GROUP**

	'--spine and head controls, display and marking sets

	AddToSelection "ValveBiped.SpineControl", , True
	AddToSelection "ValveBiped.SpineControl1", , True
	AddToSelection "ValveBiped.SpineControl2", , True
	AddToSelection "ValveBiped.SpineControl3", , True
	AddToSelection "ValveBiped.NeckControl", , True
	AddToSelection "ValveBiped.HeadControl", , True

	SetValue "ValveBiped.SpineControl.nodelocal.display.staticsel,ValveBiped.SpineControl1.nodelocal.display.staticsel,ValveBiped.SpineControl2.nodelocal.display.staticsel,ValveBiped.SpineControl3.nodelocal.display.staticsel,ValveBiped.NeckControl.nodelocal.display.staticsel,ValveBiped.HeadControl.nodelocal.display.staticsel", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.intsel,ValveBiped.SpineControl1.nodelocal.display.intsel,ValveBiped.SpineControl2.nodelocal.display.intsel,ValveBiped.SpineControl3.nodelocal.display.intsel,ValveBiped.NeckControl.nodelocal.display.intsel,ValveBiped.HeadControl.nodelocal.display.intsel", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.playbacksel,ValveBiped.SpineControl1.nodelocal.display.playbacksel,ValveBiped.SpineControl2.nodelocal.display.playbacksel,ValveBiped.SpineControl3.nodelocal.display.playbacksel,ValveBiped.NeckControl.nodelocal.display.playbacksel,ValveBiped.HeadControl.nodelocal.display.playbacksel", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.staticunselnear,ValveBiped.SpineControl1.nodelocal.display.staticunselnear,ValveBiped.SpineControl2.nodelocal.display.staticunselnear,ValveBiped.SpineControl3.nodelocal.display.staticunselnear,ValveBiped.NeckControl.nodelocal.display.staticunselnear,ValveBiped.HeadControl.nodelocal.display.staticunselnear", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.intunselnear,ValveBiped.SpineControl1.nodelocal.display.intunselnear,ValveBiped.SpineControl2.nodelocal.display.intunselnear,ValveBiped.SpineControl3.nodelocal.display.intunselnear,ValveBiped.NeckControl.nodelocal.display.intunselnear,ValveBiped.HeadControl.nodelocal.display.intunselnear", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.playbackunselnear,ValveBiped.SpineControl1.nodelocal.display.playbackunselnear,ValveBiped.SpineControl2.nodelocal.display.playbackunselnear,ValveBiped.SpineControl3.nodelocal.display.playbackunselnear,ValveBiped.NeckControl.nodelocal.display.playbackunselnear,ValveBiped.HeadControl.nodelocal.display.playbackunselnear", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.staticunselfar,ValveBiped.SpineControl1.nodelocal.display.staticunselfar,ValveBiped.SpineControl2.nodelocal.display.staticunselfar,ValveBiped.SpineControl3.nodelocal.display.staticunselfar,ValveBiped.NeckControl.nodelocal.display.staticunselfar,ValveBiped.HeadControl.nodelocal.display.staticunselfar", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.intunselfar,ValveBiped.SpineControl1.nodelocal.display.intunselfar,ValveBiped.SpineControl2.nodelocal.display.intunselfar,ValveBiped.SpineControl3.nodelocal.display.intunselfar,ValveBiped.NeckControl.nodelocal.display.intunselfar,ValveBiped.HeadControl.nodelocal.display.intunselfar", Array(0, 0, 0, 0, 0, 0)
	SetValue "ValveBiped.SpineControl.nodelocal.display.playbackunselfar,ValveBiped.SpineControl1.nodelocal.display.playbackunselfar,ValveBiped.SpineControl2.nodelocal.display.playbackunselfar,ValveBiped.SpineControl3.nodelocal.display.playbackunselfar,ValveBiped.NeckControl.nodelocal.display.playbackunselfar,ValveBiped.HeadControl.nodelocal.display.playbackunselfar", Array(0, 0, 0, 0, 0, 0)

	SetValue "ValveBiped.HeadControl.display.wirecol,ValveBiped.NeckControl.display.wirecol,ValveBiped.SpineControl3.display.wirecol,ValveBiped.SpineControl2.display.wirecol,ValveBiped.SpineControl1.display.wirecol,ValveBiped.SpineControl.display.wirecol", Array(433, 433, 433, 433, 433, 433)


	AddToMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	CreateMarkingSet "ValveBiped.SpineControl,ValveBiped.SpineControl1,ValveBiped.SpineControl2,ValveBiped.SpineControl3,ValveBiped.NeckControl,ValveBiped.HeadControl", False

	DeselectAll

	'--left arm controls display and marking sets

	SelectObj "ValveBiped.HandControlPosL", , True
	AddToSelection "ValveBiped.HandControlRotL1", , True
	AddToSelection "ValveBiped.HandControlRotL", , True
	AddToSelection "ValveBiped.ArmRollL", , True
	AddToSelection "ValveBiped.ClavicleControlL", , True

	AddProp "Display Property"

	SetValue "ValveBiped.HandControlPosL.display.staticsel,ValveBiped.HandControlRotL1.display.staticsel,ValveBiped.HandControlRotL.display.staticsel,ValveBiped.ArmRollL.display.staticsel,ValveBiped.ClavicleControlL.display.staticsel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.intsel,ValveBiped.HandControlRotL1.display.intsel,ValveBiped.HandControlRotL.display.intsel,ValveBiped.ArmRollL.display.intsel,ValveBiped.ClavicleControlL.display.intsel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.playbacksel,ValveBiped.HandControlRotL1.display.playbacksel,ValveBiped.HandControlRotL.display.playbacksel,ValveBiped.ArmRollL.display.playbacksel,ValveBiped.ClavicleControlL.display.playbacksel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.staticunselnear,ValveBiped.HandControlRotL1.display.staticunselnear,ValveBiped.HandControlRotL.display.staticunselnear,ValveBiped.ArmRollL.display.staticunselnear,ValveBiped.ClavicleControlL.display.staticunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.intunselnear,ValveBiped.HandControlRotL1.display.intunselnear,ValveBiped.HandControlRotL.display.intunselnear,ValveBiped.ArmRollL.display.intunselnear,ValveBiped.ClavicleControlL.display.intunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.playbackunselnear,ValveBiped.HandControlRotL1.display.playbackunselnear,ValveBiped.HandControlRotL.display.playbackunselnear,ValveBiped.ArmRollL.display.playbackunselnear,ValveBiped.ClavicleControlL.display.playbackunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.staticunselfar,ValveBiped.HandControlRotL1.display.staticunselfar,ValveBiped.HandControlRotL.display.staticunselfar,ValveBiped.ArmRollL.display.staticunselfar,ValveBiped.ClavicleControlL.display.staticunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.intunselfar,ValveBiped.HandControlRotL1.display.intunselfar,ValveBiped.HandControlRotL.display.intunselfar,ValveBiped.ArmRollL.display.intunselfar,ValveBiped.ClavicleControlL.display.intunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.playbackunselfar,ValveBiped.HandControlRotL1.display.playbackunselfar,ValveBiped.HandControlRotL.display.playbackunselfar,ValveBiped.ArmRollL.display.playbackunselfar,ValveBiped.ClavicleControlL.display.playbackunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.HandControlPosL.display.wirecol,ValveBiped.HandControlRotL1.display.wirecol,ValveBiped.HandControlRotL.display.wirecol,ValveBiped.ArmRollL.display.wirecol,ValveBiped.ClavicleControlL.display.wirecol", Array(367, 367, 367, 367, 367)

	DeselectAll

	SelectObj "ValveBiped.ClavicleControlL", , True
	ToggleSelection "ValveBiped.ArmRollL", , True
	ToggleSelection "ValveBiped.HandControlRotL", , True
	ToggleSelection "ValveBiped.HandControlRotL1", , True
	SetMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	CreateMarkingSet "ValveBiped.ClavicleControlL,ValveBiped.ArmRollL,ValveBiped.HandControlRotL,ValveBiped.HandControlRotL1"

	SelectObj "ValveBiped.HandControlPosL", , True
	AddToSelection "ValveBiped.lArmUpV", , True
	SetMarking "kine.local.pos.posx"
	AddToMarking "kine.local.pos.posy"
	AddToMarking "kine.local.pos.posz"
	CreateMarkingSet "ValveBiped.HandControlPosL,ValveBiped.lArmUpV", False
	DeselectAll

	'--left leg control Display and marking sets

	SelectObj "ValveBiped.LegRollL", , True
	AddToSelection "ValveBiped.LeftFoot", , True
	AddToSelection "ValveBiped.LeftRoll", , True
	AddToSelection "ValveBiped.LeftToe", , True
	AddProp "Display Property"

	SetValue "ValveBiped.LegRollL.display.staticsel,ValveBiped.LeftFoot.display.staticsel,ValveBiped.LeftRoll.display.staticsel,ValveBiped.LeftToe.display.staticsel", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.intsel,ValveBiped.LeftFoot.display.intsel,ValveBiped.LeftRoll.display.intsel,ValveBiped.LeftToe.display.intsel", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.playbacksel,ValveBiped.LeftFoot.display.playbacksel,ValveBiped.LeftRoll.display.playbacksel,ValveBiped.LeftToe.display.playbacksel", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.staticunselnear,ValveBiped.LeftFoot.display.staticunselnear,ValveBiped.LeftRoll.display.staticunselnear,ValveBiped.LeftToe.display.staticunselnear", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.intunselnear,ValveBiped.LeftFoot.display.intunselnear,ValveBiped.LeftRoll.display.intunselnear,ValveBiped.LeftToe.display.intunselnear", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.playbackunselnear,ValveBiped.LeftFoot.display.playbackunselnear,ValveBiped.LeftRoll.display.playbackunselnear,ValveBiped.LeftToe.display.playbackunselnear", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.staticunselfar,ValveBiped.LeftFoot.display.staticunselfar,ValveBiped.LeftRoll.display.staticunselfar,ValveBiped.LeftToe.display.staticunselfar", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.intunselfar,ValveBiped.LeftFoot.display.intunselfar,ValveBiped.LeftRoll.display.intunselfar,ValveBiped.LeftToe.display.intunselfar", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.playbackunselfar,ValveBiped.LeftFoot.display.playbackunselfar,ValveBiped.LeftRoll.display.playbackunselfar,ValveBiped.LeftToe.display.playbackunselfar", Array(0, 0, 0, 0)
	SetValue "ValveBiped.LegRollL.display.wirecol,ValveBiped.LeftFoot.display.wirecol,ValveBiped.LeftRoll.display.wirecol,ValveBiped.LeftToe.display.wirecol", Array(94, 94, 94, 94)
	DeselectAll


	SelectObj "ValveBiped.LegRollL", , True
	ToggleSelection "ValveBiped.LeftToe", , True
	SetMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	CreateMarkingSet "ValveBiped.LegRollL"


	DeselectAll

	'--right arm control Display and Marking sets

	SelectObj "ValveBiped.ClavicleControlR", , True
	AddToSelection "ValveBiped.ArmRollR", , True
	AddToSelection "ValveBiped.HandControlPosR", , True
	AddToSelection "ValveBiped.HandControlRotR", , True
	AddToSelection "ValveBiped.HandControlRotR1", , True
	AddProp "Display Property"

	SetValue "ValveBiped.ClavicleControlR.display.staticsel,ValveBiped.ArmRollR.display.staticsel,ValveBiped.HandControlPosR.display.staticsel,ValveBiped.HandControlRotR.display.staticsel,ValveBiped.HandControlRotR1.display.staticsel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.intsel,ValveBiped.ArmRollR.display.intsel,ValveBiped.HandControlPosR.display.intsel,ValveBiped.HandControlRotR.display.intsel,ValveBiped.HandControlRotR1.display.intsel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.playbacksel,ValveBiped.ArmRollR.display.playbacksel,ValveBiped.HandControlPosR.display.playbacksel,ValveBiped.HandControlRotR.display.playbacksel,ValveBiped.HandControlRotR1.display.playbacksel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.staticunselnear,ValveBiped.ArmRollR.display.staticunselnear,ValveBiped.HandControlPosR.display.staticunselnear,ValveBiped.HandControlRotR.display.staticunselnear,ValveBiped.HandControlRotR1.display.staticunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.intunselnear,ValveBiped.ArmRollR.display.intunselnear,ValveBiped.HandControlPosR.display.intunselnear,ValveBiped.HandControlRotR.display.intunselnear,ValveBiped.HandControlRotR1.display.intunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.playbackunselnear,ValveBiped.ArmRollR.display.playbackunselnear,ValveBiped.HandControlPosR.display.playbackunselnear,ValveBiped.HandControlRotR.display.playbackunselnear,ValveBiped.HandControlRotR1.display.playbackunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.staticunselfar,ValveBiped.ArmRollR.display.staticunselfar,ValveBiped.HandControlPosR.display.staticunselfar,ValveBiped.HandControlRotR.display.staticunselfar,ValveBiped.HandControlRotR1.display.staticunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.intunselfar,ValveBiped.ArmRollR.display.intunselfar,ValveBiped.HandControlPosR.display.intunselfar,ValveBiped.HandControlRotR.display.intunselfar,ValveBiped.HandControlRotR1.display.intunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.playbackunselfar,ValveBiped.ArmRollR.display.playbackunselfar,ValveBiped.HandControlPosR.display.playbackunselfar,ValveBiped.HandControlRotR.display.playbackunselfar,ValveBiped.HandControlRotR1.display.playbackunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.ClavicleControlR.display.wirecol,ValveBiped.ArmRollR.display.wirecol,ValveBiped.HandControlPosR.display.wirecol,ValveBiped.HandControlRotR.display.wirecol,ValveBiped.HandControlRotR1.display.wirecol", Array(13, 13, 13, 13, 13)
	DeselectAll


	SelectObj "ValveBiped.ClavicleControlR", , True
	AddToSelection "ValveBiped.ArmRollR", , True
	AddToSelection "ValveBiped.HandControlRotR1", , True
	AddToSelection "ValveBiped.HandControlRotR", , True
	SetMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	CreateMarkingSet "ValveBiped.ClavicleControlR,ValveBiped.ArmRollR,ValveBiped.HandControlRotR1,ValveBiped.HandControlRotR"

	SelectObj "ValveBiped.HandControlPosR", , True
	SetMarking "kine.local.pos.posx"
	AddToMarking "kine.local.pos.posy"
	AddToMarking "kine.local.pos.posz"
	CreateMarkingSet "ValveBiped.HandControlPosR", False
	DeselectAll

	SelectObj "ValveBiped.rArmUpV", , True
	SetMarking "kine.local.pos.posx"
	AddToMarking "kine.local.pos.posy"
	AddToMarking "kine.local.pos.posz"
	CreateMarkingSet "ValveBiped.rArmUpV"



	'--right leg controls display and marking sets
	SelectObj "ValveBiped.RightFoot", , True
	AddToSelection "ValveBiped.RightRoll", , True
	AddToSelection "ValveBiped.RightToe", , True
	AddToSelection "ValveBiped.LegRollR", , True
	AddProp "Display Property"

	SetValue "ValveBiped.RightFoot.display.staticsel,ValveBiped.LegRollR_Point_ClsCtr.display.staticsel,ValveBiped.LegRollR.display.staticsel,ValveBiped.RightRoll.display.staticsel,ValveBiped.RightToe.display.staticsel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.intsel,ValveBiped.LegRollR_Point_ClsCtr.display.intsel,ValveBiped.LegRollR.display.intsel,ValveBiped.RightRoll.display.intsel,ValveBiped.RightToe.display.intsel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.playbacksel,ValveBiped.LegRollR_Point_ClsCtr.display.playbacksel,ValveBiped.LegRollR.display.playbacksel,ValveBiped.RightRoll.display.playbacksel,ValveBiped.RightToe.display.playbacksel", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.staticunselnear,ValveBiped.LegRollR_Point_ClsCtr.display.staticunselnear,ValveBiped.LegRollR.display.staticunselnear,ValveBiped.RightRoll.display.staticunselnear,ValveBiped.RightToe.display.staticunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.intunselnear,ValveBiped.LegRollR_Point_ClsCtr.display.intunselnear,ValveBiped.LegRollR.display.intunselnear,ValveBiped.RightRoll.display.intunselnear,ValveBiped.RightToe.display.intunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.playbackunselnear,ValveBiped.LegRollR_Point_ClsCtr.display.playbackunselnear,ValveBiped.LegRollR.display.playbackunselnear,ValveBiped.RightRoll.display.playbackunselnear,ValveBiped.RightToe.display.playbackunselnear", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.staticunselfar,ValveBiped.LegRollR_Point_ClsCtr.display.staticunselfar,ValveBiped.LegRollR.display.staticunselfar,ValveBiped.RightRoll.display.staticunselfar,ValveBiped.RightToe.display.staticunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.intunselfar,ValveBiped.LegRollR_Point_ClsCtr.display.intunselfar,ValveBiped.LegRollR.display.intunselfar,ValveBiped.RightRoll.display.intunselfar,ValveBiped.RightToe.display.intunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.playbackunselfar,ValveBiped.LegRollR_Point_ClsCtr.display.playbackunselfar,ValveBiped.LegRollR.display.playbackunselfar,ValveBiped.RightRoll.display.playbackunselfar,ValveBiped.RightToe.display.playbackunselfar", Array(0, 0, 0, 0, 0)
	SetValue "ValveBiped.RightFoot.display.wirecol,ValveBiped.LegRollR_Point_ClsCtr.display.wirecol,ValveBiped.LegRollR.display.wirecol,ValveBiped.RightRoll.display.wirecol,ValveBiped.RightToe.display.wirecol", Array(11, 11, 11, 11, 11)
	DeselectAll

	SelectObj "ValveBiped.LegRollR", , True
	SetMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	CreateMarkingSet "ValveBiped.LegRollR", False

	SelectObj "ValveBiped.RightFoot", , True
	SetMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	AddToMarking "kine.local.pos.posx"
	AddToMarking "kine.local.pos.posy"
	AddToMarking "kine.local.pos.posz"
	CreateMarkingSet "ValveBiped.RightFoot", False

	SelectObj "ValveBiped.RightRoll", , True
	SetMarking "kine.local.ori.euler.rotx"
	CreateMarkingSet "ValveBiped.RightRoll", False


	'--marking sets for pelvis control

	SelectObj "ValveBiped.Bip01_Pelvis", , True
	SetMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	CreateMarkingSet "ValveBiped.Bip01_Pelvis", False
	DeselectAll

	'--hidden distracting objects

	CreateGroup
	SetValue "Group.Name", "hidden_distractions"
	SetValue "hidden_distractions.viewvis", 0
	CopyPaste "hidden_distractions", , "ValveBiped", 1
	AddToSelection "ValveBiped.Bip01,ValveBiped.LeftFootParent,ValveBiped.LfootUpVector,ValveBiped.LfootUpVector2,ValveBiped.spacer,ValveBiped.heel,ValveBiped.toe,ValveBiped.mid,ValveBiped.ankle,ValveBiped.LegRollL_Point_ClsCtr,ValveBiped.RightFootParent,ValveBiped.spacer1,ValveBiped.heel1,ValveBiped.toe1,ValveBiped.mid1,ValveBiped.ankle1,ValveBiped.RfootUpVector2,ValveBiped.RfootUpVector1,ValveBiped.LegRollR_Point_ClsCtr,ValveBiped.UpperBodyParent,ValveBiped.SpineControlParent,ValveBiped.SpineControlConstrainer,ValveBiped.SpineControlConstrainer1,ValveBiped.SpineControlConstrainer2,ValveBiped.SpineControlConstrainer3,ValveBiped.NeckControlConstrainer,ValveBiped.HeadControlConstrainer,ValveBiped.ClavicleControlParentL,ValveBiped.ClavicleControlParentR,ValveBiped.HandControlPosParentL,ValveBiped.ArmRollL_Point_ClsCtr,ValveBiped.HandControlRotGrandmaL,ValveBiped.HandControlRotParentL,ValveBiped.HandControlRotConstrainerL,ValveBiped.HandControlPosParentR,ValveBiped.ArmRollR_Point_ClsCtr,ValveBiped.HandControlRotGrandmaR,ValveBiped.HandControlRotParentR,ValveBiped.HandControlRotConstrainerR,ValveBiped.LegRollControlParent_L,ValveBiped.LegRollControlParent_R", , True
	SIAddToGroup "ValveBiped.hidden_distractions", "ValveBiped.Bip01,ValveBiped.LeftFootParent,ValveBiped.LfootUpVector,ValveBiped.LfootUpVector2,ValveBiped.spacer,ValveBiped.heel,ValveBiped.toe,ValveBiped.mid,ValveBiped.ankle,ValveBiped.LegRollL_Point_ClsCtr,ValveBiped.RightFootParent,ValveBiped.spacer1,ValveBiped.heel1,ValveBiped.toe1,ValveBiped.mid1,ValveBiped.ankle1,ValveBiped.RfootUpVector2,ValveBiped.RfootUpVector1,ValveBiped.LegRollR_Point_ClsCtr,ValveBiped.UpperBodyParent,ValveBiped.SpineControlParent,ValveBiped.SpineControlConstrainer,ValveBiped.SpineControlConstrainer1,ValveBiped.SpineControlConstrainer2,ValveBiped.SpineControlConstrainer3,ValveBiped.NeckControlConstrainer,ValveBiped.HeadControlConstrainer,ValveBiped.ClavicleControlParentL,ValveBiped.ClavicleControlParentR,ValveBiped.HandControlPosParentL,ValveBiped.ArmRollL_Point_ClsCtr,ValveBiped.HandControlRotGrandmaL,ValveBiped.HandControlRotParentL,ValveBiped.HandControlRotConstrainerL,ValveBiped.HandControlPosParentR,ValveBiped.ArmRollR_Point_ClsCtr,ValveBiped.HandControlRotGrandmaR,ValveBiped.HandControlRotParentR,ValveBiped.HandControlRotConstrainerR,ValveBiped.LegRollControlParent_L,ValveBiped.LegRollControlParent_R"
	SelectObj "ValveBiped.spacer3", , True
	AddToSelection "ValveBiped.LeftToeTarget", , True
	AddToSelection "ValveBiped.spacer4", , True
	AddToSelection "ValveBiped.RightToeTarget", , True
	AddToSelection "ValveBiped.RToeEff", , True
	AddToSelection "ValveBiped.LToeEff", , True
	AddToSelection "ValveBiped.LFootEff,ValveBiped.LRootFoot", , True
	AddToSelection "ValveBiped.RFootEff,ValveBiped.RRootFoot", , True
	AddToSelection "ValveBiped.RLegRoot", , True
	AddToSelection "ValveBiped.LLegRoot", , True
	AddToSelection "ValveBiped.Spine_Root", , True
	AddToSelection "ValveBiped.RRootShoulder", , True
	AddToSelection "ValveBiped.LRootShoulder", , True
	AddToSelection "ValveBiped.eff,ValveBiped.Neck1", , True
	AddToSelection "ValveBiped.eff1", , True
	AddToSelection "ValveBiped.LArmRoot", , True
	AddToSelection "ValveBiped.LShoulderEff", , True
	AddToSelection "ValveBiped.RShoulderEff,ValveBiped.RArmRoot", , True
	AddToSelection "ValveBiped.LArmEff,ValveBiped.LHandRoot", , True
	AddToSelection "ValveBiped.LThumbRoot", , True
	AddToSelection "ValveBiped.LIndexRoot", , True
	AddToSelection "ValveBiped.LHandEffector", , True
	AddToSelection "ValveBiped.LRingRoot", , True
	AddToSelection "ValveBiped.LPinkyRoot", , True
	AddToSelection "ValveBiped.LHandEffector,ValveBiped.LMiddleRoot", , True
	AddToSelection "ValveBiped.RArmEff,ValveBiped.RHandRoot", , True
	AddToSelection "ValveBiped.RThumbRoot", , True
	AddToSelection "ValveBiped.RIndexRoot", , True
	AddToSelection "ValveBiped.RHandEffector,ValveBiped.RMiddleRoot", , True
	AddToSelection "ValveBiped.RRingRoot", , True
	AddToSelection "ValveBiped.RPinkyRoot", , True
	SIAddToGroup "ValveBiped.hidden_distractions", "ValveBiped.spacer3,ValveBiped.LeftToeTarget,ValveBiped.spacer4,ValveBiped.RightToeTarget,ValveBiped.RToeEff,ValveBiped.LToeEff,ValveBiped.LFootEff,ValveBiped.LRootFoot,ValveBiped.RFootEff,ValveBiped.RRootFoot,ValveBiped.RLegRoot,ValveBiped.LLegRoot,ValveBiped.Spine,ValveBiped.RRootShoulder,ValveBiped.LRootShoulder,ValveBiped.eff,ValveBiped.Neck1,ValveBiped.eff1,ValveBiped.LArmRoot,ValveBiped.LShoulderEff,ValveBiped.RShoulderEff,ValveBiped.RArmRoot,ValveBiped.LArmEff,ValveBiped.LHandRoot,ValveBiped.LThumbRoot,ValveBiped.LIndexRoot,ValveBiped.LHandEffector,ValveBiped.LRingRoot,ValveBiped.LPinkyRoot,ValveBiped.LMiddleRoot,ValveBiped.RArmEff,ValveBiped.RHandRoot,ValveBiped.RThumbRoot,ValveBiped.RIndexRoot,ValveBiped.RHandEffector,ValveBiped.RMiddleRoot,ValveBiped.RRingRoot,ValveBiped.RPinkyRoot"
	DeselectAll

	'--unselectable group **NO,THANKS.  I HAVE CLEARANCE TO SELECT THINGS**

	'--add expression constants to controls

	AddExpr "ValveBiped.SpineControl.kine.local.posx"
	AddExpr "ValveBiped.SpineControl2.kine.local.posx"
	AddExpr "ValveBiped.SpineControl1.kine.local.posx"
	AddExpr "ValveBiped.SpineControl3.kine.local.posx"
	AddExpr "ValveBiped.NeckControl.kine.local.posx"
	AddExpr "ValveBiped.HeadControl.kine.local.posx"
	AddExpr "ValveBiped.SpineControl.kine.local.posy"
	AddExpr "ValveBiped.SpineControl2.kine.local.posy"
	AddExpr "ValveBiped.SpineControl1.kine.local.posy"
	AddExpr "ValveBiped.SpineControl3.kine.local.posy"
	AddExpr "ValveBiped.NeckControl.kine.local.posy"
	AddExpr "ValveBiped.HeadControl.kine.local.posy"
	AddExpr "ValveBiped.SpineControl.kine.local.posz"
	AddExpr "ValveBiped.SpineControl2.kine.local.posz"
	AddExpr "ValveBiped.SpineControl1.kine.local.posz"
	AddExpr "ValveBiped.SpineControl3.kine.local.posz"
	AddExpr "ValveBiped.NeckControl.kine.local.posz"
	AddExpr "ValveBiped.HeadControl.kine.local.posz"
	AddExpr "ValveBiped.ArmRollL.kine.local.posx"
	AddExpr "ValveBiped.HandControlRotL.kine.local.posx"
	AddExpr "ValveBiped.HandControlRotL1.kine.local.posx"
	AddExpr "ValveBiped.ClavicleControlR.kine.local.posx"
	AddExpr "ValveBiped.HandControlRotR.kine.local.posx"
	AddExpr "ValveBiped.HandControlRotR1.kine.local.posx"
	AddExpr "ValveBiped.ArmRollR.kine.local.posx"
	AddExpr "ValveBiped.LegRollL.kine.local.posx"
	AddExpr "ValveBiped.LegRollR.kine.local.posx"
	AddExpr "ValveBiped.Bip01_Pelvis.kine.local.posx"
	AddExpr "ValveBiped.ClavicleControlL.kine.local.posy"
	AddExpr "ValveBiped.ArmRollL.kine.local.posy"
	AddExpr "ValveBiped.HandControlRotL.kine.local.posy"
	AddExpr "ValveBiped.HandControlRotL1.kine.local.posy"
	AddExpr "ValveBiped.ClavicleControlR.kine.local.posy"
	AddExpr "ValveBiped.HandControlRotR.kine.local.posy"
	AddExpr "ValveBiped.HandControlRotR1.kine.local.posy"
	AddExpr "ValveBiped.ArmRollR.kine.local.posy"
	AddExpr "ValveBiped.LegRollL.kine.local.posy"
	AddExpr "ValveBiped.LegRollR.kine.local.posy"
	AddExpr "ValveBiped.Bip01_Pelvis.kine.local.posy"
	AddExpr "ValveBiped.ClavicleControlL.kine.local.posz"
	AddExpr "ValveBiped.ArmRollL.kine.local.posz"
	AddExpr "ValveBiped.HandControlRotL.kine.local.posz"
	AddExpr "ValveBiped.HandControlRotL1.kine.local.posz"
	AddExpr "ValveBiped.ClavicleControlR.kine.local.posz"
	AddExpr "ValveBiped.HandControlRotR.kine.local.posz"
	AddExpr "ValveBiped.HandControlRotR1.kine.local.posz"
	AddExpr "ValveBiped.ArmRollR.kine.local.posz"
	AddExpr "ValveBiped.LegRollL.kine.local.posz"
	AddExpr "ValveBiped.LegRollR.kine.local.posz"
	AddExpr "ValveBiped.Bip01_Pelvis.kine.local.posz"
	AddExpr "ValveBiped.HandControlPosR.kine.local.rotx"
	AddExpr "ValveBiped.HandControlPosL.kine.local.rotx"
	AddExpr "ValveBiped.HandControlPosR.kine.local.roty"
	AddExpr "ValveBiped.HandControlPosL.kine.local.roty"
	AddExpr "ValveBiped.HandControlPosR.kine.local.rotz"
	AddExpr "ValveBiped.HandControlPosL.kine.local.rotz"

	'--transform setups

	DeselectAll
	AddToSelection "ValveBiped.HandControlRotL", , True
	AddToSelection "ValveBiped.HandControlRotR", , True
	AddToSelection "ValveBiped.Bip01_Pelvis", , True
	AddToSelection "ValveBiped.SpineControl", , True
	AddToSelection "ValveBiped.SpineControl1", , True
	AddToSelection "ValveBiped.SpineControl2", , True
	AddToSelection "ValveBiped.SpineControl3", , True
	AddToSelection "ValveBiped.NeckControl", , True
	AddToSelection "ValveBiped.HeadControl", , True
	AddToSelection "ValveBiped.ClavicleControlL", , True
	AddToSelection "ValveBiped.ClavicleControlR", , True
	AddToSelection "ValveBiped.ArmRollR", , True
	AddToSelection "ValveBiped.ArmRollL", , True
	AddToSelection "ValveBiped.LegRollL", , True
	AddToSelection "ValveBiped.LegRollR", , True
	AddToSelection "ValveBiped.RightRoll", , True
	AddToSelection "ValveBiped.LeftRoll", , True
	AddToSelection "ValveBiped.RightToe", , True
	AddToSelection "ValveBiped.LeftToe", , True

	AddProp "Transform Setup"
	SetValue "ValveBiped.HandControlRotL.transformsetup.tool,ValveBiped.HandControlRotR.transformsetup.tool,ValveBiped.Bip01_Pelvis.transformsetup.tool,ValveBiped.SpineControl.transformsetup.tool,ValveBiped.SpineControl1.transformsetup.tool,ValveBiped.SpineControl2.transformsetup.tool,ValveBiped.SpineControl3.transformsetup.tool,ValveBiped.NeckControl.transformsetup.tool,ValveBiped.HeadControl.transformsetup.tool,ValveBiped.ClavicleControlL.transformsetup.tool,ValveBiped.ClavicleControlR.transformsetup.tool,ValveBiped.ArmRollR.transformsetup.tool,ValveBiped.ArmRollL.transformsetup.tool,ValveBiped.LegRollL.transformsetup.tool,ValveBiped.LegRollR.transformsetup.tool,ValveBiped.RightRoll.transformsetup.tool,ValveBiped.LeftRoll.transformsetup.tool,ValveBiped.RightToe.transformsetup.tool,ValveBiped.LeftToe.transformsetup.tool", Array(3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3)

	DeselectAll

	'--custom panel for IK/FK switch **DELETED**

	'--synoptic **DELETED, SORRY**

	'--rename spine and neck roots

'	SetValue "ValveBiped.Bip01.Spine.Name", "Spine_Root"
'	SetValue "ValveBiped.eff.Neck1.Name", "Neck_Root"


	'--add plotted group (includes marking set for fingers)

	SelectObj "ValveBiped.Bip01_L_Finger0", , True
	AddToSelection "ValveBiped.Bip01_L_Finger4,ValveBiped.Bip01_L_Finger41,ValveBiped.Bip01_L_Finger42,ValveBiped.Bip01_L_Finger3,ValveBiped.Bip01_L_Finger31,ValveBiped.Bip01_L_Finger32,ValveBiped.Bip01_L_Finger2,ValveBiped.Bip01_L_Finger21,ValveBiped.Bip01_L_Finger22,ValveBiped.Bip01_L_Finger1,ValveBiped.Bip01_L_Finger11,ValveBiped.Bip01_L_Finger12,ValveBiped.Bip01_L_Finger0,ValveBiped.Bip01_L_Finger01,ValveBiped.Bip01_L_Finger02", , True
	AddToSelection "ValveBiped.Bip01_R_Finger4,ValveBiped.Bip01_R_Finger41,ValveBiped.Bip01_R_Finger42,ValveBiped.Bip01_R_Finger3,ValveBiped.Bip01_R_Finger31,ValveBiped.Bip01_R_Finger32,ValveBiped.Bip01_R_Finger2,ValveBiped.Bip01_R_Finger21,ValveBiped.Bip01_R_Finger22,ValveBiped.Bip01_R_Finger1,ValveBiped.Bip01_R_Finger11,ValveBiped.Bip01_R_Finger12,ValveBiped.Bip01_R_Finger0,ValveBiped.Bip01_R_Finger01,ValveBiped.Bip01_R_Finger02", , True
	SetMarking "kine.local.ori.euler.rotx"
	AddToMarking "kine.local.ori.euler.roty"
	AddToMarking "kine.local.ori.euler.rotz"
	CreateMarkingSet "ValveBiped.Bip01_L_Finger0,ValveBiped.Bip01_L_Finger4,ValveBiped.Bip01_L_Finger41,ValveBiped.Bip01_L_Finger42,ValveBiped.Bip01_L_Finger3,ValveBiped.Bip01_L_Finger31,ValveBiped.Bip01_L_Finger32,ValveBiped.Bip01_L_Finger2,ValveBiped.Bip01_L_Finger21,ValveBiped.Bip01_L_Finger22,ValveBiped.Bip01_L_Finger1,ValveBiped.Bip01_L_Finger11,ValveBiped.Bip01_L_Finger12,ValveBiped.Bip01_L_Finger01,ValveBiped.Bip01_L_Finger02,ValveBiped.Bip01_R_Finger4,ValveBiped.Bip01_R_Finger41,ValveBiped.Bip01_R_Finger42,ValveBiped.Bip01_R_Finger3,ValveBiped.Bip01_R_Finger31,ValveBiped.Bip01_R_Finger32,ValveBiped.Bip01_R_Finger2,ValveBiped.Bip01_R_Finger21,ValveBiped.Bip01_R_Finger22,ValveBiped.Bip01_R_Finger1,ValveBiped.Bip01_R_Finger11,ValveBiped.Bip01_R_Finger12,ValveBiped.Bip01_R_Finger0,ValveBiped.Bip01_R_Finger01,ValveBiped.Bip01_R_Finger02", False
	AddToSelection "ValveBiped.SpineControl", , True
	AddToSelection "ValveBiped.SpineControl1", , True
	AddToSelection "ValveBiped.SpineControl2", , True
	AddToSelection "ValveBiped.SpineControl3", , True
	AddToSelection "ValveBiped.NeckControl", , True
	AddToSelection "ValveBiped.ClavicleControlL", , True
	AddToSelection "ValveBiped.ArmRollL", , True
	AddToSelection "ValveBiped.HandControlPosL", , True
	AddToSelection "ValveBiped.HandControlRotL", , True
	AddToSelection "ValveBiped.ClavicleControlR", , True
	AddToSelection "ValveBiped.HeadControl", , True
	AddToSelection "ValveBiped.ArmRollR", , True
	AddToSelection "ValveBiped.HandControlPosR", , True
	AddToSelection "ValveBiped.HandControlRotR", , True
	AddToSelection "ValveBiped.UpperBody", , True
	AddToSelection "ValveBiped.Bip01_Pelvis", , True
	AddToSelection "ValveBiped.LegRollL", , True
	AddToSelection "ValveBiped.LegRollR", , True
	AddToSelection "ValveBiped.LeftFoot", , True
	AddToSelection "ValveBiped.LeftToe", , True
	AddToSelection "ValveBiped.RightToe", , True
	AddToSelection "ValveBiped.RightFoot", , True
	AddToSelection "ValveBiped.RightLeg_UpV", , True
	AddToSelection "ValveBiped.LeftLeg_UpV", , True
	AddToSelection "ValveBiped.RightRoll", , True
	AddToSelection "ValveBiped.LeftRoll", , True
	AddToSelection "ValveBiped.rArmUpV", , True
	AddToSelection "ValveBiped.lArmUpV", , True


	CreateGroup
	SetValue "Group.Name", "Plotted"
	CopyPaste "Plotted", , "ValveBiped", 1
	DeselectAll

	'--hand pos transform setup

	SelectObj "ValveBiped.HandControlPosR", , True
	AddToSelection "ValveBiped.HandControlPosL", , True
	AddProp "Transform Setup"
	SetValue "ValveBiped.HandControlPosR.transformsetup.tool,ValveBiped.HandControlPosL.transformsetup.tool", Array(4, 4)
	SetValue "ValveBiped.HandControlPosR.transformsetup.translate,ValveBiped.HandControlPosL.transformsetup.translate", Array(0, 0)
	DeselectAll

	'--remove roots and effectors from "Envelope_Deformers" group

	SelectObj "ValveBiped.LArmRoot"
	ToggleSelection "ValveBiped.LHandRoot,ValveBiped.LIndexRoot,ValveBiped.LLegRoot,ValveBiped.LMiddleRoot,ValveBiped.LPinkyRoot,ValveBiped.LRingRoot,ValveBiped.LRootFoot,ValveBiped.LRootShoulder,ValveBiped.LThumbRoot,ValveBiped.Neck_Root,ValveBiped.RArmRoot,ValveBiped.RHandRoot,ValveBiped.RIndexRoot,ValveBiped.RLegRoot,ValveBiped.RMiddleRoot,ValveBiped.RPinkyRoot,ValveBiped.RRingRoot,ValveBiped.RRootFoot,ValveBiped.RRootShoulder,ValveBiped.RThumbRoot,ValveBiped.Spine_Root"
	RemoveFromGroup "ValveBiped.Envelope_Deformers", "ValveBiped.LArmRoot,ValveBiped.LHandRoot,ValveBiped.LIndexRoot,ValveBiped.LLegRoot,ValveBiped.LMiddleRoot,ValveBiped.LPinkyRoot,ValveBiped.LRingRoot,ValveBiped.LRootFoot,ValveBiped.LRootShoulder,ValveBiped.LThumbRoot,ValveBiped.Neck_Root,ValveBiped.RArmRoot,ValveBiped.RHandRoot,ValveBiped.RIndexRoot,ValveBiped.RLegRoot,ValveBiped.RMiddleRoot,ValveBiped.RPinkyRoot,ValveBiped.RRingRoot,ValveBiped.RRootFoot,ValveBiped.RRootShoulder,ValveBiped.RThumbRoot,ValveBiped.Spine_Root"
	SelectObj "ValveBiped.eff"
	ToggleSelection "ValveBiped.eff1,ValveBiped.LArmeff,ValveBiped.LFooteff,ValveBiped.LHandeffector,ValveBiped.LIndexeffector,ValveBiped.LMiddleeffector,ValveBiped.LPinkyeffector,ValveBiped.LRingeffector,ValveBiped.LShouldereff,ValveBiped.LThumbeffector,ValveBiped.LToeeff,ValveBiped.RArmeff,ValveBiped.RFooteff,ValveBiped.RHandeffector,ValveBiped.RIndexeffector,ValveBiped.RMiddleeffector,ValveBiped.RPinkyeffector,ValveBiped.RRingeffector,ValveBiped.RShouldereff,ValveBiped.RThumbeffector,ValveBiped.RToeeff"
	RemoveFromGroup "ValveBiped.Envelope_Deformers", "ValveBiped.eff,ValveBiped.eff1,ValveBiped.LArmeff,ValveBiped.LFooteff,ValveBiped.LHandeffector,ValveBiped.LIndexeffector,ValveBiped.LMiddleeffector,ValveBiped.LPinkyeffector,ValveBiped.LRingeffector,ValveBiped.LShouldereff,ValveBiped.LThumbeffector,ValveBiped.LToeeff,ValveBiped.RArmeff,ValveBiped.RFooteff,ValveBiped.RHandeffector,ValveBiped.RIndexeffector,ValveBiped.RMiddleeffector,ValveBiped.RPinkyeffector,ValveBiped.RRingeffector,ValveBiped.RShouldereff,ValveBiped.RThumbeffector,ValveBiped.RToeeff"
	DeselectAll

	'---Hide legacy controls.  Don't delete them if you plan on using most action presets created to date
	SelectObj "ValveBiped.ArmRollR", , True
	ToggleSelection "ValveBiped.ArmRollL", , True
	ToggleSelection "ValveBiped.HandControlRotR1", , True
	ToggleSelection "ValveBiped.HandControlRotL1", , True
	ToggleSelection "ValveBiped.LegRollR", , True
	ToggleSelection "ValveBiped.LegRollL", , True
	ToggleVisibility

	SetUserPref "AutoInspectEnabled", PreviousAutoInspect 			



end sub

'//////////////////////////////////////////////////////////////////////////////////////
' Command: vstCreateRigFromGuide
' (Hierachy From Biped)
' misner. ported to MOD Tool by Dominic Laflamme
'//////////////////////////////////////////////////////////////////////////////////////
function vstCreateRigFromGuide_Execute()

	dim hType, valid, nbVertebra 

	set vstCreateRigFromGuide_Execute = Nothing
	
	dim oGuide : set oGuide = Nothing
	on error resume next
	set oGuide = Dictionary.GetObject("ValveGuide")
	on error goto 0
	
	if (typename(oGuide) = "Nothing") then
		logmessage "vstCreateRigFromGuide: ValveGuide not found", siError
		exit function
	end if
	
	setuserpref("SI3D_NODETRANSFORM_CHILD_COMPENSATE"), 0
	
 	attach_skeleton

'-----------------------------------

'--- delete CSguide

'DeleteObj "B:ValveGuide"

'--cut hand

SelectObj "ValveBiped.RHandRoot"
CutObj
SelectObj "ValveBiped.LHandRoot", , True
CutObj




'--move arm upvector controls

SelectObj "ValveBiped.lArmUpVParent1", , True
MatchTransform "ValveBiped.lArmUpVParent1", "ValveBiped.Bip01_R_UpperArm", siTrn
Translate , 0, 0, -10, siRelative, siGlobal, siObj, siXYZ
SelectObj "ValveBiped.lArmUpVParent", , True
MatchTransform "ValveBiped.lArmUpVParent", "ValveBiped.Bip01_L_UpperArm", siTrn
Translate , 0, 0, -10, siRelative, siGlobal, siObj, siXYZ
SetValue "ValveBiped.lArmUpVParent.kine.global.rotx", 0
SetValue "ValveBiped.lArmUpVParent.kine.global.roty", 0
SetValue "ValveBiped.lArmUpVParent.kine.global.rotz", 0
SetValue "ValveBiped.lArmUpVParent1.kine.global.rotx", 0
SetValue "ValveBiped.lArmUpVParent1.kine.global.roty", 0
SetValue "ValveBiped.lArmUpVParent1.kine.global.rotz", 0

'--reparent hands

SelectObj "ValveBiped.RHandRoot"
ParentObj "B:ValveBiped.RArmeff", "ValveBiped.RHandRoot"
SelectObj "B:ValveBiped.RArmeff"
SelectObj "ValveBiped.LHandRoot"
ParentObj "B:ValveBiped.LArmeff", "ValveBiped.LHandRoot"
SelectObj "B:ValveBiped.LArmeff"
DeselectAll

'--move leg upvector contols

SelectObj "ValveBiped.RUpVParent", , True
MatchTransform "ValveBiped.RUpVParent", "ValveBiped.Bip01_R_Thigh", siTrn
Translate , 0, 0, -10, siRelative, siGlobal, siObj, siXYZ
SetValue "ValveBiped.RUpVParent.kine.global.rotx", 0
SetValue "ValveBiped.RUpVParent.kine.global.roty", 0
SetValue "ValveBiped.RUpVParent.kine.global.rotz", 0
SelectObj "ValveBiped.LUpVParent", , True
MatchTransform "ValveBiped.LUpVParent", "ValveBiped.Bip01_L_Thigh", siTrn
Translate , 0, 0, -10, siRelative, siGlobal, siObj, siXYZ
SetValue "ValveBiped.LUpVParent.kine.global.rotx", 0
SetValue "ValveBiped.LUpVParent.kine.global.roty", 0
SetValue "ValveBiped.LUpVParent.kine.global.rotz", 0
DeselectAll

'---- ValveRig from IsnerRig

'#####  Setup Upperbody control, bip01 and pelvis, adjust hierarchy
SelectObj "ValveBiped.Spine", , True
ToggleSelection "ValveBiped.LLegRoot", , True
ToggleSelection "ValveBiped.RLegRoot", , True
CutObj

DeleteObj "ValveBiped.Bip01_Pelvis"

'MatchTransform "ValveBiped.Bip01", "MaxSceneRoot.Bip01", siTrn

ApplyCns "TwoPoints", "ValveBiped.Bip01", "ValveBiped.Bip01_R_Thigh,ValveBiped.Bip01_L_Thigh"
SelectObj "ValveBiped.Bip01", , True
Refresh
RemoveAllCns

MatchTransform "ValveBiped.UpperBody", "ValveBiped.Bip01", siSRT

	'-- create new pelvis, make child of bip01, align correctly 

GetPrim "Cube"
SetValue "cube.Name", "Bip01_Pelvis"
MatchTransform "Bip01_Pelvis", "ValveBiped.Bip01", siSRT
CopyPaste "Bip01_Pelvis", , "ValveBiped.Bip01", 1
MatchTransform "ValveBiped.Bip01_Pelvis", "ValveBiped.Bip01", siRot


	'-- constrain bip01, setup hierarchy
	
ApplyCns "Pose", "ValveBiped.Bip01", "ValveBiped.UpperBody"
CopyPaste "ValveBiped.LLegRoot", , "ValveBiped.Bip01_Pelvis", 1
CopyPaste "ValveBiped.RLegRoot", , "ValveBiped.Bip01_Pelvis", 1
CopyPaste "ValveBiped.Spine", , "ValveBiped.Bip01", 1


'########  Create and Setup Spine, Neck, and Head controls 

	'-- SpineControl
	
		'-Create Objects
		
CreatePrim "Sphere", "NurbsSurface"
SetValue "sphere.surfmsh.geom.subdivu", 4
SetValue "sphere.surfmsh.geom.subdivv", 2
AddProp "Display Property"
SetValue "sphere.display.staticsel", 0
SetValue "sphere.display.intsel", 0
SetValue "sphere.display.playbacksel", 0
SetValue "sphere.display.staticunselnear", 0
SetValue "sphere.display.intunselnear", 0
SetValue "sphere.display.playbackunselnear", 0
SetValue "sphere.display.staticunselfar", 0
SetValue "sphere.display.intunselfar", 0
SetValue "sphere.display.playbackunselfar", 0
SetValue "sphere.Name", "SpineControl"

GetPrim "Null"
SetValue "null.Name", "SpineControlParent"

GetPrim "Null"
SetValue "null.Name", "SpineControlConstrainer"

		'-Setup Hierarchy, align objects, constrain Bone to control
		
MatchTransform "SpineControlParent", "ValveBiped.Spine", siSRT
CopyPaste "SpineControlParent", , "ValveBiped.UpperBody", 1
CopyPaste "SpineControl", , "ValveBiped.SpineControlParent", 1
CopyPaste "SpineControlConstrainer", , "ValveBiped.SpineControl", 1
MatchTransform "ValveBiped.SpineControl", "ValveBiped.SpineControlParent", siSRT
SelectObj "ValveBiped.SpineControlParent", , True
Rotate , 90, 0, 0, siAbsolute, siAdd, siObj, siX
Rotate , 0, 0, 90, siAbsolute, siAdd, siObj, siZ
MatchTransform "ValveBiped.SpineControlConstrainer", "ValveBiped.Bip01_Spine", siRot
ApplyCns "Orientation", "ValveBiped.Bip01_Spine", "ValveBiped.SpineControlConstrainer"

SelectObj "ValveBiped.SpineControl", , True
ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.SpineControl.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
Translate , 0, 0, -10.120039710109, siRelative, siGlobal, siObj, siXYZ

	'-- SpineControl1
	
Duplicate "ValveBiped.SpineControl", , 2, 1, 1, 0, 0, 1, 0, 1
MatchTransform "ValveBiped.SpineControl1", "ValveBiped.Bip01_Spine1", siTrn
CopyPaste "ValveBiped.SpineControl1", , "ValveBiped.SpineControl", 1
Duplicate "ValveBiped.SpineControlConstrainer", , 2, 1, 1, 0, 0, 1, 0, 1
CopyPaste "ValveBiped.SpineControlConstrainer1", , "ValveBiped.SpineControl1", 1
MatchTransform "ValveBiped.SpineControlConstrainer1", "ValveBiped.Bip01_Spine1", siSRT
ApplyCns "Orientation", "ValveBiped.Bip01_Spine1", "ValveBiped.SpineControlConstrainer1"
SelectObj "ValveBiped.SpineControl1", , True
ActivateVertexSelTool
Translate , 0, 0, -1.04703723952745, siRelative, siGlobal, siObj, siXYZ
ActivateObjectSelTool

	'-- SpineControl2
	
Duplicate "ValveBiped.SpineControl1", , 2, 1, 1, 0, 0, 1, 0, 1
Duplicate "ValveBiped.SpineControlConstrainer1", , 2, 1, 1, 0, 0, 1, 0, 1
CopyPaste "ValveBiped.SpineControlConstrainer2", , "ValveBiped.SpineControl2", 1
CopyPaste "ValveBiped.SpineControl2", , "ValveBiped.SpineControl1", 1
MatchTransform "ValveBiped.SpineControl2", "ValveBiped.Bip01_Spine2", siTrn
MatchTransform "ValveBiped.SpineControlConstrainer2", "ValveBiped.Bip01_Spine2", siSRT
ApplyCns "Orientation", "ValveBiped.Bip01_Spine2", "ValveBiped.SpineControlConstrainer2"

	'-- SpineControl3
	
Duplicate "ValveBiped.SpineControl2", , 2, 1, 1, 0, 0, 1, 0, 1
Duplicate "ValveBiped.SpineControlConstrainer2", , 2, 1, 1, 0, 0, 1, 0, 1
CopyPaste "ValveBiped.SpineControlConstrainer3", , "ValveBiped.SpineControl3", 1
CopyPaste "ValveBiped.SpineControl3", , "ValveBiped.SpineControl2", 1
MatchTransform "ValveBiped.SpineControl3", "ValveBiped.Bip01_Spine4", siTrn
MatchTransform "ValveBiped.SpineControlConstrainer3", "ValveBiped.Bip01_Spine4", siSRT
ApplyCns "Orientation", "ValveBiped.Bip01_Spine4", "ValveBiped.SpineControlConstrainer3"

	'-- NeckControl
	
Duplicate "ValveBiped.SpineControl3", , 2, 1, 1, 0, 0, 1, 0, 1
SetValue "ValveBiped.SpineControl2.SpineControl4.Name", "NeckControl"
Duplicate "ValveBiped.SpineControlConstrainer3", , 2, 1, 1, 0, 0, 1, 0, 1
SetValue "ValveBiped.SpineControl3.SpineControlConstrainer4.Name", "NeckControlConstrainer"
CopyPaste "ValveBiped.NeckControlConstrainer", , "ValveBiped.NeckControl", 1
CopyPaste "ValveBiped.NeckControl", , "ValveBiped.SpineControl3", 1
MatchTransform "ValveBiped.NeckControl", "ValveBiped.Bip01_Neck1", siTrn
MatchTransform "ValveBiped.NeckControlConstrainer", "ValveBiped.Bip01_Neck1", siSRT
ApplyCns "Orientation", "ValveBiped.Bip01_Neck1", "ValveBiped.NeckControlConstrainer"
		
	'--HeadControl
	
Duplicate "ValveBiped.NeckControl", , 2, 1, 1, 0, 0, 1, 0, 1
SetValue "ValveBiped.SpineControl3.NeckControl1.Name", "HeadControl"
Duplicate "ValveBiped.NeckControlConstrainer", , 2, 1, 1, 0, 0, 1, 0, 1
SetValue "ValveBiped.NeckControl.NeckControlConstrainer1.Name", "HeadControlConstrainer"
CopyPaste "ValveBiped.HeadControl", , "ValveBiped.NeckControl", 1
CopyPaste "ValveBiped.HeadControlConstrainer", , "ValveBiped.HeadControl", 1
MatchTransform "ValveBiped.HeadControl", "ValveBiped.Bip01_Head1", siTrn
MatchTransform "ValveBiped.HeadControlConstrainer", "ValveBiped.Bip01_Head1", siSRT
ApplyCns "Orientation", "ValveBiped.Bip01_Head1", "ValveBiped.HeadControlConstrainer"

		'--Slight adjustments to control points for easier selecting	 (human specific results)
	
SelectObj "ValveBiped.SpineControl2", , True
ActivateVertexSelTool
Translate , 6.79319096764948E-17, 3.32913340503841, -1.10944891743307, siRelative, siView, siObj, siXYZ
ActivateObjectSelTool

SelectObj "ValveBiped.NeckControl", , True
ActivateVertexSelTool
Translate , -9.18861341181465E-17, 4.11245891210626, 1.50101912358591, siRelative, siView, siObj, siXYZ
ActivateObjectSelTool

SelectObj "ValveBiped.HeadControl", , True
ActivateVertexSelTool
Translate , -1.03896214277265E-16, 11.097111350128, 1.69680422666234, siRelative, siView, siObj, siXYZ
ActivateObjectSelTool


'##########   ClavicleControl Setup

	'--ClavicleControlL
	
GetPrim "Null"
SetValue "null.Name", "ClavicleControlParentL"
Duplicate "ValveBiped.HeadControl", , 2, 1, 1, 0, 0, 1, 0, 1
SetValue "ValveBiped.NeckControl.HeadControl1.Name", "ClavicleControlL"
CopyPaste "ClavicleControlParentL", , "ValveBiped.SpineControl3", 1
CopyPaste "ValveBiped.ClavicleControlL", , "ValveBiped.ClavicleControlParentL", 1
Refresh
MatchTransform "ValveBiped.ClavicleControlParentL", "ValveBiped.Bip01_L_Clavicle", siSRT
MatchTransform "ValveBiped.ClavicleControlL", "ValveBiped.Bip01_L_Clavicle", siSRT
Refresh
SelectObj "ValveBiped.ClavicleControlL", , True
ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.ClavicleControlL.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
Translate , 3, 18.3075249631281, 0, siRelative, siGlobal, siObj, siXYZ
Refresh
ActivateObjectSelTool
ApplyCns "Orientation", "ValveBiped.Bip01_L_Clavicle", "ValveBiped.ClavicleControlL"

	'ClavicleControlR
	
GetPrim "Null"
SetValue "null.Name", "ClavicleControlParentR"
Duplicate "ValveBiped.HeadControl", , 2, 1, 1, 0, 0, 1, 0, 1
SetValue "ValveBiped.NeckControl.HeadControl1.Name", "ClavicleControlR"
CopyPaste "ClavicleControlParentR", , "ValveBiped.SpineControl3", 1
CopyPaste "ValveBiped.ClavicleControlR", , "ValveBiped.ClavicleControlParentR", 1
Refresh
MatchTransform "ValveBiped.ClavicleControlParentR", "ValveBiped.Bip01_R_Clavicle", siSRT
MatchTransform "ValveBiped.ClavicleControlR", "ValveBiped.Bip01_R_Clavicle", siSRT
Refresh
SelectObj "ValveBiped.ClavicleControlR", , True
ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.ClavicleControlR.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
Translate , -4, 18.3075249631281, 0, siRelative, siGlobal, siObj, siXYZ
Refresh
ActivateObjectSelTool
ApplyCns "Orientation", "ValveBiped.Bip01_R_Clavicle", "ValveBiped.ClavicleControlR"



'######HandControl Setup

	'--LeftHand
GetPrim "Null"
SetValue "null.Name", "HandControlPosParentL"
CopyPaste "HandControlPosParentL", , "ValveBiped", 1
ApplyCns "Position", "ValveBiped.HandControlPosParentL", "ValveBiped.LShoulderEff"
	
GetPrim "Null"
SetValue "null.Name", "HandControlRotGrandmaL"
CopyPaste "HandControlRotGrandmaL", , "ValveBiped", 1
ApplyCns "Pose", "ValveBiped.HandControlRotGrandmaL", "ValveBiped.Bip01_L_Forearm"

		'--UpperArmControlL **NOPE**
		
Refresh


		'--HandControlPosL
		
CreatePrim "Sphere", "NurbsSurface"
SetValue "sphere.surfmsh.geom.subdivu", 4
SetValue "sphere.surfmsh.geom.subdivv", 2
AddProp "Display Property"
SetValue "sphere.display.staticsel", 0
SetValue "sphere.display.intsel", 0
SetValue "sphere.display.playbacksel", 0
SetValue "sphere.display.staticunselnear", 0
SetValue "sphere.display.intunselnear", 0
SetValue "sphere.display.playbackunselnear", 0
SetValue "sphere.display.staticunselfar", 0
SetValue "sphere.display.intunselfar", 0
SetValue "sphere.display.playbackunselfar", 0
SetValue "sphere.Name", "HandControlPosL"
SetValue "HandControlPosL.sphere.radius", 3
MatchTransform "HandControlPosL", "ValveBiped.LArmEff", siTrn
CopyPaste "HandControlPosL", , "ValveBiped.HandControlPosParentL", 1
ApplyCns "Position", "ValveBiped.LArmEff", "ValveBiped.HandControlPosL"


		'--HandControlRotL
		
GetPrim "Null"
SetValue "null.Name", "HandControlRotParentL"
MatchTransform "HandControlRotParentL", "ValveBiped.Bip01_L_Hand", siSRT
CopyPaste "HandControlRotParentL", , "ValveBiped.HandControlRotGrandmaL", 1

CreatePrim "Cube", "MeshSurface"
SetValue "cube.Name", "HandControlRotL"
MatchTransform "HandControlRotL", "ValveBiped.HandControlRotParentL", siSRT
CopyPaste "HandControlRotL", , "ValveBiped.HandControlRotParentL", 1
SetValue "ValveBiped.HandControlRotL.cube.length", 4
Refresh

ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.HandControlRotL.pnt[*]"
Translate , 1.96995415771412, 0, 0, siRelative, siParent, siObj, siXYZ
Scale , 1, 0.585664335664336, 1, siRelative, siParent, siObj, siXYZ
Scale , 1, 1, 1.18881118881119, siRelative, siParent, siObj, siXYZ
ActivateObjectSelTool
Refresh

Duplicate "ValveBiped.HandControlRotL", , 2, 1, 1, 0, 0, 1, 0, 1
CopyPaste "ValveBiped.HandControlRotL1", , "ValveBiped.HandControlRotL", 1
SelectObj "ValveBiped.HandControlRotL1"
ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.HandControlRotL1.pnt[*]"
Scale , 0.732517482517483, 0.732517482517483, 0.732517482517483, siRelative, siParent, siObj, siXYZ
ActivateObjectSelTool
Refresh

GetPrim "Null"
SetValue "null.Name", "HandControlRotConstrainerL"
CopyPaste "HandControlRotConstrainerL", , "ValveBiped.HandControlRotL1", 1
MatchTransform "ValveBiped.HandControlRotConstrainerL", "ValveBiped.Bip01_L_Hand", siSRT
Refresh
ApplyCns "Orientation", "ValveBiped.Bip01_L_Hand", "ValveBiped.HandControlRotConstrainerL"
Refresh

	
	
	'--RightHand
	
GetPrim "Null"
SetValue "null.Name", "HandControlPosParentR"
CopyPaste "HandControlPosParentR", , "ValveBiped", 1
ApplyCns "Position", "ValveBiped.HandControlPosParentR", "ValveBiped.RShoulderEff"
	
GetPrim "Null"
SetValue "null.Name", "HandControlRotGrandmaR"
CopyPaste "HandControlRotGrandmaR", , "ValveBiped", 1
ApplyCns "Pose", "ValveBiped.HandControlRotGrandmaR", "ValveBiped.Bip01_R_Forearm"

		'--UpperArmControlR **NOPE**

		'--HandControlPosR
		
CreatePrim "Sphere", "NurbsSurface"
SetValue "sphere.surfmsh.geom.subdivu", 4
SetValue "sphere.surfmsh.geom.subdivv", 2
AddProp "Display Property"
SetValue "sphere.display.staticsel", 0
SetValue "sphere.display.intsel", 0
SetValue "sphere.display.playbacksel", 0
SetValue "sphere.display.staticunselnear", 0
SetValue "sphere.display.intunselnear", 0
SetValue "sphere.display.playbackunselnear", 0
SetValue "sphere.display.staticunselfar", 0
SetValue "sphere.display.intunselfar", 0
SetValue "sphere.display.playbackunselfar", 0
SetValue "sphere.Name", "HandControlPosR"
SetValue "HandControlPosR.sphere.radius", 3
MatchTransform "HandControlPosR", "ValveBiped.RArmEff", siTrn
CopyPaste "HandControlPosR", , "ValveBiped.HandControlPosParentR", 1
ApplyCns "Position", "ValveBiped.RArmEff", "ValveBiped.HandControlPosR"
Refresh

		'--HandControlRotR
		
GetPrim "Null"
SetValue "null.Name", "HandControlRotParentR"
MatchTransform "HandControlRotParentR", "ValveBiped.Bip01_R_Hand", siSRT
CopyPaste "HandControlRotParentR", , "ValveBiped.HandControlRotGrandmaR", 1

CreatePrim "Cube", "MeshSurface"
SetValue "cube.Name", "HandControlRotR"
MatchTransform "HandControlRotR", "ValveBiped.HandControlRotParentR", siSRT
CopyPaste "HandControlRotR", , "ValveBiped.HandControlRotParentR", 1
SetValue "ValveBiped.HandControlRotR.cube.length", 4
Refresh

ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.HandControlRotR.pnt[*]"
Translate , 1.96995415771412, 0, 0, siRelative, siParent, siObj, siXYZ
Scale , 1, 0.585664335664336, 1, siRelative, siParent, siObj, siXYZ
Scale , 1, 1, 1.18881118881119, siRelative, siParent, siObj, siXYZ
ActivateObjectSelTool
Refresh

Duplicate "ValveBiped.HandControlRotR", , 2, 1, 1, 0, 0, 1, 0, 1
CopyPaste "ValveBiped.HandControlRotR1", , "ValveBiped.HandControlRotR", 1
SelectObj "ValveBiped.HandControlRotR1"
ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.HandControlRotR1.pnt[*]"
Scale , 0.732517482517483, 0.732517482517483, 0.732517482517483, siRelative, siParent, siObj, siXYZ
ActivateObjectSelTool
Refresh

GetPrim "Null"
SetValue "null.Name", "HandControlRotConstrainerR"
CopyPaste "HandControlRotConstrainerR", , "ValveBiped.HandControlRotR1", 1
MatchTransform "ValveBiped.HandControlRotConstrainerR", "ValveBiped.Bip01_R_Hand", siSRT
ApplyCns "Orientation", "ValveBiped.Bip01_R_Hand", "ValveBiped.HandControlRotConstrainerR"
Refresh

'--Kill Hidden Group

SelectObj "ValveBiped.Hidden"
DeleteObj "ValveBiped.Hidden"

'--New ArmRollL

SelectObj "ValveBiped.lArmUpVParent"
CutObj
CutObj
CreatePrim "Sphere", "NurbsSurface"
SetValue "sphere.sphere.radius", 1
SetValue "sphere.surfmsh.geom.subdivu", 4
SetValue "sphere.surfmsh.geom.subdivv", 2
MatchTransform "sphere", "ValveBiped.LArmRoll", siTrn
SelectObj "ValveBiped.LArmRoll"
DeleteObj "ValveBiped.LArmRoll"
SelectObj "sphere"
SetValue "sphere.Name", "ArmRollL"
CopyPaste "ArmRollL", , "ValveBiped.HandControlPosParentL", 1
SelectObj "lArmUpVParent"
CopyPaste "lArmUpVParent", , "ValveBiped.ArmRollL", 1
SelectObj "ValveBiped.ArmRollL"
SetSelFilter "Point"
SelectAllUsingFilter "Point", siCheckComponentVisibility
CreateClusterCenterWithNull "ValveBiped.ArmRollL.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
ActivateObjectSelTool
DeselectAll
SelectObj "ValveBiped.ArmRollL_Point_ClsCtr"
MatchTransform "ValveBiped.ArmRollL_Point_ClsCtr", "ValveBiped.HandControlPosL", siTrn
Translate , 0, 9, 0, siRelative, siView, siObj, siXYZ
CutObj
CutObj
CopyPaste "ArmRollL_Point_ClsCtr", , "ValveBiped.HandControlPosL", 1
InspectObj "ValveBiped.ArmRollL_Point_ClsCtr.kine.local"
SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.cnsori", False
SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.cnsscl", False
SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.affbyori", False
SetValue "ValveBiped.ArmRollL_Point_ClsCtr.kine.local.affbyscl", False

'--New ArmRollR

SelectObj "ValveBiped.lArmUpVParent1"
CutObj
CutObj
CreatePrim "Sphere", "NurbsSurface"
SetValue "sphere.sphere.radius", 1
SetValue "sphere.surfmsh.geom.subdivu", 4
SetValue "sphere.surfmsh.geom.subdivv", 2
MatchTransform "sphere", "ValveBiped.RArmRoll", siTrn
SelectObj "ValveBiped.RArmRoll"
DeleteObj "ValveBiped.RArmRoll"
SelectObj "sphere"
SetValue "sphere.Name", "ArmRollR"
CopyPaste "ArmRollR", , "ValveBiped.HandControlPosParentR", 1
SelectObj "lArmUpVParent1"
SelectObj "lArmUpVParent1"
ParentObj "B:ValveBiped.ArmRollR", "lArmUpVParent1"
SelectObj "B:ValveBiped.ArmRollR"
SelectObj "ValveBiped.ArmRollR"
SetSelFilter "Point"
SelectAllUsingFilter "Point", siCheckComponentVisibility
CreateClusterCenterWithNull "ValveBiped.ArmRollR.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
ActivateObjectSelTool
DeselectAll
SelectObj "ValveBiped.ArmRollR_Point_ClsCtr"
MatchTransform "ValveBiped.ArmRollR_Point_ClsCtr", "ValveBiped.HandControlPosR", siTrn
Translate , 0, 9, 0, siRelative, siView, siObj, siXYZ
CutObj
CutObj
CopyPaste "ArmRollR_Point_ClsCtr", , "ValveBiped.HandControlPosR", 1
InspectObj "ValveBiped.ArmRollR_Point_ClsCtr.kine.local"
SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.cnsori", False
SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.cnsscl", False
SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.affbyori", False
SetValue "ValveBiped.ArmRollR_Point_ClsCtr.kine.local.affbyscl", False

'-- New LLegRoll

SelectObj "ValveBiped.LUpVParent"
CutObj
CutObj
CreatePrim "Sphere", "NurbsSurface"
SetValue "sphere.sphere.radius", 1
SetValue "sphere.surfmsh.geom.subdivu", 4
SetValue "sphere.surfmsh.geom.subdivv", 2
MatchTransform "sphere", "ValveBiped.LLegRoll", siTrn
SetValue "sphere.Name", "LegRollL"
SelectObj "ValveBiped.LLegRoll"
DeleteObj "ValveBiped.LLegRoll"
GetPrim "Null"
SetValue "null.Name", "LegRollControlParent_L"
MatchTransform "LegRollControlParent_L", "LegRollL", siTrn
SelectObj "LegRollL"
CopyPaste "LegRollL", , "LegRollControlParent_L", 1
SelectObj "LUpVParent"
CopyPaste "LUpVParent", , "LegRollL", 1
SelectObj "LegRollL", , True
SetSelFilter "Point"
SelectAllUsingFilter "Point", siCheckComponentVisibility
CreateClusterCenterWithNull "LegRollL.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
ActivateObjectSelTool
DeselectAll
SelectObj "LegRollControlParent_L"
CopyPaste "LegRollControlParent_L", , "ValveBiped", 1
SelectObj "ValveBiped.LegRollL_Point_ClsCtr"
MatchTransform "ValveBiped.LegRollL_Point_ClsCtr", "ValveBiped.LeftFoot", siTrn
Translate , 8, 0, 0, siRelative, siLocal, siObj, siX
CutObj
CutObj
CopyPaste "LegRollL_Point_ClsCtr", , "ValveBiped.LeftFoot", 1
InspectObj "ValveBiped.LegRollL_Point_ClsCtr.kine.local"
SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.cnsori", False
SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.cnsscl", False
SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.affbyori", False
SetValue "ValveBiped.LegRollL_Point_ClsCtr.kine.local.affbyscl", False

'-- New RLegRoll

SelectObj "ValveBiped.RUpVParent"
CutObj
CutObj
CreatePrim "Sphere", "NurbsSurface"
SetValue "sphere.sphere.radius", 1
SetValue "sphere.surfmsh.geom.subdivu", 4
SetValue "sphere.surfmsh.geom.subdivv", 2
MatchTransform "sphere", "ValveBiped.RLegRoll", siTrn
SetValue "sphere.Name", "LegRollR"
SelectObj "ValveBiped.RLegRoll"
DeleteObj "ValveBiped.RLegRoll"
GetPrim "Null"
SetValue "null.Name", "LegRollControlParent_R"
MatchTransform "LegRollControlParent_R", "LegRollR", siTrn
SelectObj "LegRollR"
CopyPaste "LegRollR", , "LegRollControlParent_R", 1
SelectObj "RUpVParent"
CopyPaste "RUpVParent", , "LegRollR", 1
SelectObj "LegRollR", , True
SetSelFilter "Point"
SelectAllUsingFilter "Point", siCheckComponentVisibility
CreateClusterCenterWithNull "LegRollR.pnt[(0,0),(1,0),(2,0),(3,0),(0,1),(1,1),(2,1),(3,1),(0,2),(1,2),(2,2),(3,2),(0,3),(1,3),(2,3),(3,3),(0,4),(1,4),(2,4),(3,4)]"
ActivateObjectSelTool
DeselectAll
SelectObj "LegRollControlParent_R"
CopyPaste "LegRollControlParent_R", , "ValveBiped", 1
SelectObj "ValveBiped.LegRollR_Point_ClsCtr"
MatchTransform "ValveBiped.LegRollR_Point_ClsCtr", "ValveBiped.RightFoot", siTrn
Translate , -8, 0, 0, siRelative, siRelative, siLocal, siObj, siX
CutObj
CutObj
CopyPaste "LegRollR_Point_ClsCtr", , "ValveBiped.RightFoot", 1
InspectObj "ValveBiped.LegRollR_Point_ClsCtr.kine.local"
SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.cnsori", False
SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.cnsscl", False
SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.affbyori", False
SetValue "ValveBiped.LegRollR_Point_ClsCtr.kine.local.affbyscl", False
DeselectAll

'-- constrain LegRollControlParents to LegRoots

SelectObj "ValveBiped.LegRollControlParent_L"
ApplyCns "Position", "ValveBiped.LegRollControlParent_L", "ValveBiped.LLegRoot"
SelectObj "ValveBiped.LegRollControlParent_R"
ApplyCns "Position", "ValveBiped.LegRollControlParent_R", "ValveBiped.RLegRoot"
DeselectAll


'-- cosmetic changes

SelectObj "ValveBiped.SpineControl", , True
AddToSelection "ValveBiped.SpineControl,ValveBiped.SpineControl1", , True
AddToSelection "ValveBiped.SpineControl2", , True
AddToSelection "ValveBiped.SpineControl3", , True
AddToSelection "ValveBiped.NeckControl", , True
AddToSelection "ValveBiped.HeadControl", , True
SetValue "ValveBiped.SpineControl.sphere.radius,ValveBiped.SpineControl1.sphere.radius,ValveBiped.SpineControl2.sphere.radius,ValveBiped.SpineControl3.sphere.radius,ValveBiped.NeckControl.sphere.radius,ValveBiped.HeadControl.sphere.radius", Array(1.5, 1.5, 1.5, 1.5, 1.5, 1.5)

SelectObj "ValveBiped.ClavicleControlL", , True
AddToSelection "ValveBiped.ClavicleControlR", , True
SetValue "ValveBiped.ClavicleControlL.sphere.radius,ValveBiped.ClavicleControlR.sphere.radius", Array(2, 2)

SelectObj "ValveBiped.HandControlRotL", , True
ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.HandControlRotL.pnt[1,3,5,LAST]"
Translate , 1.87375999944803, 0, 0, siRelative, siParent, siObj, siXYZ
ActivateObjectSelTool
SelectObj "ValveBiped.HandControlRotR", , True
ActivateVertexSelTool
SelectGeometryComponents "ValveBiped.HandControlRotR.pnt[1,3,5,LAST]"
Translate , 1.87375999944803, 0, 0, siRelative, siParent, siObj, siXYZ
ActivateObjectSelTool
DeselectAll

'--rotation controls group **DELETED**


'-- deforming bones group

SelectObj "ValveBiped.Bip01", "BRANCH"
SelectChildNodes
CreateGroup
SetValue "Group.Name", "Envelope_Deformers"
CopyPaste "Envelope_Deformers", , "ValveBiped", 1

DeselectAll

'-- all animateable controls group **DELETED, REDUNDANT WITH PLOTTED GROUP**

'--spine and head controls, display and marking sets

AddToSelection "ValveBiped.SpineControl", , True
AddToSelection "ValveBiped.SpineControl1", , True
AddToSelection "ValveBiped.SpineControl2", , True
AddToSelection "ValveBiped.SpineControl3", , True
AddToSelection "ValveBiped.NeckControl", , True
AddToSelection "ValveBiped.HeadControl", , True

SetValue "ValveBiped.SpineControl.nodelocal.display.staticsel,ValveBiped.SpineControl1.nodelocal.display.staticsel,ValveBiped.SpineControl2.nodelocal.display.staticsel,ValveBiped.SpineControl3.nodelocal.display.staticsel,ValveBiped.NeckControl.nodelocal.display.staticsel,ValveBiped.HeadControl.nodelocal.display.staticsel", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.intsel,ValveBiped.SpineControl1.nodelocal.display.intsel,ValveBiped.SpineControl2.nodelocal.display.intsel,ValveBiped.SpineControl3.nodelocal.display.intsel,ValveBiped.NeckControl.nodelocal.display.intsel,ValveBiped.HeadControl.nodelocal.display.intsel", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.playbacksel,ValveBiped.SpineControl1.nodelocal.display.playbacksel,ValveBiped.SpineControl2.nodelocal.display.playbacksel,ValveBiped.SpineControl3.nodelocal.display.playbacksel,ValveBiped.NeckControl.nodelocal.display.playbacksel,ValveBiped.HeadControl.nodelocal.display.playbacksel", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.staticunselnear,ValveBiped.SpineControl1.nodelocal.display.staticunselnear,ValveBiped.SpineControl2.nodelocal.display.staticunselnear,ValveBiped.SpineControl3.nodelocal.display.staticunselnear,ValveBiped.NeckControl.nodelocal.display.staticunselnear,ValveBiped.HeadControl.nodelocal.display.staticunselnear", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.intunselnear,ValveBiped.SpineControl1.nodelocal.display.intunselnear,ValveBiped.SpineControl2.nodelocal.display.intunselnear,ValveBiped.SpineControl3.nodelocal.display.intunselnear,ValveBiped.NeckControl.nodelocal.display.intunselnear,ValveBiped.HeadControl.nodelocal.display.intunselnear", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.playbackunselnear,ValveBiped.SpineControl1.nodelocal.display.playbackunselnear,ValveBiped.SpineControl2.nodelocal.display.playbackunselnear,ValveBiped.SpineControl3.nodelocal.display.playbackunselnear,ValveBiped.NeckControl.nodelocal.display.playbackunselnear,ValveBiped.HeadControl.nodelocal.display.playbackunselnear", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.staticunselfar,ValveBiped.SpineControl1.nodelocal.display.staticunselfar,ValveBiped.SpineControl2.nodelocal.display.staticunselfar,ValveBiped.SpineControl3.nodelocal.display.staticunselfar,ValveBiped.NeckControl.nodelocal.display.staticunselfar,ValveBiped.HeadControl.nodelocal.display.staticunselfar", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.intunselfar,ValveBiped.SpineControl1.nodelocal.display.intunselfar,ValveBiped.SpineControl2.nodelocal.display.intunselfar,ValveBiped.SpineControl3.nodelocal.display.intunselfar,ValveBiped.NeckControl.nodelocal.display.intunselfar,ValveBiped.HeadControl.nodelocal.display.intunselfar", Array(0, 0, 0, 0, 0, 0)
SetValue "ValveBiped.SpineControl.nodelocal.display.playbackunselfar,ValveBiped.SpineControl1.nodelocal.display.playbackunselfar,ValveBiped.SpineControl2.nodelocal.display.playbackunselfar,ValveBiped.SpineControl3.nodelocal.display.playbackunselfar,ValveBiped.NeckControl.nodelocal.display.playbackunselfar,ValveBiped.HeadControl.nodelocal.display.playbackunselfar", Array(0, 0, 0, 0, 0, 0)

SetValue "ValveBiped.HeadControl.display.wirecol,ValveBiped.NeckControl.display.wirecol,ValveBiped.SpineControl3.display.wirecol,ValveBiped.SpineControl2.display.wirecol,ValveBiped.SpineControl1.display.wirecol,ValveBiped.SpineControl.display.wirecol", Array(433, 433, 433, 433, 433, 433)


AddToMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
CreateMarkingSet "ValveBiped.SpineControl,ValveBiped.SpineControl1,ValveBiped.SpineControl2,ValveBiped.SpineControl3,ValveBiped.NeckControl,ValveBiped.HeadControl", False

DeselectAll

'--left arm controls display and marking sets

SelectObj "ValveBiped.HandControlPosL", , True
AddToSelection "ValveBiped.HandControlRotL1", , True
AddToSelection "ValveBiped.HandControlRotL", , True
AddToSelection "ValveBiped.ArmRollL", , True
AddToSelection "ValveBiped.ClavicleControlL", , True

AddProp "Display Property"

SetValue "ValveBiped.HandControlPosL.display.staticsel,ValveBiped.HandControlRotL1.display.staticsel,ValveBiped.HandControlRotL.display.staticsel,ValveBiped.ArmRollL.display.staticsel,ValveBiped.ClavicleControlL.display.staticsel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.intsel,ValveBiped.HandControlRotL1.display.intsel,ValveBiped.HandControlRotL.display.intsel,ValveBiped.ArmRollL.display.intsel,ValveBiped.ClavicleControlL.display.intsel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.playbacksel,ValveBiped.HandControlRotL1.display.playbacksel,ValveBiped.HandControlRotL.display.playbacksel,ValveBiped.ArmRollL.display.playbacksel,ValveBiped.ClavicleControlL.display.playbacksel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.staticunselnear,ValveBiped.HandControlRotL1.display.staticunselnear,ValveBiped.HandControlRotL.display.staticunselnear,ValveBiped.ArmRollL.display.staticunselnear,ValveBiped.ClavicleControlL.display.staticunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.intunselnear,ValveBiped.HandControlRotL1.display.intunselnear,ValveBiped.HandControlRotL.display.intunselnear,ValveBiped.ArmRollL.display.intunselnear,ValveBiped.ClavicleControlL.display.intunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.playbackunselnear,ValveBiped.HandControlRotL1.display.playbackunselnear,ValveBiped.HandControlRotL.display.playbackunselnear,ValveBiped.ArmRollL.display.playbackunselnear,ValveBiped.ClavicleControlL.display.playbackunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.staticunselfar,ValveBiped.HandControlRotL1.display.staticunselfar,ValveBiped.HandControlRotL.display.staticunselfar,ValveBiped.ArmRollL.display.staticunselfar,ValveBiped.ClavicleControlL.display.staticunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.intunselfar,ValveBiped.HandControlRotL1.display.intunselfar,ValveBiped.HandControlRotL.display.intunselfar,ValveBiped.ArmRollL.display.intunselfar,ValveBiped.ClavicleControlL.display.intunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.playbackunselfar,ValveBiped.HandControlRotL1.display.playbackunselfar,ValveBiped.HandControlRotL.display.playbackunselfar,ValveBiped.ArmRollL.display.playbackunselfar,ValveBiped.ClavicleControlL.display.playbackunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.HandControlPosL.display.wirecol,ValveBiped.HandControlRotL1.display.wirecol,ValveBiped.HandControlRotL.display.wirecol,ValveBiped.ArmRollL.display.wirecol,ValveBiped.ClavicleControlL.display.wirecol", Array(367, 367, 367, 367, 367)

DeselectAll

SelectObj "ValveBiped.ClavicleControlL", , True
ToggleSelection "ValveBiped.ArmRollL", , True
ToggleSelection "ValveBiped.HandControlRotL", , True
ToggleSelection "ValveBiped.HandControlRotL1", , True
SetMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
CreateMarkingSet "ValveBiped.ClavicleControlL,ValveBiped.ArmRollL,ValveBiped.HandControlRotL,ValveBiped.HandControlRotL1"

SelectObj "ValveBiped.HandControlPosL", , True
AddToSelection "ValveBiped.lArmUpV", , True
SetMarking "kine.local.pos.posx"
AddToMarking "kine.local.pos.posy"
AddToMarking "kine.local.pos.posz"
CreateMarkingSet "ValveBiped.HandControlPosL,ValveBiped.lArmUpV", False
DeselectAll

'--left leg control Display and marking sets

SelectObj "ValveBiped.LegRollL", , True
AddToSelection "ValveBiped.LeftFoot", , True
AddToSelection "ValveBiped.LeftRoll", , True
AddToSelection "ValveBiped.LeftToe", , True
AddProp "Display Property"

SetValue "ValveBiped.LegRollL.display.staticsel,ValveBiped.LeftFoot.display.staticsel,ValveBiped.LeftRoll.display.staticsel,ValveBiped.LeftToe.display.staticsel", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.intsel,ValveBiped.LeftFoot.display.intsel,ValveBiped.LeftRoll.display.intsel,ValveBiped.LeftToe.display.intsel", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.playbacksel,ValveBiped.LeftFoot.display.playbacksel,ValveBiped.LeftRoll.display.playbacksel,ValveBiped.LeftToe.display.playbacksel", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.staticunselnear,ValveBiped.LeftFoot.display.staticunselnear,ValveBiped.LeftRoll.display.staticunselnear,ValveBiped.LeftToe.display.staticunselnear", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.intunselnear,ValveBiped.LeftFoot.display.intunselnear,ValveBiped.LeftRoll.display.intunselnear,ValveBiped.LeftToe.display.intunselnear", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.playbackunselnear,ValveBiped.LeftFoot.display.playbackunselnear,ValveBiped.LeftRoll.display.playbackunselnear,ValveBiped.LeftToe.display.playbackunselnear", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.staticunselfar,ValveBiped.LeftFoot.display.staticunselfar,ValveBiped.LeftRoll.display.staticunselfar,ValveBiped.LeftToe.display.staticunselfar", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.intunselfar,ValveBiped.LeftFoot.display.intunselfar,ValveBiped.LeftRoll.display.intunselfar,ValveBiped.LeftToe.display.intunselfar", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.playbackunselfar,ValveBiped.LeftFoot.display.playbackunselfar,ValveBiped.LeftRoll.display.playbackunselfar,ValveBiped.LeftToe.display.playbackunselfar", Array(0, 0, 0, 0)
SetValue "ValveBiped.LegRollL.display.wirecol,ValveBiped.LeftFoot.display.wirecol,ValveBiped.LeftRoll.display.wirecol,ValveBiped.LeftToe.display.wirecol", Array(94, 94, 94, 94)
DeselectAll


SelectObj "ValveBiped.LegRollL", , True
ToggleSelection "ValveBiped.LeftToe", , True
SetMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
CreateMarkingSet "ValveBiped.LegRollL"


DeselectAll

'--right arm control Display and Marking sets

SelectObj "ValveBiped.ClavicleControlR", , True
AddToSelection "ValveBiped.ArmRollR", , True
AddToSelection "ValveBiped.HandControlPosR", , True
AddToSelection "ValveBiped.HandControlRotR", , True
AddToSelection "ValveBiped.HandControlRotR1", , True
AddProp "Display Property"

SetValue "ValveBiped.ClavicleControlR.display.staticsel,ValveBiped.ArmRollR.display.staticsel,ValveBiped.HandControlPosR.display.staticsel,ValveBiped.HandControlRotR.display.staticsel,ValveBiped.HandControlRotR1.display.staticsel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.intsel,ValveBiped.ArmRollR.display.intsel,ValveBiped.HandControlPosR.display.intsel,ValveBiped.HandControlRotR.display.intsel,ValveBiped.HandControlRotR1.display.intsel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.playbacksel,ValveBiped.ArmRollR.display.playbacksel,ValveBiped.HandControlPosR.display.playbacksel,ValveBiped.HandControlRotR.display.playbacksel,ValveBiped.HandControlRotR1.display.playbacksel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.staticunselnear,ValveBiped.ArmRollR.display.staticunselnear,ValveBiped.HandControlPosR.display.staticunselnear,ValveBiped.HandControlRotR.display.staticunselnear,ValveBiped.HandControlRotR1.display.staticunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.intunselnear,ValveBiped.ArmRollR.display.intunselnear,ValveBiped.HandControlPosR.display.intunselnear,ValveBiped.HandControlRotR.display.intunselnear,ValveBiped.HandControlRotR1.display.intunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.playbackunselnear,ValveBiped.ArmRollR.display.playbackunselnear,ValveBiped.HandControlPosR.display.playbackunselnear,ValveBiped.HandControlRotR.display.playbackunselnear,ValveBiped.HandControlRotR1.display.playbackunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.staticunselfar,ValveBiped.ArmRollR.display.staticunselfar,ValveBiped.HandControlPosR.display.staticunselfar,ValveBiped.HandControlRotR.display.staticunselfar,ValveBiped.HandControlRotR1.display.staticunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.intunselfar,ValveBiped.ArmRollR.display.intunselfar,ValveBiped.HandControlPosR.display.intunselfar,ValveBiped.HandControlRotR.display.intunselfar,ValveBiped.HandControlRotR1.display.intunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.playbackunselfar,ValveBiped.ArmRollR.display.playbackunselfar,ValveBiped.HandControlPosR.display.playbackunselfar,ValveBiped.HandControlRotR.display.playbackunselfar,ValveBiped.HandControlRotR1.display.playbackunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.ClavicleControlR.display.wirecol,ValveBiped.ArmRollR.display.wirecol,ValveBiped.HandControlPosR.display.wirecol,ValveBiped.HandControlRotR.display.wirecol,ValveBiped.HandControlRotR1.display.wirecol", Array(13, 13, 13, 13, 13)
DeselectAll


SelectObj "ValveBiped.ClavicleControlR", , True
AddToSelection "ValveBiped.ArmRollR", , True
AddToSelection "ValveBiped.HandControlRotR1", , True
AddToSelection "ValveBiped.HandControlRotR", , True
SetMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
CreateMarkingSet "ValveBiped.ClavicleControlR,ValveBiped.ArmRollR,ValveBiped.HandControlRotR1,ValveBiped.HandControlRotR"

SelectObj "ValveBiped.HandControlPosR", , True
SetMarking "kine.local.pos.posx"
AddToMarking "kine.local.pos.posy"
AddToMarking "kine.local.pos.posz"
CreateMarkingSet "ValveBiped.HandControlPosR", False
DeselectAll

SelectObj "ValveBiped.rArmUpV", , True
SetMarking "kine.local.pos.posx"
AddToMarking "kine.local.pos.posy"
AddToMarking "kine.local.pos.posz"
CreateMarkingSet "ValveBiped.rArmUpV"



'--right leg controls display and marking sets
SelectObj "ValveBiped.RightFoot", , True
AddToSelection "ValveBiped.RightRoll", , True
AddToSelection "ValveBiped.RightToe", , True
AddToSelection "ValveBiped.LegRollR", , True
AddProp "Display Property"

SetValue "ValveBiped.RightFoot.display.staticsel,ValveBiped.LegRollR_Point_ClsCtr.display.staticsel,ValveBiped.LegRollR.display.staticsel,ValveBiped.RightRoll.display.staticsel,ValveBiped.RightToe.display.staticsel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.intsel,ValveBiped.LegRollR_Point_ClsCtr.display.intsel,ValveBiped.LegRollR.display.intsel,ValveBiped.RightRoll.display.intsel,ValveBiped.RightToe.display.intsel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.playbacksel,ValveBiped.LegRollR_Point_ClsCtr.display.playbacksel,ValveBiped.LegRollR.display.playbacksel,ValveBiped.RightRoll.display.playbacksel,ValveBiped.RightToe.display.playbacksel", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.staticunselnear,ValveBiped.LegRollR_Point_ClsCtr.display.staticunselnear,ValveBiped.LegRollR.display.staticunselnear,ValveBiped.RightRoll.display.staticunselnear,ValveBiped.RightToe.display.staticunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.intunselnear,ValveBiped.LegRollR_Point_ClsCtr.display.intunselnear,ValveBiped.LegRollR.display.intunselnear,ValveBiped.RightRoll.display.intunselnear,ValveBiped.RightToe.display.intunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.playbackunselnear,ValveBiped.LegRollR_Point_ClsCtr.display.playbackunselnear,ValveBiped.LegRollR.display.playbackunselnear,ValveBiped.RightRoll.display.playbackunselnear,ValveBiped.RightToe.display.playbackunselnear", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.staticunselfar,ValveBiped.LegRollR_Point_ClsCtr.display.staticunselfar,ValveBiped.LegRollR.display.staticunselfar,ValveBiped.RightRoll.display.staticunselfar,ValveBiped.RightToe.display.staticunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.intunselfar,ValveBiped.LegRollR_Point_ClsCtr.display.intunselfar,ValveBiped.LegRollR.display.intunselfar,ValveBiped.RightRoll.display.intunselfar,ValveBiped.RightToe.display.intunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.playbackunselfar,ValveBiped.LegRollR_Point_ClsCtr.display.playbackunselfar,ValveBiped.LegRollR.display.playbackunselfar,ValveBiped.RightRoll.display.playbackunselfar,ValveBiped.RightToe.display.playbackunselfar", Array(0, 0, 0, 0, 0)
SetValue "ValveBiped.RightFoot.display.wirecol,ValveBiped.LegRollR_Point_ClsCtr.display.wirecol,ValveBiped.LegRollR.display.wirecol,ValveBiped.RightRoll.display.wirecol,ValveBiped.RightToe.display.wirecol", Array(11, 11, 11, 11, 11)
DeselectAll

SelectObj "ValveBiped.LegRollR", , True
SetMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
CreateMarkingSet "ValveBiped.LegRollR", False

SelectObj "ValveBiped.RightFoot", , True
SetMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
AddToMarking "kine.local.pos.posx"
AddToMarking "kine.local.pos.posy"
AddToMarking "kine.local.pos.posz"
CreateMarkingSet "ValveBiped.RightFoot", False

SelectObj "ValveBiped.RightRoll", , True
SetMarking "kine.local.ori.euler.rotx"
CreateMarkingSet "ValveBiped.RightRoll", False


'--marking sets for pelvis control

SelectObj "ValveBiped.Bip01_Pelvis", , True
SetMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
CreateMarkingSet "ValveBiped.Bip01_Pelvis", False
DeselectAll

'--hidden distracting objects

CreateGroup
SetValue "Group.Name", "hidden_distractions"
SetValue "hidden_distractions.viewvis", 0
CopyPaste "hidden_distractions", , "ValveBiped", 1
AddToSelection "ValveBiped.Bip01,ValveBiped.LeftFootParent,ValveBiped.LfootUpVector,ValveBiped.LfootUpVector2,ValveBiped.spacer,ValveBiped.heel,ValveBiped.toe,ValveBiped.mid,ValveBiped.ankle,ValveBiped.LegRollL_Point_ClsCtr,ValveBiped.RightFootParent,ValveBiped.spacer1,ValveBiped.heel1,ValveBiped.toe1,ValveBiped.mid1,ValveBiped.ankle1,ValveBiped.RfootUpVector2,ValveBiped.RfootUpVector1,ValveBiped.LegRollR_Point_ClsCtr,ValveBiped.UpperBodyParent,ValveBiped.SpineControlParent,ValveBiped.SpineControlConstrainer,ValveBiped.SpineControlConstrainer1,ValveBiped.SpineControlConstrainer2,ValveBiped.SpineControlConstrainer3,ValveBiped.NeckControlConstrainer,ValveBiped.HeadControlConstrainer,ValveBiped.ClavicleControlParentL,ValveBiped.ClavicleControlParentR,ValveBiped.HandControlPosParentL,ValveBiped.ArmRollL_Point_ClsCtr,ValveBiped.HandControlRotGrandmaL,ValveBiped.HandControlRotParentL,ValveBiped.HandControlRotConstrainerL,ValveBiped.HandControlPosParentR,ValveBiped.ArmRollR_Point_ClsCtr,ValveBiped.HandControlRotGrandmaR,ValveBiped.HandControlRotParentR,ValveBiped.HandControlRotConstrainerR,ValveBiped.LegRollControlParent_L,ValveBiped.LegRollControlParent_R", , True
SIAddToGroup "ValveBiped.hidden_distractions", "ValveBiped.Bip01,ValveBiped.LeftFootParent,ValveBiped.LfootUpVector,ValveBiped.LfootUpVector2,ValveBiped.spacer,ValveBiped.heel,ValveBiped.toe,ValveBiped.mid,ValveBiped.ankle,ValveBiped.LegRollL_Point_ClsCtr,ValveBiped.RightFootParent,ValveBiped.spacer1,ValveBiped.heel1,ValveBiped.toe1,ValveBiped.mid1,ValveBiped.ankle1,ValveBiped.RfootUpVector2,ValveBiped.RfootUpVector1,ValveBiped.LegRollR_Point_ClsCtr,ValveBiped.UpperBodyParent,ValveBiped.SpineControlParent,ValveBiped.SpineControlConstrainer,ValveBiped.SpineControlConstrainer1,ValveBiped.SpineControlConstrainer2,ValveBiped.SpineControlConstrainer3,ValveBiped.NeckControlConstrainer,ValveBiped.HeadControlConstrainer,ValveBiped.ClavicleControlParentL,ValveBiped.ClavicleControlParentR,ValveBiped.HandControlPosParentL,ValveBiped.ArmRollL_Point_ClsCtr,ValveBiped.HandControlRotGrandmaL,ValveBiped.HandControlRotParentL,ValveBiped.HandControlRotConstrainerL,ValveBiped.HandControlPosParentR,ValveBiped.ArmRollR_Point_ClsCtr,ValveBiped.HandControlRotGrandmaR,ValveBiped.HandControlRotParentR,ValveBiped.HandControlRotConstrainerR,ValveBiped.LegRollControlParent_L,ValveBiped.LegRollControlParent_R"
SelectObj "ValveBiped.spacer3", , True
AddToSelection "ValveBiped.LeftToeTarget", , True
AddToSelection "ValveBiped.spacer4", , True
AddToSelection "ValveBiped.RightToeTarget", , True
AddToSelection "ValveBiped.RToeEff", , True
AddToSelection "ValveBiped.LToeEff", , True
AddToSelection "ValveBiped.LFootEff,ValveBiped.LRootFoot", , True
AddToSelection "ValveBiped.RFootEff,ValveBiped.RRootFoot", , True
AddToSelection "ValveBiped.RLegRoot", , True
AddToSelection "ValveBiped.LLegRoot", , True
AddToSelection "ValveBiped.Spine", , True
AddToSelection "ValveBiped.RRootShoulder", , True
AddToSelection "ValveBiped.LRootShoulder", , True
AddToSelection "ValveBiped.eff,ValveBiped.Neck1", , True
AddToSelection "ValveBiped.eff1", , True
AddToSelection "ValveBiped.LArmRoot", , True
AddToSelection "ValveBiped.LShoulderEff", , True
AddToSelection "ValveBiped.RShoulderEff,ValveBiped.RArmRoot", , True
AddToSelection "ValveBiped.LArmEff,ValveBiped.LHandRoot", , True
AddToSelection "ValveBiped.LThumbRoot", , True
AddToSelection "ValveBiped.LIndexRoot", , True
AddToSelection "ValveBiped.LHandEffector", , True
AddToSelection "ValveBiped.LRingRoot", , True
AddToSelection "ValveBiped.LPinkyRoot", , True
AddToSelection "ValveBiped.LHandEffector,ValveBiped.LMiddleRoot", , True
AddToSelection "ValveBiped.RArmEff,ValveBiped.RHandRoot", , True
AddToSelection "ValveBiped.RThumbRoot", , True
AddToSelection "ValveBiped.RIndexRoot", , True
AddToSelection "ValveBiped.RHandEffector,ValveBiped.RMiddleRoot", , True
AddToSelection "ValveBiped.RRingRoot", , True
AddToSelection "ValveBiped.RPinkyRoot", , True
SIAddToGroup "ValveBiped.hidden_distractions", "ValveBiped.spacer3,ValveBiped.LeftToeTarget,ValveBiped.spacer4,ValveBiped.RightToeTarget,ValveBiped.RToeEff,ValveBiped.LToeEff,ValveBiped.LFootEff,ValveBiped.LRootFoot,ValveBiped.RFootEff,ValveBiped.RRootFoot,ValveBiped.RLegRoot,ValveBiped.LLegRoot,ValveBiped.Spine,ValveBiped.RRootShoulder,ValveBiped.LRootShoulder,ValveBiped.eff,ValveBiped.Neck1,ValveBiped.eff1,ValveBiped.LArmRoot,ValveBiped.LShoulderEff,ValveBiped.RShoulderEff,ValveBiped.RArmRoot,ValveBiped.LArmEff,ValveBiped.LHandRoot,ValveBiped.LThumbRoot,ValveBiped.LIndexRoot,ValveBiped.LHandEffector,ValveBiped.LRingRoot,ValveBiped.LPinkyRoot,ValveBiped.LMiddleRoot,ValveBiped.RArmEff,ValveBiped.RHandRoot,ValveBiped.RThumbRoot,ValveBiped.RIndexRoot,ValveBiped.RHandEffector,ValveBiped.RMiddleRoot,ValveBiped.RRingRoot,ValveBiped.RPinkyRoot"
DeselectAll

'--unselectable group **NO,THANKS.  I HAVE CLEARANCE TO SELECT THINGS**

'--add expression constants to controls

AddExpr "ValveBiped.SpineControl.kine.local.posx"
AddExpr "ValveBiped.SpineControl2.kine.local.posx"
AddExpr "ValveBiped.SpineControl1.kine.local.posx"
AddExpr "ValveBiped.SpineControl3.kine.local.posx"
AddExpr "ValveBiped.NeckControl.kine.local.posx"
AddExpr "ValveBiped.HeadControl.kine.local.posx"
AddExpr "ValveBiped.SpineControl.kine.local.posy"
AddExpr "ValveBiped.SpineControl2.kine.local.posy"
AddExpr "ValveBiped.SpineControl1.kine.local.posy"
AddExpr "ValveBiped.SpineControl3.kine.local.posy"
AddExpr "ValveBiped.NeckControl.kine.local.posy"
AddExpr "ValveBiped.HeadControl.kine.local.posy"
AddExpr "ValveBiped.SpineControl.kine.local.posz"
AddExpr "ValveBiped.SpineControl2.kine.local.posz"
AddExpr "ValveBiped.SpineControl1.kine.local.posz"
AddExpr "ValveBiped.SpineControl3.kine.local.posz"
AddExpr "ValveBiped.NeckControl.kine.local.posz"
AddExpr "ValveBiped.HeadControl.kine.local.posz"
AddExpr "ValveBiped.ArmRollL.kine.local.posx"
AddExpr "ValveBiped.HandControlRotL.kine.local.posx"
AddExpr "ValveBiped.HandControlRotL1.kine.local.posx"
AddExpr "ValveBiped.ClavicleControlR.kine.local.posx"
AddExpr "ValveBiped.HandControlRotR.kine.local.posx"
AddExpr "ValveBiped.HandControlRotR1.kine.local.posx"
AddExpr "ValveBiped.ArmRollR.kine.local.posx"
AddExpr "ValveBiped.LegRollL.kine.local.posx"
AddExpr "ValveBiped.LegRollR.kine.local.posx"
AddExpr "ValveBiped.Bip01_Pelvis.kine.local.posx"
AddExpr "ValveBiped.ClavicleControlL.kine.local.posy"
AddExpr "ValveBiped.ArmRollL.kine.local.posy"
AddExpr "ValveBiped.HandControlRotL.kine.local.posy"
AddExpr "ValveBiped.HandControlRotL1.kine.local.posy"
AddExpr "ValveBiped.ClavicleControlR.kine.local.posy"
AddExpr "ValveBiped.HandControlRotR.kine.local.posy"
AddExpr "ValveBiped.HandControlRotR1.kine.local.posy"
AddExpr "ValveBiped.ArmRollR.kine.local.posy"
AddExpr "ValveBiped.LegRollL.kine.local.posy"
AddExpr "ValveBiped.LegRollR.kine.local.posy"
AddExpr "ValveBiped.Bip01_Pelvis.kine.local.posy"
AddExpr "ValveBiped.ClavicleControlL.kine.local.posz"
AddExpr "ValveBiped.ArmRollL.kine.local.posz"
AddExpr "ValveBiped.HandControlRotL.kine.local.posz"
AddExpr "ValveBiped.HandControlRotL1.kine.local.posz"
AddExpr "ValveBiped.ClavicleControlR.kine.local.posz"
AddExpr "ValveBiped.HandControlRotR.kine.local.posz"
AddExpr "ValveBiped.HandControlRotR1.kine.local.posz"
AddExpr "ValveBiped.ArmRollR.kine.local.posz"
AddExpr "ValveBiped.LegRollL.kine.local.posz"
AddExpr "ValveBiped.LegRollR.kine.local.posz"
AddExpr "ValveBiped.Bip01_Pelvis.kine.local.posz"
AddExpr "ValveBiped.HandControlPosR.kine.local.rotx"
AddExpr "ValveBiped.HandControlPosL.kine.local.rotx"
AddExpr "ValveBiped.HandControlPosR.kine.local.roty"
AddExpr "ValveBiped.HandControlPosL.kine.local.roty"
AddExpr "ValveBiped.HandControlPosR.kine.local.rotz"
AddExpr "ValveBiped.HandControlPosL.kine.local.rotz"

'--transform setups

DeselectAll
AddToSelection "ValveBiped.HandControlRotL", , True
AddToSelection "ValveBiped.HandControlRotR", , True
AddToSelection "ValveBiped.Bip01_Pelvis", , True
AddToSelection "ValveBiped.SpineControl", , True
AddToSelection "ValveBiped.SpineControl1", , True
AddToSelection "ValveBiped.SpineControl2", , True
AddToSelection "ValveBiped.SpineControl3", , True
AddToSelection "ValveBiped.NeckControl", , True
AddToSelection "ValveBiped.HeadControl", , True
AddToSelection "ValveBiped.ClavicleControlL", , True
AddToSelection "ValveBiped.ClavicleControlR", , True
AddToSelection "ValveBiped.ArmRollR", , True
AddToSelection "ValveBiped.ArmRollL", , True
AddToSelection "ValveBiped.LegRollL", , True
AddToSelection "ValveBiped.LegRollR", , True
AddToSelection "ValveBiped.RightRoll", , True
AddToSelection "ValveBiped.LeftRoll", , True
AddToSelection "ValveBiped.RightToe", , True
AddToSelection "ValveBiped.LeftToe", , True

AddProp "Transform Setup"
SetValue "ValveBiped.HandControlRotL.transformsetup.tool,ValveBiped.HandControlRotR.transformsetup.tool,ValveBiped.Bip01_Pelvis.transformsetup.tool,ValveBiped.SpineControl.transformsetup.tool,ValveBiped.SpineControl1.transformsetup.tool,ValveBiped.SpineControl2.transformsetup.tool,ValveBiped.SpineControl3.transformsetup.tool,ValveBiped.NeckControl.transformsetup.tool,ValveBiped.HeadControl.transformsetup.tool,ValveBiped.ClavicleControlL.transformsetup.tool,ValveBiped.ClavicleControlR.transformsetup.tool,ValveBiped.ArmRollR.transformsetup.tool,ValveBiped.ArmRollL.transformsetup.tool,ValveBiped.LegRollL.transformsetup.tool,ValveBiped.LegRollR.transformsetup.tool,ValveBiped.RightRoll.transformsetup.tool,ValveBiped.LeftRoll.transformsetup.tool,ValveBiped.RightToe.transformsetup.tool,ValveBiped.LeftToe.transformsetup.tool", Array(3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3)

DeselectAll

'--custom panel for IK/FK switch **DELETED**

'--synoptic **DELETED, SORRY**

'--rename spine and neck roots

SetValue "ValveBiped.Bip01.Spine.Name", "Spine_Root"
SetValue "ValveBiped.eff.Neck1.Name", "Neck_Root"


'--add plotted group (includes marking set for fingers)

SelectObj "ValveBiped.Bip01_L_Finger0", , True
AddToSelection "ValveBiped.Bip01_L_Finger4,ValveBiped.Bip01_L_Finger41,ValveBiped.Bip01_L_Finger42,ValveBiped.Bip01_L_Finger3,ValveBiped.Bip01_L_Finger31,ValveBiped.Bip01_L_Finger32,ValveBiped.Bip01_L_Finger2,ValveBiped.Bip01_L_Finger21,ValveBiped.Bip01_L_Finger22,ValveBiped.Bip01_L_Finger1,ValveBiped.Bip01_L_Finger11,ValveBiped.Bip01_L_Finger12,ValveBiped.Bip01_L_Finger0,ValveBiped.Bip01_L_Finger01,ValveBiped.Bip01_L_Finger02", , True
AddToSelection "ValveBiped.Bip01_R_Finger4,ValveBiped.Bip01_R_Finger41,ValveBiped.Bip01_R_Finger42,ValveBiped.Bip01_R_Finger3,ValveBiped.Bip01_R_Finger31,ValveBiped.Bip01_R_Finger32,ValveBiped.Bip01_R_Finger2,ValveBiped.Bip01_R_Finger21,ValveBiped.Bip01_R_Finger22,ValveBiped.Bip01_R_Finger1,ValveBiped.Bip01_R_Finger11,ValveBiped.Bip01_R_Finger12,ValveBiped.Bip01_R_Finger0,ValveBiped.Bip01_R_Finger01,ValveBiped.Bip01_R_Finger02", , True
SetMarking "kine.local.ori.euler.rotx"
AddToMarking "kine.local.ori.euler.roty"
AddToMarking "kine.local.ori.euler.rotz"
CreateMarkingSet "ValveBiped.Bip01_L_Finger0,ValveBiped.Bip01_L_Finger4,ValveBiped.Bip01_L_Finger41,ValveBiped.Bip01_L_Finger42,ValveBiped.Bip01_L_Finger3,ValveBiped.Bip01_L_Finger31,ValveBiped.Bip01_L_Finger32,ValveBiped.Bip01_L_Finger2,ValveBiped.Bip01_L_Finger21,ValveBiped.Bip01_L_Finger22,ValveBiped.Bip01_L_Finger1,ValveBiped.Bip01_L_Finger11,ValveBiped.Bip01_L_Finger12,ValveBiped.Bip01_L_Finger01,ValveBiped.Bip01_L_Finger02,ValveBiped.Bip01_R_Finger4,ValveBiped.Bip01_R_Finger41,ValveBiped.Bip01_R_Finger42,ValveBiped.Bip01_R_Finger3,ValveBiped.Bip01_R_Finger31,ValveBiped.Bip01_R_Finger32,ValveBiped.Bip01_R_Finger2,ValveBiped.Bip01_R_Finger21,ValveBiped.Bip01_R_Finger22,ValveBiped.Bip01_R_Finger1,ValveBiped.Bip01_R_Finger11,ValveBiped.Bip01_R_Finger12,ValveBiped.Bip01_R_Finger0,ValveBiped.Bip01_R_Finger01,ValveBiped.Bip01_R_Finger02", False
AddToSelection "ValveBiped.SpineControl", , True
AddToSelection "ValveBiped.SpineControl1", , True
AddToSelection "ValveBiped.SpineControl2", , True
AddToSelection "ValveBiped.SpineControl3", , True
AddToSelection "ValveBiped.NeckControl", , True
AddToSelection "ValveBiped.ClavicleControlL", , True
AddToSelection "ValveBiped.ArmRollL", , True
AddToSelection "ValveBiped.HandControlPosL", , True
AddToSelection "ValveBiped.HandControlRotL", , True
AddToSelection "ValveBiped.ClavicleControlR", , True
AddToSelection "ValveBiped.HeadControl", , True
AddToSelection "ValveBiped.ArmRollR", , True
AddToSelection "ValveBiped.HandControlPosR", , True
AddToSelection "ValveBiped.HandControlRotR", , True
AddToSelection "ValveBiped.UpperBody", , True
AddToSelection "ValveBiped.Bip01_Pelvis", , True
AddToSelection "ValveBiped.LegRollL", , True
AddToSelection "ValveBiped.LegRollR", , True
AddToSelection "ValveBiped.LeftFoot", , True
AddToSelection "ValveBiped.LeftToe", , True
AddToSelection "ValveBiped.RightToe", , True
AddToSelection "ValveBiped.RightFoot", , True
AddToSelection "ValveBiped.RightLeg_UpV", , True
AddToSelection "ValveBiped.LeftLeg_UpV", , True
AddToSelection "ValveBiped.RightRoll", , True
AddToSelection "ValveBiped.LeftRoll", , True
AddToSelection "ValveBiped.rArmUpV", , True
AddToSelection "ValveBiped.lArmUpV", , True


CreateGroup
SetValue "Group.Name", "Plotted"
CopyPaste "Plotted", , "ValveBiped", 1
DeselectAll

'--hand pos transform setup

SelectObj "ValveBiped.HandControlPosR", , True
AddToSelection "ValveBiped.HandControlPosL", , True
AddProp "Transform Setup"
SetValue "ValveBiped.HandControlPosR.transformsetup.tool,ValveBiped.HandControlPosL.transformsetup.tool", Array(4, 4)
SetValue "ValveBiped.HandControlPosR.transformsetup.translate,ValveBiped.HandControlPosL.transformsetup.translate", Array(0, 0)
DeselectAll

'--remove roots and effectors from "Envelope_Deformers" group

SelectObj "ValveBiped.LArmRoot"
ToggleSelection "ValveBiped.LHandRoot,ValveBiped.LIndexRoot,ValveBiped.LLegRoot,ValveBiped.LMiddleRoot,ValveBiped.LPinkyRoot,ValveBiped.LRingRoot,ValveBiped.LRootFoot,ValveBiped.LRootShoulder,ValveBiped.LThumbRoot,ValveBiped.Neck_Root,ValveBiped.RArmRoot,ValveBiped.RHandRoot,ValveBiped.RIndexRoot,ValveBiped.RLegRoot,ValveBiped.RMiddleRoot,ValveBiped.RPinkyRoot,ValveBiped.RRingRoot,ValveBiped.RRootFoot,ValveBiped.RRootShoulder,ValveBiped.RThumbRoot,ValveBiped.Spine_Root"
RemoveFromGroup "ValveBiped.Envelope_Deformers", "ValveBiped.LArmRoot,ValveBiped.LHandRoot,ValveBiped.LIndexRoot,ValveBiped.LLegRoot,ValveBiped.LMiddleRoot,ValveBiped.LPinkyRoot,ValveBiped.LRingRoot,ValveBiped.LRootFoot,ValveBiped.LRootShoulder,ValveBiped.LThumbRoot,ValveBiped.Neck_Root,ValveBiped.RArmRoot,ValveBiped.RHandRoot,ValveBiped.RIndexRoot,ValveBiped.RLegRoot,ValveBiped.RMiddleRoot,ValveBiped.RPinkyRoot,ValveBiped.RRingRoot,ValveBiped.RRootFoot,ValveBiped.RRootShoulder,ValveBiped.RThumbRoot,ValveBiped.Spine_Root"
SelectObj "ValveBiped.eff"
ToggleSelection "ValveBiped.eff1,ValveBiped.LArmeff,ValveBiped.LFooteff,ValveBiped.LHandeffector,ValveBiped.LIndexeffector,ValveBiped.LMiddleeffector,ValveBiped.LPinkyeffector,ValveBiped.LRingeffector,ValveBiped.LShouldereff,ValveBiped.LThumbeffector,ValveBiped.LToeeff,ValveBiped.RArmeff,ValveBiped.RFooteff,ValveBiped.RHandeffector,ValveBiped.RIndexeffector,ValveBiped.RMiddleeffector,ValveBiped.RPinkyeffector,ValveBiped.RRingeffector,ValveBiped.RShouldereff,ValveBiped.RThumbeffector,ValveBiped.RToeeff"
RemoveFromGroup "ValveBiped.Envelope_Deformers", "ValveBiped.eff,ValveBiped.eff1,ValveBiped.LArmeff,ValveBiped.LFooteff,ValveBiped.LHandeffector,ValveBiped.LIndexeffector,ValveBiped.LMiddleeffector,ValveBiped.LPinkyeffector,ValveBiped.LRingeffector,ValveBiped.LShouldereff,ValveBiped.LThumbeffector,ValveBiped.LToeeff,ValveBiped.RArmeff,ValveBiped.RFooteff,ValveBiped.RHandeffector,ValveBiped.RIndexeffector,ValveBiped.RMiddleeffector,ValveBiped.RPinkyeffector,ValveBiped.RRingeffector,ValveBiped.RShouldereff,ValveBiped.RThumbeffector,ValveBiped.RToeeff"
DeselectAll

'---Hide legacy controls.  Don't delete them if you plan on using most action presets created to date
SelectObj "ValveBiped.ArmRollR", , True
ToggleSelection "ValveBiped.ArmRollL", , True
ToggleSelection "ValveBiped.HandControlRotR1", , True
ToggleSelection "ValveBiped.HandControlRotL1", , True
ToggleSelection "ValveBiped.LegRollR", , True
ToggleSelection "ValveBiped.LegRollL", , True
ToggleVisibility




 			
end function

'*********************************************
'*********************************************
'*********************************************

' Attach Skeleton

'*********************************************
'*********************************************
'*********************************************
'---------------------------------------------

sub attach_skeleton()

	dim gTool : gTool = "ValveSource"
	dim gToolInstallDir : gToolInstallDir = GetInstallDir(gTool)	

	mpath = gToolInstallDir & "\Data\Models\ValveBiped.emdl"

	ImportModel mpath ,,,myModel
	
	AS_model_and_Bip01_match myModel, valid
	if valid = False then exit sub
	
	AS_Attach_hips myModel, valid
	
	AS_Attach_Bip01_Pelvis myModel, valid
	
	AS_Attach_Hands myModel, valid
	
	AS_attach_feet myModel, valid
	
	attach_feet myModel, valid
	

	
end sub
'---------------------------------------------
' Model and Global SRT Match
'---------------------------------------------
function AS_model_and_Bip01_match(in_rigModel, out_valid)

	dim oRoot, PModel, PgSRT, RiggSRT

	set oRoot = Application.ActiveProject.ActiveScene.Root
	
	set PModel = oRoot.FindChild("ValveGuide")
	set PgSRT= oRoot.FindChild("ValveGuide.GlobalSRT")
	set RiggSRT = oRoot.FindChild(in_rigModel & ".Bip01")
	
	
	'match up the models
	rt_match PModel, in_rigModel
	
	'match up the globalSRT's
	rt_match PgSRT, RiggSRT
	
	out_valid = True
	
end function
'---------------------------------------------
' Attach Hips
'---------------------------------------------
function AS_Attach_hips (in_rigModel, out_valid)

	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0
	SelectObj in_rigModel & ".Bip01_Pelvis", "BRANCH"
	MatchTransform , "ValveGuide.cog", siTrn
	
	
	'now match the top of the legs
	MatchTransform in_rigModel & ".LLegRoot", "ValveGuide.LLeg", siTrn
	MatchTransform in_rigModel & ".RLegRoot", "ValveGuide.RLeg", siTrn
		
	out_valid = True
		
end function
'---------------------------------------------
' Attach Upper Body
'---------------------------------------------
function AS_Attach_Bip01_Pelvis(in_rigModel, out_valid)
	
	MatchTransform in_rigModel & ".Bip01_Pelvis", "ValveGuide.SpineBase", siTrn
	MatchTransform in_rigModel & ".Spine", "ValveGuide.SpineBase", siTrn

	'match the bone lengths
	dim s1, s2, s3, s4

	s1 = GetValue("ValveGuide.bone_3.bone.length")
	s2 = GetValue("ValveGuide.bone_2.bone.length")
	s3 = GetValue("ValveGuide.bone_1.bone.length")
	s4 = GetValue("ValveGuide.bone37.bone.length")

	SetValue in_rigModel & ".Bip01_Spine.bone.length", s1
	SetValue in_rigModel & ".Bip01_Spine1.bone.length", s2
	SetValue in_rigModel & ".Bip01_Spine2.bone.length", s3
	SetValue in_rigModel & ".Bip01_Spine4.bone.length", s4

	'match the positions
	dim c1, c2, c3, c4
	c1 = ApplyCns ("Position", in_rigModel & ".Bip01_Spine1", "ValveGuide.Spine1")
	c2 = ApplyCns ("Position", in_rigModel & ".Bip01_Spine2", "ValveGuide.Spine2")
	c3 = ApplyCns ("Position", in_rigModel & ".Bip01_Spine4", "ValveGuide.Spine3")
	c4 = ApplyCns ("Position", in_rigModel & ".eff", "ValveGuide.RibCageEnd")

	refresh
	
	DeleteObj c1
	DeleteObj c2	
	DeleteObj c3
	DeleteObj c4

	dim b1,b2,b3,b4,b5,b6,b7,b8
	
	'head
	b1 = GetValue("ValveGuide.bone35.bone.length")
	b2 = GetValue("ValveGuide.bone36.bone.length")

	SetValue in_rigModel & ".Bip01_Neck1.bone.length", b1
	SetValue in_rigModel & ".Bip01_Head1.bone.length", b2
	
	c1 = ApplyCns ("Position", in_rigModel & ".Bip01_Neck1", "ValveGuide.RibCageEnd")
	c2 = ApplyCns ("Position", in_rigModel & ".Bip01_Head1", "ValveGuide.Neck")
	c3 = ApplyCns ("Position", in_rigModel & ".eff1", "ValveGuide.Head")
	
	refresh
	
	DeleteObj c1
	DeleteObj c2	
	DeleteObj c3
		
	'arms

	
	b3 = GetValue("ValveGuide.bone32.bone.length")
	b4 = GetValue("ValveGuide.bone33.bone.length")
	b5 = GetValue("ValveGuide.bone34.bone.length")
	b6 = GetValue("ValveGuide.bone31.bone.length")
	b7 = GetValue("ValveGuide.bone30.bone.length")	
	b8 = GetValue("ValveGuide.bone29.bone.length")
		
	
	SetValue in_rigModel & ".Bip01_R_Clavicle.bone.length", b3
	SetValue in_rigModel & ".Bip01_R_UpperArm.bone.length", b4
	SetValue in_rigModel & ".Bip01_R_Forearm.bone.length", b5
	SetValue in_rigModel & ".Bip01_L_Clavicle.bone.length", b6
	SetValue in_rigModel & ".Bip01_L_UpperArm.bone.length", b7
	SetValue in_rigModel & ".Bip01_L_Forearm.bone.length", b8
	
	
	
	c1 = ApplyCns ("Position", in_rigModel & ".RRootShoulder", "ValveGuide.RShoulderStart")
	c2 = ApplyCns ("Position", in_rigModel & ".RShoulderEff", "ValveGuide.RShoulder")
	c3 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Forearm", "ValveGuide.RElbow")
	c4 = ApplyCns ("Position", in_rigModel & ".RArmEff", "ValveGuide.RHand")
	
	c5 = ApplyCns ("Position", in_rigModel & ".LRootShoulder", "ValveGuide.LShoulderStart")
	c6 = ApplyCns ("Position", in_rigModel & ".LShoulderEff", "ValveGuide.LShoulder")
	c7 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Forearm", "ValveGuide.LElbow")
	c8 = ApplyCns ("Position", in_rigModel & ".LArmEff", "ValveGuide.LHand")	
	
	refresh

	Align_Arms in_rigModel 


	DeleteObj c1
	DeleteObj c2
	DeleteObj c3
	DeleteObj c4
	DeleteObj c5
	DeleteObj c6	
	DeleteObj c7	
	DeleteObj c8		

	out_valid = True
	
end function
'---------------------------------------------
' Attach Hands
'---------------------------------------------
function AS_Attach_Hands(in_rigModel, out_valid)



	out_valid = False
	dim c1
	
	'right hand

	'match the bone lengths
	dim b0, b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15
	
	b0 = GetValue("ValveGuide.bone62.bone.length")	
	
	b1 = GetValue("ValveGuide.bone46.bone.length")
	b2 = GetValue("ValveGuide.bone47.bone.length")
	b3 = GetValue("ValveGuide.bone48.bone.length")
	b4 = GetValue("ValveGuide.bone49.bone.length")
	b5 = GetValue("ValveGuide.bone50.bone.length")
	b6 = GetValue("ValveGuide.bone51.bone.length")
	b7 = GetValue("ValveGuide.bone52.bone.length")	
	b8 = GetValue("ValveGuide.bone53.bone.length")
	b9 = GetValue("ValveGuide.bone54.bone.length")
	b10 = GetValue("ValveGuide.bone55.bone.length")
	b11 = GetValue("ValveGuide.bone56.bone.length")
	b12 = GetValue("ValveGuide.bone57.bone.length")
	b13 = GetValue("ValveGuide.bone58.bone.length")
	b14 = GetValue("ValveGuide.bone59.bone.length")
	b15 = GetValue("ValveGuide.bone60.bone.length")
	
	
	SetValue in_rigModel & ".Bip01_R_Hand.bone.length", b0
	
	SetValue in_rigModel & ".Bip01_R_Finger0.bone.length", b1*.5
	SetValue in_rigModel & ".Bip01_R_Finger01.bone.length", b2*.5
	SetValue in_rigModel & ".Bip01_R_Finger02.bone.length", b3*.5
	SetValue in_rigModel & ".Bip01_R_Finger1.bone.length", b4*.5
	SetValue in_rigModel & ".Bip01_R_Finger11.bone.length", b5*.5
	SetValue in_rigModel & ".Bip01_R_Finger12.bone.length", b6*.5
	SetValue in_rigModel & ".Bip01_R_Finger2.bone.length", b7*.5
	SetValue in_rigModel & ".Bip01_R_Finger21.bone.length", b8*.5
	SetValue in_rigModel & ".Bip01_R_Finger22.bone.length", b9*.5
	SetValue in_rigModel & ".Bip01_R_Finger3.bone.length", b10*.5
	SetValue in_rigModel & ".Bip01_R_Finger31.bone.length", b11*.5
	SetValue in_rigModel & ".Bip01_R_Finger31.bone.length", b12*.5
	SetValue in_rigModel & ".Bip01_R_Finger4.bone.length", b13*.5
	SetValue in_rigModel & ".Bip01_R_Finger41.bone.length", b14*.5
	SetValue in_rigModel & ".Bip01_R_Finger42.bone.length", b15*.5
	

	
	Rotate in_rigModel & ".Bip01_R_Hand", 0, 0, 0, siAbsolute, 70, siObj, siXYZ
	set c0 = ApplyCns ("Position", in_rigModel & ".RHandEffector", "ValveGuide.RHandEnd")	
	
	MatchTransform in_rigModel & ".RThumbRoot", "ValveGuide.RThumb1", siTrn
	MatchTransform in_rigModel & ".RIndexRoot", "ValveGuide.RIndex1", siTrn
	MatchTransform in_rigModel & ".RMiddleRoot", "ValveGuide.RMiddle1", siTrn
	MatchTransform in_rigModel & ".RRingRoot", "ValveGuide.RRing1", siTrn
	MatchTransform in_rigModel & ".RPinkyRoot", "ValveGuide.RPinky1", siTrn	


	dim c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16

	set c2 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger01", "ValveGuide.RThumb2")
	set c3 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger02", "ValveGuide.RThumb3")
	set c4 = ApplyCns ("Position", in_rigModel & ".RThumbEffector", "ValveGuide.RThumb4")

	set c5 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger11", "ValveGuide.RIndex2")
	set c6 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger12", "ValveGuide.RIndex3")
	set c7 = ApplyCns ("Position", in_rigModel & ".RIndexEffector", "ValveGuide.RIndex4")

	set c8 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger21", "ValveGuide.RMiddle2")
	set c9 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger22", "ValveGuide.RMiddle3")
	set c10 = ApplyCns ("Position", in_rigModel & ".RMiddleEffector", "ValveGuide.RMiddle4")


	set c11 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger31", "ValveGuide.RRing2")
	set c12 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger32", "ValveGuide.RRing3")
	set c13 = ApplyCns ("Position", in_rigModel & ".RRingEffector", "ValveGuide.RRing4")

	set c14 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger41", "ValveGuide.RPinky2")
	set c15 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Finger42", "ValveGuide.RPinky3")
	set c16 = ApplyCns ("Position", in_rigModel & ".RPinkyEffector", "ValveGuide.RPinky4")
	
	

	
	refresh
	
	DeleteObj c0	
	
	DeleteObj c2	
	DeleteObj c3
	DeleteObj c4
	DeleteObj c5	
	DeleteObj c6	
	DeleteObj c7
	DeleteObj c8	
	DeleteObj c9
	DeleteObj c10
	DeleteObj c11	
	DeleteObj c12
	DeleteObj c13 
	DeleteObj c14	
	DeleteObj c15
	DeleteObj c16

	'left hand

	'match the bone lengths
	
	b0 = GetValue("ValveGuide.bone62.bone.length")	
	
	b1 = GetValue("ValveGuide.bone.bone.length")
	b2 = GetValue("ValveGuide.bone15.bone.length")
	b3 = GetValue("ValveGuide.bone16.bone.length")
	b4 = GetValue("ValveGuide.bone19.bone.length")
	b5 = GetValue("ValveGuide.bone18.bone.length")
	b6 = GetValue("ValveGuide.bone17.bone.length")
	
	b7 = GetValue("ValveGuide.bone20.bone.length")	
	b8 = GetValue("ValveGuide.bone21.bone.length")
	b9 = GetValue("ValveGuide.bone22.bone.length")
	
	b10 = GetValue("ValveGuide.bone23.bone.length")
	b11 = GetValue("ValveGuide.bone24.bone.length")
	b12 = GetValue("ValveGuide.bone25.bone.length")
	
	b13 = GetValue("ValveGuide.bone26.bone.length")
	b14 = GetValue("ValveGuide.bone27.bone.length")
	b15 = GetValue("ValveGuide.bone28.bone.length")
	
	
	SetValue in_rigModel & ".Bip01_L_Hand.bone.length", b0
	
	SetValue in_rigModel & ".Bip01_L_Finger0.bone.length", b1*.5
	SetValue in_rigModel & ".Bip01_L_Finger01.bone.length", b2*.5
	SetValue in_rigModel & ".Bip01_L_Finger02.bone.length", b3*.5
	SetValue in_rigModel & ".Bip01_L_Finger1.bone.length", b4*.5
	SetValue in_rigModel & ".Bip01_L_Finger11.bone.length", b5*.5
	SetValue in_rigModel & ".Bip01_L_Finger12.bone.length", b6*.5
	SetValue in_rigModel & ".Bip01_L_Finger2.bone.length", b7*.5
	SetValue in_rigModel & ".Bip01_L_Finger21.bone.length", b8*.5
	SetValue in_rigModel & ".Bip01_L_Finger22.bone.length", b9*.5
	SetValue in_rigModel & ".Bip01_L_Finger3.bone.length", b10*.5
	SetValue in_rigModel & ".Bip01_L_Finger31.bone.length", b11*.5
	SetValue in_rigModel & ".Bip01_L_Finger31.bone.length", b12*.5
	SetValue in_rigModel & ".Bip01_L_Finger4.bone.length", b13*.5
	SetValue in_rigModel & ".Bip01_L_Finger41.bone.length", b14*.5
	SetValue in_rigModel & ".Bip01_L_Finger42.bone.length", b15*.5
	


	set c0 = ApplyCns ("Position", in_rigModel & ".LHandEffector", "ValveGuide.LHandEnd")	

	refresh

	DeleteObj c0

	MatchTransform in_rigModel & ".LThumbRoot", "ValveGuide.LThumb1", siTrn
	MatchTransform in_rigModel & ".LIndexRoot", "ValveGuide.LIndex1", siTrn
	MatchTransform in_rigModel & ".LMiddleRoot", "ValveGuide.LMiddle1", siTrn
	MatchTransform in_rigModel & ".LRingRoot", "ValveGuide.LRing1", siTrn
	MatchTransform in_rigModel & ".LPinkyRoot", "ValveGuide.LPinky1", siTrn	


		

	set c2 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger01", "ValveGuide.LThumb2")
	set c3 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger02", "ValveGuide.LThumb3")
	set c4 = ApplyCns ("Position", in_rigModel & ".LThumbEffector", "ValveGuide.LThumb4")

	set c5 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger11", "ValveGuide.LIndex2")
	set c6 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger12", "ValveGuide.LIndex3")
	set c7 = ApplyCns ("Position", in_rigModel & ".LIndexEffector", "ValveGuide.LIndex4")

	set c8 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger21", "ValveGuide.LMiddle2")
	set c9 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger22", "ValveGuide.LMiddle3")
	set c10 = ApplyCns ("Position", in_rigModel & ".LMiddleEffector", "ValveGuide.LMiddle4")


	set c11 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger31", "ValveGuide.LRing2")
	set c12 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger32", "ValveGuide.LRing3")
	set c13 = ApplyCns ("Position", in_rigModel & ".LRingEffector", "ValveGuide.LRing4")

	set c14 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger41", "ValveGuide.LPinky2")
	set c15 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Finger42", "ValveGuide.LPinky3")
	set c16 = ApplyCns ("Position", in_rigModel & ".LPinkyEffector", "ValveGuide.LPinky4")
	
	refresh
	
	
	DeleteObj c2	
	DeleteObj c3
	DeleteObj c4
	DeleteObj c5	
	DeleteObj c6	
	DeleteObj c7
	DeleteObj c8	
	DeleteObj c9
	DeleteObj c10
	DeleteObj c11	
	DeleteObj c12
	DeleteObj c13 
	DeleteObj c14	
	DeleteObj c15
	DeleteObj c16

	out_valid = True
	
end function


'---------------------------------------------
' Attach Feet
'---------------------------------------------
function AS_attach_feet(in_rigModel, out_valid)

	out_valid = False

	'match the bone lengths
	dim b1,b2,b3,b4,b5,b6,b7,b8
	b1 = GetValue("ValveGuide.bone42.bone.length")
	b2 = GetValue("ValveGuide.bone43.bone.length")
	b3 = GetValue("ValveGuide.bone44.bone.length")
	b4 = GetValue("ValveGuide.bone45.bone.length")
	b5 = GetValue("ValveGuide.bone38.bone.length")
	b6 = GetValue("ValveGuide.bone39.bone.length")
	b7 = GetValue("ValveGuide.bone40.bone.length")	
	b8 = GetValue("ValveGuide.bone61.bone.length")
	
	SetValue in_rigModel & ".Bip01_R_Thigh.bone.length", b1
	SetValue in_rigModel & ".Bip01_R_Calf.bone.length", b2
	SetValue in_rigModel & ".Bip01_R_Foot.bone.length", b3
	SetValue in_rigModel & ".Bip01_R_Toe0.bone.length", b4
	SetValue in_rigModel & ".Bip01_L_Thigh.bone.length", b5
	SetValue in_rigModel & ".Bip01_L_Calf.bone.length", b6
	SetValue in_rigModel & ".Bip01_L_Foot.bone.length", b7
	SetValue in_rigModel & ".Bip01_L_Toe0.bone.length", b8
												
												
 	'match the foot postition
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0
	
	SelectObj in_rigModel & ".RLegRoot", "BRANCH"
	MatchTransform , "ValveGuide.RLeg", siTrn
	
	SelectObj in_rigModel & ".LLegRoot", "BRANCH"
	MatchTransform , "ValveGuide.LLeg", siTrn		
										
	set c1 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Calf", "ValveGuide.RKnee")
	set c2 = ApplyCns ("Position", in_rigModel & ".RFootEff", "ValveGuide.RFoot")
	set c3 = ApplyCns ("Position", in_rigModel & ".Bip01_R_Toe0", "ValveGuide.RHeel")
	set c4 = ApplyCns ("Position", in_rigModel & ".RToeEff", "ValveGuide.RToe")
	
	set c5 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Calf", "ValveGuide.LKnee")
	set c6 = ApplyCns ("Position", in_rigModel & ".LFootEff", "ValveGuide.LFoot")
	set c7 = ApplyCns ("Position", in_rigModel & ".Bip01_L_Toe0", "ValveGuide.LHeel")
	set c8 = ApplyCns ("Position", in_rigModel & ".LToeEff", "ValveGuide.LToe")
	refresh
	
	Align_Legs in_rigModel
		
	
	
	DeleteObj c1
	DeleteObj c2
	DeleteObj c3
	DeleteObj c4	
	DeleteObj c5
	DeleteObj c6
	DeleteObj c7
	DeleteObj c8	
		
end function
'---------------------------------------------
' FK Prep
'---------------------------------------------
function AS_fk_prep(in_model)
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 1
	
	root_Alignment in_model, "SpineRoot"
	root_Alignment in_model, "Spine"	
	
	'shoulders
	root_Alignment in_model, "Bip01_Spine4"
	root_Alignment in_model, "Bip01_L_Finger22"	
	
	'hands
	root_Alignment in_model, "Bip01_R_Clavicle"	
		
		
	root_Alignment in_model, "Bip01_R_Finger12"	
	root_Alignment in_model, "Bip01_R_Finger02"	
	root_Alignment in_model, "RThumbEffector"	
	root_Alignment in_model, "Bip01_L_Finger41"	
	root_Alignment in_model, "Bip01_R_UpperArm"						
			
	root_Alignment in_model, "Bip01_L_Clavicle"

	root_Alignment in_model, "Bip01_L_UpperArm"	
	root_Alignment in_model, "Bip01_Neck1"	
	root_Alignment in_model, "LThumbEffector"	
	root_Alignment in_model, "Bip01_L_Finger02"
	root_Alignment in_model, "Bip01_L_Finger12"	
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0	
	
end function
'---------------------------------------------
' Root Alignment
'---------------------------------------------
function root_Alignment(in_model, in_rootstring)

	in_root = in_model.FindChild(in_rootstring)

	dim mySkeleton, BoneIndex, rroot, rbone, zero, newrot
	set mySkeleton =  GetSkeleton(in_root, BoneIndex ) 	
	
	set boneRot = XSIMath.CreateRotation
	set zero = XSIMath.CreateRotation
		
	mySkeleton(1).Kinematics.local.Transform.GetRotation boneRot
	boneRot.GetXYZAnglesValues x,y,z

	Rotate mySkeleton(0), 0, 0, XSIMath.RadiansToDegrees(z), siRelative, siLocal, siObj, siXYZ
	Rotate mySkeleton(0), 0, XSIMath.RadiansToDegrees(y), 0, siRelative, siLocal, siObj, siXYZ
	Rotate mySkeleton(0), XSIMath.RadiansToDegrees(x), 0, 0, siRelative, siLocal, siObj, siXYZ

	set boneTrans = mySkeleton(1).Kinematics.local.Transform
	zero.SetFromXYZAnglesValues 0,0,0 	
	
	boneTrans.SetRotation zero 
	
	mySkeleton(1).Kinematics.local.Transform = boneTrans 
		
end function
'---------------------------------------------
' Is Component 
'---------------------------------------------
function IsComponent(in_string)
	
	dim oRoot
	IsComponent = True
	set oRoot = Application.ActiveProject.ActiveScene.Root
	if typename(oRoot.FindChild(in_string)) = "Nothing" then IsComponent = False
	
end function
'--------------------------------------
' RT Match
'--------------------------------------
function rt_match (in_source, in_target)

	if (typename(in_source) = "Nothing") then
		logmessage "rt_match: invalid source"
		rt_match = False
		exit function
	end if
	if (typename(in_target) = "Nothing") then
		logmessage "rt_match: invalid target"
		rt_match = False
		exit function
	end if
	
	logmessage "SOURCE: " & in_source
	logmessage "TARGET: " & in_target	


	dim sx, sy, sz
	dim rx, ry, rz	
	dim tx, ty, tz
	
	sx = GetValue(in_source & ".kine.global.sclx")
	sy = GetValue(in_source & ".kine.global.scly")	
	sz = GetValue(in_source & ".kine.global.sclz")
	
	rx = GetValue(in_source & ".kine.global.rotx")
	ry = GetValue(in_source & ".kine.global.roty")
	rz = GetValue(in_source & ".kine.global.rotz")
	
	tx = GetValue(in_source & ".kine.global.posx")
	ty = GetValue(in_source & ".kine.global.posy")
	tz = GetValue(in_source & ".kine.global.posz")
	
	SelectObj in_target, "BRANCH"

	Scale , sx, sy, sz, siAbsolute, siGlobal, siObj, siXYZ
	Rotate , rx, ry, rz, siAbsolute, siGlobal, siObj, siXYZ
	Translate , tx, ty, tz, siAbsolute, siGlobal, siObj, siXYZ
	
end function
'-----------------------------------------------
' Set Prefangles
'-----------------------------------------------
function set_prefangles(in_bone)
	 
	' set the rotations to pref angles. 
	
	dim pref1, pref2, pref3, chain, bIndex
	
	set chain = GetSkeleton(in_bone, bIndex)
	
	'if not the root or the first bone set the pref of one back
	
	if bIndex > 1 then
		set pref1 = createobject("Sumatra\Scripting\Math\SIVector3")
		GetVector chain(bIndex -1) & ".kine.local.rot", pref1
		SetValue chain(bIndex - 1) &".joint.prefrotx"	, pref1.x
		SetValue chain(bIndex - 1) &".joint.prefroty"	, pref1.y
		SetValue chain(bIndex - 1) &".joint.prefrotz"	, pref1.z			
	end if	

	
	' if in_bone is not an effector set itself.
	
	if bIndex < chain.count - 1 then
		set pref2 = createobject("Sumatra\Scripting\Math\SIVector3")
		GetVector chain(bIndex) & ".kine.local.rot", pref2
		SetValue chain(bIndex) &".joint.prefrotx"	, pref2.x
		SetValue chain(bIndex) &".joint.prefroty"	, pref2.y
		SetValue chain(bIndex) &".joint.prefrotz"	, pref2.z	
	end if
	
	
	' if bIndex + 1 is beyond the size of the chain or and effector set
	' the pref angles
		
	if bIndex < chain.count - 2 then
		set pref3 = createobject("Sumatra\Scripting\Math\SIVector3")  
		GetVector chain(bIndex + 1) & ".kine.local.rot", pref3
		SetValue chain(bIndex + 1) &".joint.prefrotx"	, pref3.x
		SetValue chain(bIndex + 1) &".joint.prefroty"	, pref3.y
		SetValue chain(bIndex + 1) &".joint.prefrotz"	, pref3.z	
	end if	
	
	
end function

'-----------------------------------------------
' Get Vector
'-----------------------------------------------
function GetVector(in_Path, out_vector)

	'Create a vector containing the x y z component of the path
	'for example  GetValueVector(obj & "kine.local.pos", frame)
	Dim V
	set V = createobject("Sumatra\Scripting\Math\SIVector3")
	V.Set GetValue(in_Path & "x"),_
	GetValue(in_Path & "y"),_
	GetValue(in_Path & "z")
	set out_vector = V
end function
'---------------------------------------------
' Attach Feet
'---------------------------------------------
function attach_feet(in_rigModel, out_valid)

	out_valid = False
	

 	'match the foot postition
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0
	
	SelectObj in_rigModel & ".LeftFootParent", "BRANCH"
	MatchTransform , "ValveGuide.LFoot", siTrn
	
	SelectObj in_rigModel & ".RightFootParent", "BRANCH"
	MatchTransform , "ValveGuide.RFoot", siTrn											
		

	set root = ActiveProject.ActiveScene.Root								
	dim lFoot, lHeel, rFoot, rHeel
	
	'match the left rig
	
	set lFoot = root.FindChild(in_rigModel & ".LeftFoot")
	set lHeel = root.FindChild("ValveGuide.LHeel")
	align_foot lFoot, lHeel 
	
	SelectObj in_rigModel & ".heel", "BRANCH", True
	MatchTransform , "ValveGuide.LHeelPivot", siTrn
	MatchTransform in_rigModel & ".toe", "ValveGuide.LToe", siTrn
	SelectObj in_rigModel & ".mid", "BRANCH", True									
	MatchTransform , "ValveGuide.LHeel", siTrn																		
	MatchTransform in_rigModel & ".ankle", "ValveGuide.LFoot", siTrn	
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 1
	refresh
	MatchTransform in_rigModel & ".LeftToe", "ValveGuide.LHeel", siTrn
	MatchTransform in_rigModel & ".LToeEff", "ValveGuide.LToe", siTrn	
	
	DeleteObj in_rigModel & ".LeftToeTarget.kine.posecns"
	MatchTransform in_rigModel & ".LeftToeTarget", "ValveGuide.LToe", siTrn
	ApplyCns "Pose", in_rigModel & ".LeftToeTarget", in_rigModel & ".LeftToe", True	
	
	DeleteObj in_rigModel & ".LToeEff.kine.poscns"	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0	
	ApplyCns "Position", in_rigModel & ".LToeEff", in_rigModel & ".LeftToeTarget"
	
	

	'match the right rig
	set rFoot = root.FindChild(in_rigModel & ".RightFoot")
	set rHeel = root.FindChild("ValveGuide.RHeel")
	align_foot rFoot, rHeel 
	
	SelectObj in_rigModel & ".heel1", "BRANCH", True
	MatchTransform , "ValveGuide.RHeelPivot", siTrn
	MatchTransform in_rigModel & ".toe1", "ValveGuide.RToe", siTrn
	SelectObj in_rigModel & ".mid1", "BRANCH", True									
	MatchTransform , "ValveGuide.RHeel", siTrn																		
	MatchTransform in_rigModel & ".ankle1", "ValveGuide.RFoot", siTrn	
	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 1
	refresh
	MatchTransform in_rigModel & ".RightToe", "ValveGuide.RHeel", siTrn
	MatchTransform in_rigModel & ".RToeEff", "ValveGuide.RToe", siTrn	
	
	DeleteObj in_rigModel & ".RightToeTarget.kine.posecns"
	MatchTransform in_rigModel & ".RightToeTarget", "ValveGuide.RToe", siTrn
	ApplyCns "Pose", in_rigModel & ".RightToeTarget", in_rigModel & ".RightToe", True	
	
	DeleteObj in_rigModel & ".RToeEff.kine.poscns"	
	setuserpref("SI3D_CONSTRAINT_COMPENSATION_MODE"), 0	
	ApplyCns "Position", in_rigModel & ".RToeEff", in_rigModel & ".RightToeTarget"	
		

	out_valid = True
	
end function
'------------------------------------
' Align Foot
'------------------------------------
function align_foot(in_foot, in_heel)

	dim Pose1, dCnst, Pose2, yRot
	
	set Pose1 = in_foot.Kinematics.Global.Transform

	' Point the foot at the Toe
	set dCnst = in_foot.Kinematics.AddConstraint("Direction", in_heel, False)
	dCnst.dirx = 0
	dCnst.dirz = 1
	dCnst.upvct_active = True

	'now calculate the rotation to get there only rotating on the global y axis

	set Pose2 = in_foot.Kinematics.Global.Transform

	get_y_rotation Pose2, Pose1, yRot


	DeleteObj dCnst
	in_foot.Kinematics.Global.Transform = Pose1

	if yRot > 90 then yRot = yRot - 180
	if yRot < -90 then yRot = yRot + 180
	
	Rotate in_foot, 0, yRot, 0, siAbsolute, 65, siObj, siY

end function
'------------------------------------
' Get Y rotation
'------------------------------------
' We're not completly matching the rotations because 
' small variations of the foot angle up or down 
' shouldn't be used to rotate the body 
'....................................
function get_y_rotation(in_t1, in_t2, out_yRot)
	
	dim pushOut, gv1, gv2, gt1, gt2, cp ,angle, sroot
	
	set sRoot = Application.ActiveProject.ActiveScene.Root	
	
	set pushOut = xsimath.CreateVector3 
	pushOut.set 5,0,0
	
	'get the pushOut vector from local cordinates to a
	'global position
	
	set gv1 = XSIMath.MapObjectPositionToWorldSpace(in_t1,pushOut)  
	set gv2 = XSIMath.MapObjectPositionToWorldSpace(in_t2,pushOut) 
	
	'get the global translation to bring both vectors back to the origion
	set gt1 = xsimath.CreateVector3 
	set gt2 = xsimath.CreateVector3 
	in_t1.getTranslation gt1 
	in_t2.getTranslation gt2 

	gv1.SubInPlace gt1 
	gv2.SubInPlace gt2 
	
	'now project those vectors onto the xz plane	
	gv1.y = 0
	gv2.y = 0
	
	'now get the angle between them
	
	angle = gv2.Angle(gv1)  

	'get the cross product
	set cp = xsimath.CreateVector3 
	cp.cross gv1, gv2
	
	out_yRot = XSIMath.RadiansToDegrees(angle)

	if cp.y > 0 then out_yRot = out_yRot * -1 
	
end function
'-----------------------------------
' Handle Orientation
'-----------------------------------
function LegHandleOrientation(inSkeleton, inHandle, inDiagnostic)
	
	' get the normal for the Handle orientation plane
	' on this skeleton it should be the x+ axis of the 
	' up vector rotation handle
	
	set HandlePlaneStart = XSIMath.CreateVector3
	set HandleTransform = inHandle.parent.Kinematics.Global.Transform
	HandleTransform.GetTranslation HandlePlaneStart
	set ZAxis = XSIMath.CreateVector3
	ZAxis.set 0,-1,0
	HandleTransform.AddLocalTranslation ZAxis
	set HandlePlaneEnd = XSIMath.CreateVector3
	HandleTransform.GetTranslation HandlePlaneEnd 
	set HandlePlane = XSIMath.CreateVector3
	HandlePlane.sub HandlePlaneEnd, HandlePlaneStart



	'Also get the zero point for the HandleParent
	'in this case it's the -z axis (0,0,-1)
	
	set ZeroTrans = inHandle.parent.Kinematics.Global.Transform
	set ZeroOri = XSIMath.CreateVector3
	ZeroOri.set 0,0,-1
	ZeroTrans.AddLocalTranslation ZeroOri
	set ZeroEnd = XSIMath.CreateVector3
	ZeroTrans.GetTranslation ZeroEnd
	set ZeroVector = XSIMath.CreateVector3
	ZeroVector.Sub ZeroEnd, HandlePlaneStart
	

	'now get the plane for the upvector
	ProcessUpV inSkeleton, inDiagnostic, UpVPlane, FirstBoneYplus
	
	'now get the projection of the UpVectorPlane onto the HandlePlane
	set Projection = XSIMath.CreateVector3
	Projection.cross UpVPlane, HandlePlane


	' when taking the vector between the two planes you need to pick
	' the one that closest matches the Y plus vector coming off the
	' first bone.
	
	set NegProjection = XSIMath.CreateVector3
	NegProjection.Negate Projection
	
	ArmDot1 = Projection.Dot(FirstBoneYplus)
	ArmDot2 = NegProjection.Dot(FirstBoneYplus)
	

		
	if ArmDot1 < .0001 and ArmDot1 > -.0001 then exit function
	
	if ArmDot2 > ArmDot1 then
		set Projection = NegProjection
	end if
	
	
	'now get the angle between the Projection and the ZeroVector
	
	angle = ZeroVector.Angle(Projection)
	angle = XSIMath.RadiansToDegrees(angle)
	
	'find the sign of the angle
	
	set SignCp = xsimath.CreateVector3 
	SignCp.cross ZeroVector, Projection
	
	'the sign can be determined by the projection of SignCp onto the handlePlane
	
	if SignCp.Dot(HandlePlane) > 0 then
		sign = -1
	else	
		sign = 1
	end if
	
	SetValue inHandle & ".kine.local.roty", angle * sign
	
	
	if inDiagnostic = True then

		logmessage "DOT WITH PROJECTION: " & ArmDot1
		logmessage "DOT WITH NEG PROJECTION: " & ArmDot2

		set FirstBoneYplusDraw = XSIMath.CreateVector3
		FirstBoneYplus.AddInPlace HandlePlaneStart
		DrawLine2 HandlePlaneStart, FirstBoneYplus, "FirstBoneYplus"	

		HandlePlane.ScaleInPlace 10
		set HandlePlaneDraw = XSIMath.CreateVector3
		HandlePlaneDraw.add HandlePlaneStart, HandlePlane
		DrawLine2 HandlePlaneStart, HandlePlaneDraw, "HandlePlane"
		
		ZeroVector.ScaleInPlace 10
		set ZeroVectorDraw = XSIMath.CreateVector3
		ZeroVectorDraw.add HandlePlaneStart, ZeroVector
		DrawLine2 HandlePlaneStart, ZeroVectorDraw, "ZeroVector"
		
		set ProjectionDraw = XSIMath.CreateVector3
		ProjectionDraw.add HandlePlaneStart, Projection
		DrawLine2 HandlePlaneStart, ProjectionDraw, "Projection"					
		
	end if
	
	
end function
'-----------------------------------
' Process UpV
'-----------------------------------
function ProcessUpV(inSkel, inDiagnostic, outUpVectorPlane, outFirstBoneYplus)
	
	set inBone = inSkel(1)
	
	'first get the global transform
	
	set Trans = inBone.Kinematics.Global.Transform
	set startPoint = XSIMath.CreateVector3
	Trans.GetTranslation startPoint
	
	'get the bone length
	length = inBone.length.value
	
	'get the bone roll
	roll = GetValue(inBone & ".joint.roll")
	
	set EffPointAtTrans = RootToEffectorTrans(inSkel)
	

	'spin back the Trans as though it wasn't affected by the bone roll
	
	set Trans = RotationByReference(EffPointAtTrans, Trans , 0, -roll, 0)
	

	' get the first Bone y+ (used to figure out which side of the two plane
	' intesection is the right orientation).
 	set pos = XSIMath.CreateVector3
	pos.Set 0, 7, 0
	set Trans2 = XSIMath.CreateTransform
	Trans2.Copy Trans
	Trans2.AddLocalTranslation pos	
	set FirstBoneYplus= XSIMath.CreateVector3
	Trans2.GetTranslation FirstBoneYplus
	FirstBoneYplus.SubInPlace startPoint
	set outFirstBoneYplus = FirstBoneYplus

	' Get the position of the effector
	set UpVEnd = XSIMath.CreateVector3
	inSkel(inSkel.count - 1).Kinematics.Global.Transform.GetTranslation UpVEnd	

	'get the End the of first Bone Vector, in the space of the removed roll
	set Bpos = XSIMath.CreateVector3
	set BoneEnd = XSIMath.CreateVector3
	Bpos.Set length, 0, 0
	Trans.AddLocalTranslation Bpos	
	Trans.GetTranslation BoneEnd 

	'now get the two vectors for the plane of the bone
	set V1 = XSIMath.CreateVector3
	set V2 = XSIMath.CreateVector3
	
	V1.sub BoneEnd, startPoint
	V2.sub UpVEnd, startPoint
		
	set UpVectorPlane = XSIMath.CreateVector3
	UpVectorPlane.Cross V1, V2

	if inDiagnostic = True then
		
			DrawLine startPoint, BoneEnd, "BonePlane"
			DrawLine startPoint, UpVEnd, "UpVLine"	
			
			UpVectorPlane.NormalizeInPlace
			UpVectorPlane.ScaleInPlace 5
			set UpVectorPlaneOnArm = XSIMath.CreateVector3
			UpVectorPlaneOnArm.add startPoint, UpVectorPlane
			DrawLine startPoint, UpVectorPlaneOnArm, "UpVectorPlane"	
	end if
	
	set outUpVectorPlane = UpVectorPlane
	
end function
'-----------------------------------
' Root To Effector Trans
'-----------------------------------
function RootToEffectorTrans(inSkel)

	'Get a Transform Describing the root pointing towards the effector
	
	set inEff = inSkel(0)
	set inRoot = inSkel(inSkel.count - 1)
	
	set startAxis = XSIMath.CreateVector3	
	set endAxis = XSIMath.CreateVector3
	set Axis = XSIMath.CreateVector3

	inEff.Kinematics.Global.Transform.GetTranslation startAxis
	inRoot.Kinematics.Global.Transform.GetTranslation endAxis	
	Axis.sub startAxis, endAxis
		
	set PointAxis = XSIMath.CreateVector3	
	set cp  = XSIMath.CreateVector3	
	
	PointAxis.set 0,-1,0 
	angle = Axis.Angle(PointAxis)
	cp.cross PointAxis, Axis
	
	set Orot = XSIMath.CreateRotation	
	Orot.SetFromAxisAngle cp, angle
	
	set r1 = XSIMath.CreateVector3	
	Orot.GetXYZAngles r1
		
	'Create a Transform for the Return
	set returnTrans = XSIMath.CreateTransform
	returnTrans.SetTranslation endAxis
	returnTrans.SetRotation Orot

	set RootToEffectorTrans = returnTrans
	
end function
'-------------------------------------
' Rotation By Reference
'-------------------------------------
' inRef: the transform of Rotating Reference
' inObj: the transform to rotate in the space of inRef
' inX, inY, inZ: the rotation
'~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function RotationByReference(inRef, inObj, inX, inY, inZ)

	'Describe the Object in the Space of the Reference
	set RelativeObj = XSIMath.MapWorldPoseToObjectSpace(inRef, inObj)
	
	set rot = XSIMath.CreateRotation
	
	inX = XSIMath.DegreesToRadians(inX)
	inY = XSIMath.DegreesToRadians(inY)	
	inZ = XSIMath.DegreesToRadians(inZ)
		
	rot.SetFromXYZAnglesValues inX, inY, inZ
	
	RelativeObj.AddParentRotation rot
	
	'now put the position back into global space
	set returnTrans = XSIMath.MapObjectPoseToWorldSpace(inRef, RelativeObj)
	
	set RotationByReference = returnTrans 
	
end function
'-------------------------------------
' Draw Line
'-------------------------------------
function DrawLine(inStart, inEnd, inName)

	set myCurve = SICreateCurve ("crvlist", 1, 1)
	SISetCurvePoints myCurve, "("&inStart.x&","&inStart.y&","&inStart.z&"),("&inEnd.x&","&inEnd.y&","&inEnd.z&")", 0
	
	myCurve.name = inName
	myCurve.AddProperty("Display Property")
	myCurve.properties("display").wirecol.value = 507
	
end function
'-------------------------------------
' Draw Line 2
'-------------------------------------
function DrawLine2(inStart, inEnd, inName)

	set myCurve = SICreateCurve ("crvlist", 1, 1)
	SISetCurvePoints myCurve, "("&inStart.x&","&inStart.y&","&inStart.z&"),("&inEnd.x&","&inEnd.y&","&inEnd.z&")", 0
	
	myCurve.name = inName
	myCurve.AddProperty("Display Property")
	myCurve.properties("display").wirecol.value = 222
	
end function
'-------------------------------------
' Align Legs
'-------------------------------------
function Align_Legs(inModel)
	
	Diagnostic = False	
	
	set LThigh = inModel.FindChild("Bip01_L_Thigh")
	set LRoll = inModel.FindChild("LLegRoll")
	
	set MySkeleton =  GetSkeleton(LThigh, BoneIndex ) 	
	LegHandleOrientation MySkeleton, LRoll, Diagnostic 
		
	set RThigh = inModel.FindChild("Bip01_R_Thigh")
	set RRoll = inModel.FindChild("RLegRoll")
	
	set MySkeleton =  GetSkeleton(RThigh, BoneIndex ) 	
	LegHandleOrientation MySkeleton, RRoll, Diagnostic

end function
'-------------------------------------
' Align Arms
'-------------------------------------
function Align_Arms(inModel)
	
	Diagnostic = False	
	
	set LThigh = inModel.FindChild("Bip01_L_UpperArm")
	set LRoll = inModel.FindChild("LArmRoll")
	
	set MySkeleton =  GetSkeleton(LThigh, BoneIndex ) 	
	'ArmHandleOrientation MySkeleton, LRoll, Diagnostic 
		
	set RThigh = inModel.FindChild("Bip01_R_UpperArm")
	set RRoll = inModel.FindChild("RArmRoll")
	
	set MySkeleton =  GetSkeleton(RThigh, BoneIndex ) 	
	'ArmHandleOrientation MySkeleton, RRoll, Diagnostic

end function
'-----------------------------------
' Arm Handle Orientation
'-----------------------------------
function ArmHandleOrientation(inSkeleton, inHandle, inDiagnostic)
	
	' get the normal for the Handle orientation plane
	' on this skeleton it should be the x+ axis of the 
	' up vector rotation handle
	
	set HandlePlaneStart = XSIMath.CreateVector3
	set HandleTransform = inHandle.parent.Kinematics.Global.Transform
	HandleTransform.GetTranslation HandlePlaneStart
	set ZAxis = XSIMath.CreateVector3
	ZAxis.set 1,0,0
	HandleTransform.AddLocalTranslation ZAxis
	set HandlePlaneEnd = XSIMath.CreateVector3
	HandleTransform.GetTranslation HandlePlaneEnd 
	set HandlePlane = XSIMath.CreateVector3
	HandlePlane.sub HandlePlaneEnd, HandlePlaneStart



	'Also get the zero point for the HandleParent
	'in this case it's the -z axis (0,0,-1)
	
	set ZeroTrans = inHandle.parent.Kinematics.Global.Transform
	set ZeroOri = XSIMath.CreateVector3
	ZeroOri.set 0,0,-1
	ZeroTrans.AddLocalTranslation ZeroOri
	set ZeroEnd = XSIMath.CreateVector3
	ZeroTrans.GetTranslation ZeroEnd
	set ZeroVector = XSIMath.CreateVector3
	ZeroVector.Sub ZeroEnd, HandlePlaneStart
	

	'now get the plane for the upvector
	ProcessUpV inSkeleton, inDiagnostic, UpVPlane, FirstBoneYplus
	
	'now get the projection of the UpVectorPlane onto the HandlePlane
	set Projection = XSIMath.CreateVector3
	Projection.cross UpVPlane, HandlePlane


	' when taking the vector between the two planes you need to pick
	' the one that closest matches the Y plus vector coming off the
	' first bone.
	
	set NegProjection = XSIMath.CreateVector3
	NegProjection.Negate Projection
	
	ArmDot1 = Projection.Dot(FirstBoneYplus)
	ArmDot2 = NegProjection.Dot(FirstBoneYplus)
	

		
	if ArmDot1 < .0001 and ArmDot1 > -.0001 then exit function
	
	if ArmDot2 > ArmDot1 then
		set Projection = NegProjection
	end if
	
	
	'now get the angle between the Projection and the ZeroVector
	
	angle = ZeroVector.Angle(Projection)
	angle = XSIMath.RadiansToDegrees(angle)
	
	'find the sign of the angle
	
	set SignCp = xsimath.CreateVector3 
	SignCp.cross ZeroVector, Projection
	
	'the sign can be determined by the projection of SignCp onto the handlePlane
	
	if SignCp.Dot(HandlePlane) > 0 then
		sign = 1
	else	
		sign = -1
	end if
	
	SetValue inHandle & ".kine.local.rotx", angle * sign
	
	
	if inDiagnostic = True then

		logmessage "DOT WITH PROJECTION: " & ArmDot1
		logmessage "DOT WITH NEG PROJECTION: " & ArmDot2

		set FirstBoneYplusDraw = XSIMath.CreateVector3
		FirstBoneYplus.AddInPlace HandlePlaneStart
		DrawLine2 HandlePlaneStart, FirstBoneYplus, "FirstBoneYplus"	

		HandlePlane.ScaleInPlace 10
		set HandlePlaneDraw = XSIMath.CreateVector3
		HandlePlaneDraw.add HandlePlaneStart, HandlePlane
		DrawLine2 HandlePlaneStart, HandlePlaneDraw, "HandlePlane"
		
		ZeroVector.ScaleInPlace 10
		set ZeroVectorDraw = XSIMath.CreateVector3
		ZeroVectorDraw.add HandlePlaneStart, ZeroVector
		DrawLine2 HandlePlaneStart, ZeroVectorDraw, "ZeroVector"
		
		set ProjectionDraw = XSIMath.CreateVector3
		ProjectionDraw.add HandlePlaneStart, Projection
		DrawLine2 HandlePlaneStart, ProjectionDraw, "Projection"					
		
	end if
	
	
end function

Function GetInstallDir(pluginname)
		GetInstallDir = ""
		dim fso :	Set fso = CreateObject("Scripting.FileSystemObject") 

		' check user add location
		dim path : path=InstallationPath( 5 ) &"\"&pluginname
		if fso.FolderExists(path) then
			GetInstallDir = path
			exit function
		end if

		' check workgroup addon location
		path=InstallationPath( 2 ) &"\Addons\"&pluginname
		if fso.FolderExists(path) then
			GetInstallDir = path
			exit function
		end if

		' check factory addon location
		path=InstallationPath( 4 ) &"\"&pluginname
		if fso.FolderExists(path) then
			GetInstallDir = path
			exit function
		end if

End Function