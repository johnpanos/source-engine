//============================================================================
// To change what movie plays during the level transition, change the entry named MOVIE_NAME
// valid movies to play are in the "portal2\media" folder.  They must exist in this directory
// and you only need to specify the file name, not the path.
//============================================================================

MOVIE_NAME<- [
	"gd_babysit",
	"gd_graphs",
	"gd_turret_exploded_blue",
	"gd_turret_styles",
]
//MOVIE_NAME <- "gd_babysit"

//============================================================================

function PreSpawnInstance( entityClass, entityName )
{           
	local movielength = MOVIE_NAME.len();
	local movie = MOVIE_NAME[ RandomInt( 0, movielength-1 ) ]   
    //printl("================PLAYING " + movie + ", length = " + movielength )
    return { MovieFilename = movie }
}

