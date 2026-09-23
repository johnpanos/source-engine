GroupControls( "BrowInV", "WrinkleNose", "RaiseBrowIn" );
GroupControls( "CheekH", "DeflateCheek", "InflateCheek" );
GroupControls( "CloseLid", "CloseLidLo", "CloseLidUp" );
GroupControls( "FoldLipLo", "SuckLiplo", "FunnelLipLo" );
GroupControls( "FoldLipUp", "SuckLipUp", "FunnelLipUp" );
GroupControls( "JawD", "SuckJaw", "JutJaw" );
GroupControls( "JawH", "SlideJawR", "SlideJawL" );
GroupControls( "JawV", "ClenchJaw", "OpenJaw" );
GroupControls( "LipLoV", "RaiseChin", "OpenLowerLip" );
GroupControls( "LipUpV", "JutUpperLip", "OpenUpperLip" );
GroupControls( "LipsV", "CompressLips", "OpenLips" );
GroupControls( "NoseV", "PressNose", "SneerNose" );
GroupControls( "NostrilFlare", "SuckNostril", "BlowNostril" );
GroupControls( "ScalpD", "ScalpBack", "ScalpForward" );
GroupControls( "Smile", "SmileFlat", "SmileFull", "SmileSharp" );

if useTongue then
	GroupControls( "TongueCurl", "TongueCurlUp", "TongueCurlDown" );
	GroupControls( "TongueD", "TongueBack", "TongueOut" );
	GroupControls( "TongueH", "TongueLeft", "TongueRight" );
end

if useTongue then
	ReorderControls(
		"CloseLid", 
		"InnerSquint", 
		"OuterSquint", 
		"BrowInV", 
		"BrowOutV", 
		"Frown", 
		"NoseV", 
		"NostrilFlare", 
		"CheekV", 
		"CheekH", 
		"JawD", 
		"JawH", 
		"JawV", 
		"LipsV", 
		"LipUpV", 
		"LipLoV", 
		"Smile", 
		"Platysmus", 
		"FoldLipUp", 
		"FoldLipLo", 
		"PuckerLipUp", 
		"PuckerLipLo", 
		"LipCnrTwst", 
		"Dimple", 
		"PuffLipUp", 
		"PuffLipLo", 
		"ScalpD", 
		"TongueV", 
		"TongueH", 
		"TongueCurl", 
		"TongueD"
	);
else
	ReorderControls(
		"CloseLid", 
		"InnerSquint", 
		"OuterSquint", 
		"BrowInV", 
		"BrowOutV", 
		"Frown", 
		"NoseV", 
		"NostrilFlare", 
		"CheekV", 
		"CheekH", 
		"JawD", 
		"JawH", 
		"JawV", 
		"LipsV", 
		"LipUpV", 
		"LipLoV", 
		"Smile", 
		"Platysmus", 
		"FoldLipUp", 
		"FoldLipLo", 
		"PuckerLipUp", 
		"PuckerLipLo", 
		"LipCnrTwst", 
		"Dimple", 
		"PuffLipUp", 
		"PuffLipLo", 
		"ScalpD"
	);
end

SetEyelidControl("CloseLid", true );

SetStereoControl("BrowInV", true );
SetStereoControl("BrowOutV", true );
SetStereoControl("CheekH", true );
SetStereoControl("CheekV", true );
SetStereoControl("CloseLid", true );
SetStereoControl("Dimple", true );
SetStereoControl("FoldLipLo", true );
SetStereoControl("FoldLipUp", true );
SetStereoControl("Frown", true );
SetStereoControl("InnerSquint", true );
SetStereoControl("JawD", false );
SetStereoControl("JawH", false );
SetStereoControl("JawV", false );
SetStereoControl("LipCnrTwst", true );
SetStereoControl("LipLoV", true );
SetStereoControl("LipUpV", true );
SetStereoControl("LipsV", true );
SetStereoControl("NoseV", true );
SetStereoControl("NostrilFlare", true );
SetStereoControl("OuterSquint", true );
SetStereoControl("Platysmus", true );
SetStereoControl("PuckerLipLo", true );
SetStereoControl("PuckerLipUp", true );
SetStereoControl("PuffLipLo", true );
SetStereoControl("PuffLipUp", true );
SetStereoControl("ScalpD", true );
SetStereoControl("Smile", true );


if useTongue then
	SetStereoControl("TongueCurl", false );
	SetStereoControl("TongueD", false );
	SetStereoControl("TongueH", false );
	SetStereoControl("TongueV", false );
end

----------------------------------------------------------
-- Medic has a corrector for this combo, maybe everyone should?
----------------------------------------------------------
AddDominationRule( { "FunnelLipLo" },  { "Dimple"} );
AddDominationRule( { "FunnelLipUp" },  { "Dimple"} );
----------------------------------------------------------

AddDominationRule( { "BrowOutV" },     { "WrinkleNose"} );
AddDominationRule( { "FunnelLipLo" },  { "PuffLipLo"} );
AddDominationRule( { "FunnelLipLo" },  { "PuffLipUp"} );
AddDominationRule( { "FunnelLipUp" },  { "PuffLipLo"} );
AddDominationRule( { "FunnelLipUp" },  { "PuffLipUp"} );
AddDominationRule( { "LipCnrTwst" },   { "Dimple"} );
AddDominationRule( { "OpenJaw" },      { "InflateCheek"} );
AddDominationRule( { "OpenLips" },     { "PuffLipLo"} );
AddDominationRule( { "OpenLips" },     { "PuffLipUp"} );
AddDominationRule( { "OpenLowerLip" }, { "CompressLips"} );
AddDominationRule( { "OpenLowerLip" }, { "FunnelLipLo"} );
AddDominationRule( { "OpenLowerLip" }, { "PuffLipLo"} );
AddDominationRule( { "OpenLowerLip" }, { "PuffLipUp"} );
AddDominationRule( { "OpenLowerLip", "OpenUpperLip" }, { "OpenLips"} );
AddDominationRule( { "OpenUpperLip" }, { "CompressLips"} );
AddDominationRule( { "OpenUpperLip" }, { "FunnelLipUp"} );
AddDominationRule( { "OpenUpperLip" }, { "PuffLipLo"} );
AddDominationRule( { "OpenUpperLip" }, { "PuffLipUp"} );
AddDominationRule( { "Platysmus" },    { "FunnelLipLo"} );
AddDominationRule( { "Platysmus" },    { "FunnelLipUp"} );
AddDominationRule( { "Platysmus" },    { "LipCnrTwst"} );
AddDominationRule( { "Platysmus" },    { "PuckerLipLo"} );
AddDominationRule( { "Platysmus" },    { "PuckerLipUp"} );
AddDominationRule( { "PuckerLipLo" },  { "SmileFlat"} );
AddDominationRule( { "PuckerLipLo" },  { "SmileFull"} );
AddDominationRule( { "PuckerLipLo" },  { "SmileSharp"} );
AddDominationRule( { "PuckerLipLo" },  { "SuckLipLo"} );
AddDominationRule( { "PuckerLipLo",	"OpenJaw" }, { "FunnelLipLo"} );
AddDominationRule( { "PuckerLipUp" },  { "SmileFlat"} );
AddDominationRule( { "PuckerLipUp" },  { "SmileFull"} );
AddDominationRule( { "PuckerLipUp" },  { "SmileSharp"} );
AddDominationRule( { "PuckerLipUp" },  { "SuckLipUp"} );
AddDominationRule( { "PuckerLipUp", "OpenJaw" }, { "FunnelLipUp"} );
AddDominationRule( { "SmileFull" },    { "InflateCheek"} );
AddDominationRule( { "SmileFull" },    { "SuckLipUp"} );
