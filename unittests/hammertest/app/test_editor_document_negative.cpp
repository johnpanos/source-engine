//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the reference workflow's undo guarantee
//			(RFC 0002/0005). A document whose Undo advances history but fails to
//			restore the recorded content snapshot leaves the edit in place. The
//			shared "undo restores original content" predicate must pass for the
//			real EditorDocument and be caught for the broken one.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_document.h"

#include "hammer/formats/keyvalues.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::app::EditorDocument;
using hammer::formats::CompareKeyValues;
using hammer::formats::KeyValueNode;
using hammer::formats::ParseKeyValues;

namespace
{

const char *kVmf = "world\n{\n\t\"classname\" \"worldspawn\"\n}\n";

// Broken document: keeps content but its Undo does NOT restore the pre-edit
// snapshot -- the exact defect the snapshot-based undo prevents.
class BrokenDocument
{
public:
	bool LoadFromText( const std::string &vmf, std::string &error )
	{
		const auto parsed = ParseKeyValues( vmf );
		if ( !parsed.ok )
		{
			error = parsed.error;
			return false;
		}
		m_content = parsed.root;
		return true;
	}

	bool SetFirstBlockKey( const std::string &key, const std::string &value )
	{
		if ( m_content.children.empty() )
		{
			return false;
		}
		for ( auto &pair : m_content.children[0].pairs )
		{
			if ( pair.key == key )
			{
				pair.value = value;
				return true;
			}
		}
		m_content.children[0].pairs.push_back( { key, value } );
		return true;
	}

	bool Undo() { return true; } // bug: does not restore content

	const KeyValueNode &Content() const { return m_content; }

private:
	KeyValueNode m_content;
};

// The undo-restores-content clause: after edit + undo, content must equal the
// original. Returns true when the document honors it.
template <typename Document> bool ConformsUndoRestores( const KeyValueNode &original )
{
	Document doc;
	std::string error;
	doc.LoadFromText( kVmf, error );
	doc.SetFirstBlockKey( "classname", "changed" );
	doc.Undo();
	return CompareKeyValues( doc.Content(), original ).equal;
}

} // namespace

int main()
{
	const KeyValueNode original = ParseKeyValues( kVmf ).root;

	const bool realConforms = ConformsUndoRestores<EditorDocument>( original );
	const bool brokenConforms = ConformsUndoRestores<BrokenDocument>( original );

	int checks = 0;
	int failures = 0;

	++checks;
	if ( !realConforms )
	{
		std::printf( "FAIL: real EditorDocument did not restore content on undo\n" );
		++failures;
	}
	++checks;
	if ( brokenConforms )
	{
		std::printf( "FAIL: predicate did NOT detect the non-restoring document\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.app EditorDocument negative: %d check(s) FAILED\n", failures );
		return testing::ReportConformance( checks, failures );
	}
	std::printf( "hammer.app EditorDocument negative: oracle detects violations (real passes, "
	             "broken caught)\n" );
	return testing::ReportConformance( checks, failures );
}
