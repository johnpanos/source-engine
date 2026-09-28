//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/cordon_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/cordon_ops.h"

#include "mapgeometry/vec3.h"

#include <cmath>

namespace hammer::app::ops
{

using Form = scene::DocumentSettings::CordonForm;

namespace
{

scene::CordonBox ToCordonBox( const scene::Box &box )
{
	return scene::CordonBox{ box.mins, box.maxs };
}

bool FitsSingle( const scene::DocumentSettings &s )
{
	return s.cordons.size() == 1 && s.cordons.front().boxes.size() == 1 &&
	       s.cordons.front().name.empty() && s.cordons.front().active == s.cordonsActive;
}

// Keeps the recorded form expressible (see the header).
void SettleForm( scene::DocumentSettings &s )
{
	if ( ( s.cordonForm == Form::Single && !FitsSingle( s ) ) ||
	     ( s.cordonForm == Form::None && !s.cordons.empty() ) )
	{
		s.cordonForm = Form::List;
	}
}

std::string IndexText( std::size_t index )
{
	return std::to_string( index );
}

EditResult CheckCordon( const scene::DocumentReader &doc, std::size_t index )
{
	if ( index >= doc.Settings().cordons.size() )
	{
		return Reject( "no cordon " + IndexText( index ) );
	}
	return {};
}

EditResult CheckBox( const scene::Box &box )
{
	if ( !ValidCordonBox( box ) )
	{
		return Reject( "a cordon box needs finite bounds with mins below maxs on every axis" );
	}
	return {};
}

} // namespace

bool ValidCordonBox( const scene::Box &box )
{
	for ( int axis = 0; axis < 3; ++axis )
	{
		const double lo = mapgeometry::Component( box.mins, axis );
		const double hi = mapgeometry::Component( box.maxs, axis );
		if ( !std::isfinite( lo ) || !std::isfinite( hi ) || !( lo < hi ) )
		{
			return false;
		}
	}
	return true;
}

EditResult AddCordon(
    scene::DocumentEdit &edit, const std::string &name, const scene::Box &box, std::size_t *index )
{
	if ( EditResult ok = CheckBox( box ); !ok )
	{
		return ok;
	}
	scene::DocumentSettings &s = edit.MutableSettings();
	scene::Cordon cordon;
	cordon.name = name;
	cordon.active = true;
	cordon.boxes.push_back( ToCordonBox( box ) );
	s.cordons.push_back( std::move( cordon ) );
	SettleForm( s );
	if ( index )
	{
		*index = s.cordons.size() - 1;
	}
	return {};
}

EditResult RemoveCordon( scene::DocumentEdit &edit, std::size_t index )
{
	if ( EditResult ok = CheckCordon( edit, index ); !ok )
	{
		return ok;
	}
	scene::DocumentSettings &s = edit.MutableSettings();
	s.cordons.erase( s.cordons.begin() + static_cast<std::ptrdiff_t>( index ) );
	SettleForm( s );
	return {};
}

EditResult RenameCordon( scene::DocumentEdit &edit, std::size_t index, const std::string &name )
{
	if ( EditResult ok = CheckCordon( edit, index ); !ok )
	{
		return ok;
	}
	if ( edit.Settings().cordons[index].name == name )
	{
		return NothingToDo( "the cordon already has that name" );
	}
	scene::DocumentSettings &s = edit.MutableSettings();
	s.cordons[index].name = name;
	SettleForm( s );
	return {};
}

EditResult SetCordonBox(
    scene::DocumentEdit &edit, std::size_t cordon, std::size_t boxIndex, const scene::Box &box )
{
	if ( EditResult ok = CheckCordon( edit, cordon ); !ok )
	{
		return ok;
	}
	if ( boxIndex >= edit.Settings().cordons[cordon].boxes.size() )
	{
		return Reject( "cordon " + IndexText( cordon ) + " has no box " + IndexText( boxIndex ) );
	}
	if ( EditResult ok = CheckBox( box ); !ok )
	{
		return ok;
	}
	if ( edit.Settings().cordons[cordon].boxes[boxIndex] == ToCordonBox( box ) )
	{
		return NothingToDo( "the cordon box is unchanged" );
	}
	edit.MutableSettings().cordons[cordon].boxes[boxIndex] = ToCordonBox( box );
	return {};
}

EditResult AddCordonBox( scene::DocumentEdit &edit, std::size_t cordon, const scene::Box &box )
{
	if ( EditResult ok = CheckCordon( edit, cordon ); !ok )
	{
		return ok;
	}
	if ( EditResult ok = CheckBox( box ); !ok )
	{
		return ok;
	}
	scene::DocumentSettings &s = edit.MutableSettings();
	s.cordons[cordon].boxes.push_back( ToCordonBox( box ) );
	SettleForm( s );
	return {};
}

EditResult RemoveCordonBox( scene::DocumentEdit &edit, std::size_t cordon, std::size_t boxIndex )
{
	if ( EditResult ok = CheckCordon( edit, cordon ); !ok )
	{
		return ok;
	}
	const std::vector<scene::CordonBox> &boxes = edit.Settings().cordons[cordon].boxes;
	if ( boxIndex >= boxes.size() )
	{
		return Reject( "cordon " + IndexText( cordon ) + " has no box " + IndexText( boxIndex ) );
	}
	if ( boxes.size() == 1 )
	{
		return Reject( "a cordon keeps at least one box; remove the cordon instead" );
	}
	scene::DocumentSettings &s = edit.MutableSettings();
	s.cordons[cordon].boxes.erase(
	    s.cordons[cordon].boxes.begin() + static_cast<std::ptrdiff_t>( boxIndex ) );
	SettleForm( s );
	return {};
}

EditResult SetCordonActive( scene::DocumentEdit &edit, std::size_t index, bool active )
{
	if ( EditResult ok = CheckCordon( edit, index ); !ok )
	{
		return ok;
	}
	const scene::DocumentSettings &current = edit.Settings();
	const bool single = current.cordonForm == Form::Single && FitsSingle( current );
	if ( current.cordons[index].active == active && ( !single || current.cordonsActive == active ) )
	{
		return NothingToDo(
		    active ? "the cordon is already active" : "the cordon is already inactive" );
	}
	scene::DocumentSettings &s = edit.MutableSettings();
	s.cordons[index].active = active;
	if ( single )
	{
		s.cordonsActive = active;
	}
	SettleForm( s );
	return {};
}

EditResult SetCordonsEnabled( scene::DocumentEdit &edit, bool enabled )
{
	const scene::DocumentSettings &current = edit.Settings();
	const bool single = current.cordonForm == Form::Single && FitsSingle( current );
	if ( current.cordonsActive == enabled &&
	     ( !single || current.cordons.front().active == enabled ) )
	{
		return NothingToDo(
		    enabled ? "cordons are already enabled" : "cordons are already disabled" );
	}
	scene::DocumentSettings &s = edit.MutableSettings();
	s.cordonsActive = enabled;
	if ( single )
	{
		s.cordons.front().active = enabled;
	}
	SettleForm( s );
	return {};
}

std::vector<scene::Box> ActiveCordonBoxes( const scene::DocumentReader &doc )
{
	std::vector<scene::Box> out;
	const scene::DocumentSettings &s = doc.Settings();
	if ( !s.cordonsActive )
	{
		return out;
	}
	for ( const scene::Cordon &cordon : s.cordons )
	{
		if ( !cordon.active )
		{
			continue;
		}
		for ( const scene::CordonBox &box : cordon.boxes )
		{
			out.push_back( scene::Box{ box.mins, box.maxs } );
		}
	}
	return out;
}

bool InsideCordons( const std::vector<scene::Box> &active, const scene::Box &bounds )
{
	if ( active.empty() )
	{
		return true;
	}
	for ( const scene::Box &box : active )
	{
		if ( box.Intersects( bounds, 0.0 ) )
		{
			return true;
		}
	}
	return false;
}

bool InsideCordons( const scene::DocumentReader &doc, const scene::Box &bounds )
{
	return InsideCordons( ActiveCordonBoxes( doc ), bounds );
}

} // namespace hammer::app::ops
