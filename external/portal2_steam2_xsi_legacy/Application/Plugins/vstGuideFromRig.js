function XSILoadPlugin( in_reg )
{
	in_reg.Author = "wade";
	in_reg.Name = "vstGuideFromRigPlugin";
	in_reg.Email = "";
	in_reg.URL = "";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("vstGuideFromRig","vstGuideFromRig");
	//RegistrationInsertionPoint - do not remove this line

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}

function vstGuideFromRig_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	return true;
}

function vstGuideFromRig_Execute()
{
	// import a guide to match to the rig
	vstCreateGuide();

	// match the transforms for each placement object
	// symmetry currently takes care of the right side of the guide
	MatchTransform("ValveGuide.SpineBase", "ValveBiped.Spine_root", siTrn, null);
	MatchTransform("ValveGuide.Spine1", "ValveBiped.Bip01_Spine1", siTrn, null);
	MatchTransform("ValveGuide.Spine2", "ValveBiped.Bip01_Spine2", siTrn, null);
	MatchTransform("ValveGuide.Spine3", "ValveBiped.Bip01_Spine3", siTrn, null);
	MatchTransform("ValveGuide.NeckBase", "ValveBiped.Spine_eff", siTrn, null);
	MatchTransform("ValveGuide.Neck", "ValveBiped.Bip01_Head", siTrn, null);
	MatchTransform("ValveGuide.Head", "ValveBiped.Neck_eff", siTrn, null);


	MatchTransform("ValveGuide.LShoulderStart", "ValveBiped.L_Shoulder_root", siTrn, null);
	MatchTransform("ValveGuide.LShoulder", "ValveBiped.L_Shoulder_eff", siTrn, null);
	MatchTransform("ValveGuide.LElbow", "ValveBiped.Bip01_L_Forearm", siTrn, null);
	MatchTransform("ValveGuide.LHand", "ValveBiped.L_Arm_eff", siTrn, null);
	MatchTransform("ValveGuide.LHandEnd", "ValveBiped.L_Hand_eff", siTrn, null);

	MatchTransform("ValveGuide.LThumb1", "ValveBiped.L_Finger0_root", siRT, null);
	MatchTransform("ValveGuide.LThumb2", "ValveBiped.Bip01_L_Finger01", siTrn, null);
	MatchTransform("ValveGuide.LThumb3", "ValveBiped.Bip01_L_Finger02", siTrn, null);
	MatchTransform("ValveGuide.LThumb4", "ValveBiped.L_Finger0_eff", siTrn, null);

	MatchTransform("ValveGuide.LIndex1", "ValveBiped.L_Finger1_root", siRT, null);
	MatchTransform("ValveGuide.LIndex2", "ValveBiped.Bip01_L_Finger11", siTrn, null);
	MatchTransform("ValveGuide.LIndex3", "ValveBiped.Bip01_L_Finger12", siTrn, null);
	MatchTransform("ValveGuide.LIndex4", "ValveBiped.L_Finger1_eff", siTrn, null);

	MatchTransform("ValveGuide.LMiddle1", "ValveBiped.L_Finger2_root", siRT, null);
	MatchTransform("ValveGuide.LMiddle2", "ValveBiped.Bip01_L_Finger21", siTrn, null);
	MatchTransform("ValveGuide.LMiddle3", "ValveBiped.Bip01_L_Finger22", siTrn, null);
	MatchTransform("ValveGuide.LMiddle4", "ValveBiped.L_Finger2_eff", siTrn, null);

	MatchTransform("ValveGuide.LRing1", "ValveBiped.L_Finger3_root", siRT, null);
	MatchTransform("ValveGuide.LRing2", "ValveBiped.Bip01_L_Finger31", siTrn, null);
	MatchTransform("ValveGuide.LRing3", "ValveBiped.Bip01_L_Finger32", siTrn, null);
	MatchTransform("ValveGuide.LRing4", "ValveBiped.L_Finger3_eff", siTrn, null);

	MatchTransform("ValveGuide.LPinky1", "ValveBiped.L_Finger4_root", siRT, null);
	MatchTransform("ValveGuide.LPinky2", "ValveBiped.Bip01_L_Finger41", siTrn, null);
	MatchTransform("ValveGuide.LPinky3", "ValveBiped.Bip01_L_Finger42", siTrn, null);
	MatchTransform("ValveGuide.LPinky4", "ValveBiped.L_Finger4_eff", siTrn, null);

	MatchTransform("ValveGuide.LLeg", "ValveBiped.L_Leg_root", siTrn, null);
	MatchTransform("ValveGuide.LKnee", "ValveBiped.Bip01_L_Calf", siTrn, null);
	MatchTransform("ValveGuide.LFoot", "ValveBiped.L_Leg_eff", siTrn, null);
	MatchTransform("ValveGuide.LBall", "ValveBiped.Bip01_L_Toe", siTrn, null);
	MatchTransform("ValveGuide.LToe", "ValveBiped.L_Foot_eff", siTrn, null);
	MatchTransform("ValveGuide.LHeelPivot", "ValveBiped.LHeel", siTrn, null);

	return true;
}



