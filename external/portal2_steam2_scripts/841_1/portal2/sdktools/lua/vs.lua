
-- The Valve Table
local P = {
	vproject = "";
}

vs = P;

-- Returns the VPROJECT
local function VProject()

	if ( P.vproject ~= nil and string.len( P.vproject ) ~= 0 ) then
		return P.vproject;
	end

	-- Get VPROJECT environment variable
	P.vproject = os.getenv( "VPROJECT" );

	-- Change all backslashes \ to forward slashes /
	P.vproject = string.gsub( P.vproject, "[\\/]", "/" );

	-- Remove all slashes from the end
	P.vproject = string.gsub( P.vproject, "/*$", "" );

	return P.vproject;
end


-- Returns the VPROJECT
local function SetGame( game )

	game = string.gsub( game, "[\\/]", "/" );

	if ( not( string.match( game, "/" ) ) ) then
		vproject = VProject();
		game = string.gsub( vproject, "[^/]+$", game );
	end

	P.vproject = game;

	return VProject();
end


-- Returns the game directory
local function GameDir()
	gameDir = VProject() .. "/";

	return gameDir;
end


-- Get the name of the game mod being used
local function ContentDir()
	vproject = VProject();
	contentDir = string.gsub( vproject, "/game/", "/content/" );
	contentDir = contentDir .. "/";

	return contentDir;
end


-- Get the name of the game mod being used
local function ModName()
	vproject = VProject();
	nStart, nEnd = string.find( vproject, "[^/]+$" );
	modName = string.sub( vproject, nStart, nEnd );

	return modName;
end


vs = {
	VProject = VProject,
	SetGame = SetGame,
	GameDir = GameDir,
	ContentDir = ContentDir,
	ModName = ModName
};


-- remove selected vertices from named deltas so they don't move
function RemoveFromDelta( deltaList )
 for i,delta in ipairs( deltaList ) do
  ResetState();
  SetState( delta );
  Interp( "base", 1.0 );
  SaveDelta( delta )
 end
end


return vs;

