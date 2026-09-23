ResetState();
SetState( "CloseLidLo" );
ComputeWrinkle( "CloseLidLo", 0 );
		
ResetState();
SetState( "CloseLidUp" );
ComputeWrinkle( "CloseLidUp", 0 );
 
ResetState();
SetState( "ScalpBack" );
ComputeWrinkle( "ScalpBack", 0 );

ResetState();
SetState( "ScalpForward" );
ComputeWrinkle( "ScalpForward", 0 );

ResetState();
SetState( "InnerSquint" );
ComputeWrinkle( "InnerSquint", -0.8 );

ResetState();
SetState( "OuterSquint" );
ComputeWrinkle( "OuterSquint", -1 );

ResetState();
SetState( "WrinkleNose" );
ComputeWrinkle( "WrinkleNose", -0.5 );

ResetState();
SetState( "RaiseBrowIn" );
ComputeWrinkle( "RaiseBrowIn", 2 );

ResetState();
SetState( "BrowOutV" );
ComputeWrinkle( "BrowOutV", 0.75 );

ResetState();
SetState( "Frown" );
ComputeWrinkle( "Frown", -1 );

ResetState();
SetState( "PressNose" );
ComputeWrinkle( "PressNose", 1 );

ResetState();
SetState( "SneerNose" );
ComputeWrinkle( "SneerNose", -1.25 );

ResetState();
SetState( "SuckNostril" );
ComputeWrinkle( "SuckNostril", 0 );

ResetState();
SetState( "BlowNostril" );
ComputeWrinkle( "BlowNostril", 0 );

ResetState();
SetState( "CheekV" );
ComputeWrinkle( "CheekV", -1 );

ResetState();
SetState( "DeflateCheek" );
ComputeWrinkle( "DeflateCheek", 0 );

ResetState();
SetState( "InflateCheek" );
ComputeWrinkle( "InflateCheek", 0 );

ResetState();
SetState( "SuckJaw" );
ComputeWrinkle( "SuckJaw", 0 );

ResetState();
SetState( "JutJaw" );
ComputeWrinkle( "JutJaw", 0 );

ResetState();
SetState( "SlideJawR" );
ComputeWrinkle( "SlideJawR", 0 );

ResetState();
SetState( "SlideJawL" );
ComputeWrinkle( "SlideJawL", 0 );

ResetState();
SetState( "ClenchJaw" );
ComputeWrinkle( "ClenchJaw", 0 );

ResetState();
SetState( "OpenJaw" );
ComputeWrinkle( "OpenJaw", 1 );

ResetState();
SetState( "CompressLips" );
ComputeWrinkle( "CompressLips", 0 );

ResetState();
SetState( "OpenLips" );
ComputeWrinkle( "OpenLips", 0 );

ResetState();
SetState( "JutUpperLip" );
ComputeWrinkle( "JutUpperLip", 0 );

ResetState();
SetState( "OpenUpperLip" );
ComputeWrinkle( "OpenUpperLip", 0 );

ResetState();
SetState( "RaiseChin" );
ComputeWrinkle( "RaiseChin", -0.75 );

ResetState();
SetState( "OpenLowerLip" );
ComputeWrinkle( "OpenLowerLip", 0 );

ResetState();
SetState( "SmileFlat" );
ComputeWrinkle( "SmileFlat", 1 );

ResetState();
SetState( "SmileFull" );
ComputeWrinkle( "SmileFull", 1 );

ResetState();
SetState( "SmileSharp" );
ComputeWrinkle( "SmileSharp", 1 );

ResetState();
SetState( "Platysmus" );
ComputeWrinkle( "Platysmus", 2 );

ResetState();
SetState( "SuckLipUp" );
ComputeWrinkle( "SuckLipUp", 0 );

ResetState();
SetState( "FunnelLipUp" );
ComputeWrinkle( "FunnelLipUp", -0.75 );

ResetState();
SetState( "SuckLipLo" );
ComputeWrinkle( "SuckLipLo", 0 );

ResetState();
SetState( "FunnelLipLo" );
ComputeWrinkle( "FunnelLipLo", -0.5 );

ResetState();
SetState( "PuckerLipUp" );
ComputeWrinkle( "PuckerLipUp", -2 );

ResetState();
SetState( "PuckerLipLo" );
ComputeWrinkle( "PuckerLipLo", -1 );

ResetState();
SetState( "LipCnrTwst" );
ComputeWrinkle( "LipCnrTwst", 0 );

ResetState();
SetState( "Dimple" );
ComputeWrinkle( "Dimple", -2 );

ResetState();
SetState( "PuffLipUp" );
ComputeWrinkle( "PuffLipUp", 0 );

ResetState();
SetState( "PuffLipLo" );
ComputeWrinkle( "PuffLipLo", 0 );

if useTongue then
	ResetState();
	SetState( "TongueBack" );
	ComputeWrinkle( "TongueBack", 0 );
	
	ResetState();
	SetState( "TongueOut" );
	ComputeWrinkle( "TongueOut", 2 );
	
	ResetState();
	SetState( "TongueRight" );
	ComputeWrinkle( "TongueRight", -2 );
	
	ResetState();
	SetState( "TongueLeft" );
	ComputeWrinkle( "TongueLeft", 2 );
	
	ResetState();
	SetState( "TongueV" );
	ComputeWrinkle( "TongueV", 0 );
	
	ResetState();
	SetState( "TongueCurlDown" );
	ComputeWrinkle( "TongueCurlDown", 2 );
	
	ResetState();
	SetState( "TongueCurlUp" );
	ComputeWrinkle( "TongueCurlUp", 2 );
	
	ResetState();
	SetState( "TongueFunnel" );
	ComputeWrinkle( "TongueFunnel", 2 );
	
	ResetState();
	SetState( "TongueNarrow" );
	ComputeWrinkle( "TongueNarrow", 2 );
	
	ResetState();
	SetState( "TongueWide" );
	ComputeWrinkle( "TongueWide", 2 );
end	
