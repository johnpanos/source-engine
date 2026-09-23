
// --------------------------------------------------------
function StartContainerAnimations()
{	
	for(local i=1;i<=92;i+=1)
	{
//		EntFire("container_stacked_" + i,"setparentattachment", "vstattachment", 0 )
		EntFire("@container_stacked_" + i,"setanimation", "container" + i, 0 )
	}
}

function SetupContainerAttachments()
{
//	for(local i=1;i<=92;i+=1)
//	{
//		EntFire("container_stacked_" + i,"SetParentAttachmentMaintainOffset", "vstAttachment_noOrient", 0 )
//	}
}
