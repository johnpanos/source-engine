function XSILoadPlugin( in_reg )
{
	in_reg.Author = "wade";
	in_reg.Name = "vstCheckNamesPlugin";
	in_reg.Email = "";
	in_reg.URL = "";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("vstCheckBipedNames","vstCheckBipedNames");
	in_reg.RegisterCommand("vstConvertBipedEpisodic","vstConvertBipedEpisodic");
	in_reg.RegisterCommand("vstCheckGuideNames","vstCheckGuideNames");
	//RegistrationInsertionPoint - do not remove this line

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}

function vstCheckBipedNames_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	return true;
}

function vstCheckBipedNames_Execute()
{
	// arrays for objects to rename
	namesOld = new Array(
							"ValveBiped.Bip01_Head1",
							"ValveBiped.Bip01_Neck1",
							"ValveBiped.Bip01_Spine",
							"ValveBiped.Bip01_Spine4",
							"ValveBiped.Spine_Root",
							"ValveBiped.eff",
							"ValveBiped.eff1",
							"ValveBiped.Neck1",

							"ValveBiped.LRootShoulder",
							"ValveBiped.LShouldereff",
							"ValveBiped.LArmRoot",
							"ValveBiped.LArmeff",
							"ValveBiped.LHandRoot",
							"ValveBiped.LHandeffector",
							"ValveBiped.LPinkyRoot",
							"ValveBiped.LPinkyeffector",
							"ValveBiped.LRingRoot",
							"ValveBiped.LRingeffector",
							"ValveBiped.LMiddleRoot",
							"ValveBiped.LMiddleeffector",
							"ValveBiped.LIndexRoot",
							"ValveBiped.LIndexeffector",
							"ValveBiped.LThumbRoot",
							"ValveBiped.LThumbeffector",

							"ValveBiped.RRootShoulder",
							"ValveBiped.RShouldereff",
							"ValveBiped.RArmRoot",
							"ValveBiped.RArmeff",
							"ValveBiped.RHandRoot",
							"ValveBiped.RHandeffector",
							"ValveBiped.RPinkyRoot",
							"ValveBiped.RPinkyeffector",
							"ValveBiped.RRingRoot",
							"ValveBiped.RRingeffector",
							"ValveBiped.RMiddleRoot",
							"ValveBiped.RMiddleeffector",
							"ValveBiped.RIndexRoot",
							"ValveBiped.RIndexeffector",
							"ValveBiped.RThumbRoot",
							"ValveBiped.RThumbeffector",

							"ValveBiped.LLegRoot",
							"ValveBiped.LFooteff",
							"ValveBiped.LRootFoot",
							"ValveBiped.Bip01_L_Toe0",
							"ValveBiped.LToeeff",

							"ValveBiped.RLegRoot",
							"ValveBiped.RFooteff",
							"ValveBiped.RRootFoot",
							"ValveBiped.Bip01_R_Toe0",
							"ValveBiped.RToeeff"
						 );
	namesNew = new Array(
							"Bip01_Head",
							"Bip01_Neck0",
							"Bip01_Spine0",
							"Bip01_Spine3",
							"Spine_root",
							"Spine_eff",
							"Neck_eff",
							"Neck_root",

							"L_Shoulder_root",
							"L_Shoulder_eff",
							"L_Arm_root",
							"L_Arm_eff",
							"L_Hand_root",
							"L_Hand_eff",
							"L_Finger4_root",
							"L_Finger4_eff",
							"L_Finger3_root",
							"L_Finger3_eff",
							"L_Finger2_root",
							"L_Finger2_eff",
							"L_Finger1_root",
							"L_Finger1_eff",
							"L_Finger0_root",
							"L_Finger0_eff",

							"R_Shoulder_root",
							"R_Shoulder_eff",
							"R_Arm_root",
							"R_Arm_eff",
							"R_Hand_root",
							"R_Hand_eff",
							"R_Finger4_root",
							"R_Finger4_eff",
							"R_Finger3_root",
							"R_Finger3_eff",
							"R_Finger2_root",
							"R_Finger2_eff",
							"R_Finger1_root",
							"R_Finger1_eff",
							"R_Finger0_root",
							"R_Finger0_eff",

							"L_Leg_root",
							"L_Leg_eff",
							"L_Foot_root",
							"Bip01_L_Toe",
							"L_Foot_eff",

							"R_Leg_root",
							"R_Leg_eff",
							"R_Foot_root",
							"Bip01_R_Toe",
							"R_Foot_eff"
						 );

	// for each index in namesOld, look for that object and rename it to the same index in namesNew
	for (i = 0; i < namesOld.length; i++)
	{
		var objExists = ActiveSceneRoot.FindChild(namesOld[i]);
		if (objExists)
		{
			SetValue((namesOld[i] + ".Name"), namesNew[i], null);
			logmessage(namesOld[i] + " found and renamed to " + namesNew[i] + ".");
		}
//		else
//		{
//			logmessage(namesOld[i] + " not found.");
//		}
	}

	return true;
}


function vstCheckGuideNames_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	return true;
}

function vstCheckGuideNames_Execute()
{
	// arrays for objects to rename
	namesOld = new Array(
							"ValveGuide.RibCageEnd",
							"ValveGuide.RHeel",
							"ValveGuide.LHeel"
						 );
	namesNew = new Array(
							"NeckBase",
							"RBall",
							"LBall"
						 );

	// for each index in namesOld, look for that object and rename it to the same index in namesNew
	for (i = 0; i < namesOld.length; i++)
	{
		var objExists = ActiveSceneRoot.FindChild(namesOld[i]);
		if (objExists)
		{
			SetValue((namesOld[i] + ".Name"), namesNew[i], null);
			logmessage(namesOld[i] + " found and renamed to " + namesNew[i] + ".");
		}
//		else
//		{
//			logmessage(namesOld[i] + " not found.");
//		}
	}

	return true;
}


function vstConvertBipedEpisodic_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	return true;
}

function vstConvertBipedEpisodic_Execute()
{
	// arrays for objects to rename
	namesNew = new Array(
							"Bip01_Head1",
							"Bip01_Neck1",
							"Bip01_Spine",
							"Bip01_Spine4",
							"Spine_Root",
							"eff",
							"eff1",
							"Neck1",

							"LRootShoulder",
							"LShouldereff",
							"LArmRoot",
							"LArmeff",
							"LHandRoot",
							"LHandeffector",
							"LPinkyRoot",
							"LPinkyeffector",
							"LRingRoot",
							"LRingeffector",
							"LMiddleRoot",
							"LMiddleeffector",
							"LIndexRoot",
							"LIndexeffector",
							"LThumbRoot",
							"LThumbeffector",

							"RRootShoulder",
							"RShouldereff",
							"RArmRoot",
							"RArmeff",
							"RHandRoot",
							"RHandeffector",
							"RPinkyRoot",
							"RPinkyeffector",
							"RRingRoot",
							"RRingeffector",
							"RMiddleRoot",
							"RMiddleeffector",
							"RIndexRoot",
							"RIndexeffector",
							"RThumbRoot",
							"RThumbeffector",

							"LLegRoot",
							"LFooteff",
							"LRootFoot",
							"Bip01_L_Toe0",
							"LToeeff",

							"RLegRoot",
							"RFooteff",
							"RRootFoot",
							"Bip01_R_Toe0",
							"RToeeff",

							"LfootUpVector",
							"LfootUpVector2",
							"spacer",
							"heel",
							"toe",
							"mid",
							"ankle",
							"spacer3",

							"RfootUpVector1",
							"RfootUpVector2",
							"spacer1",
							"heel1",
							"toe1",
							"mid1",
							"ankle1",
							"spacer4",

							"lArmUpVParent",
							"lArmUpV",

							"lArmUpVParent1",
							"rArmUpV"
						 );
	namesOld = new Array(
							"ValveBiped.Bip01_Head",
							"ValveBiped.Bip01_Neck0",
							"ValveBiped.Bip01_Spine0",
							"ValveBiped.Bip01_Spine3",
							"ValveBiped.Spine_root",
							"ValveBiped.Spine_eff",
							"ValveBiped.Neck_eff",
							"ValveBiped.Neck_root",

							"ValveBiped.L_Shoulder_root",
							"ValveBiped.L_Shoulder_eff",
							"ValveBiped.L_Arm_root",
							"ValveBiped.L_Arm_eff",
							"ValveBiped.L_Hand_root",
							"ValveBiped.L_Hand_eff",
							"ValveBiped.L_Finger4_root",
							"ValveBiped.L_Finger4_eff",
							"ValveBiped.L_Finger3_root",
							"ValveBiped.L_Finger3_eff",
							"ValveBiped.L_Finger2_root",
							"ValveBiped.L_Finger2_eff",
							"ValveBiped.L_Finger1_root",
							"ValveBiped.L_Finger1_eff",
							"ValveBiped.L_Finger0_root",
							"ValveBiped.L_Finger0_eff",

							"ValveBiped.R_Shoulder_root",
							"ValveBiped.R_Shoulder_eff",
							"ValveBiped.R_Arm_root",
							"ValveBiped.R_Arm_eff",
							"ValveBiped.R_Hand_root",
							"ValveBiped.R_Hand_eff",
							"ValveBiped.R_Finger4_root",
							"ValveBiped.R_Finger4_eff",
							"ValveBiped.R_Finger3_root",
							"ValveBiped.R_Finger3_eff",
							"ValveBiped.R_Finger2_root",
							"ValveBiped.R_Finger2_eff",
							"ValveBiped.R_Finger1_root",
							"ValveBiped.R_Finger1_eff",
							"ValveBiped.R_Finger0_root",
							"ValveBiped.R_Finger0_eff",

							"ValveBiped.L_Leg_root",
							"ValveBiped.L_Leg_eff",
							"ValveBiped.L_Foot_root",
							"ValveBiped.Bip01_L_Toe",
							"ValveBiped.L_Foot_eff",

							"ValveBiped.R_Leg_root",
							"ValveBiped.R_Leg_eff",
							"ValveBiped.R_Foot_root",
							"ValveBiped.Bip01_R_Toe",
							"ValveBiped.R_Foot_eff",

							"ValveBiped.LfootUpVectorHeel",
							"ValveBiped.LfootUpVectorBall",
							"ValveBiped.LAnkleGroup",
							"ValveBiped.LHeel",
							"ValveBiped.LToe",
							"ValveBiped.LBall",
							"ValveBiped.LAnkle",
							"ValveBiped.LBallGroup",

							"ValveBiped.RfootUpVectorHeel",
							"ValveBiped.RfootUpVectorBall",
							"ValveBiped.RAnkleGroup",
							"ValveBiped.RHeel",
							"ValveBiped.RToe",
							"ValveBiped.RBall",
							"ValveBiped.RAnkle",
							"ValveBiped.RBallGroup",

							"ValveBiped.LArmUpVParent",
							"ValveBiped.LArmUpV",

							"ValveBiped.RArmUpVParent",
							"ValveBiped.RArmUpV"
						 );

	// for each index in namesOld, look for that object and rename it to the same index in namesNew
	for (i = 0; i < namesOld.length; i++)
	{
		var objExists = ActiveSceneRoot.FindChild(namesOld[i]);
		if (objExists)
		{
			logmessage(namesOld[i] + " found and renamed to " + namesNew[i] + ".");
			SetValue((namesOld[i] + ".Name"), namesNew[i], null);
		}
	}

	return true;
}
