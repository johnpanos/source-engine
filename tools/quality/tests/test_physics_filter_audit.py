"""Negative and positive fixtures for the game collision-filter audit (RFC 0013 P3)."""

import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
ROOT = QUALITY.parents[1]
SPEC = importlib.util.spec_from_file_location("physics_filter_audit", QUALITY / "physics_filter_audit.py")
audit = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(audit)

SERVER = """
bool WheelCollidesWith( IPhysicsObject *pObj, CBaseEntity *pEntity )
{
	return pEntity->GetMoveType() == MOVETYPE_PUSH;
}
int CCollisionEvent::ShouldCollide( IPhysicsObject *pObj0, IPhysicsObject *pObj1, void *pGameData0, void *pGameData1 )
#if _DEBUG
{
	return ShouldCollide_2( pObj0, pObj1, pGameData0, pGameData1 );
}
int CCollisionEvent::ShouldCollide_2( IPhysicsObject *pObj0, IPhysicsObject *pObj1, void *pGameData0, void *pGameData1 )
#endif
{
	CallbackContext check(this);
	// a comment mentioning RandomInt( 0, 1 ) and m_count = 3 must not count
	const char *pText = "EmitSound( x )";
	if ( g_pGameRules->ShouldCollide( 1, 2 ) ) { return 1; }
	return 0;
}
"""
CLIENT = """
int CCollisionEvent::ShouldCollide( IPhysicsObject *pObj0, IPhysicsObject *pObj1, void *pGameData0, void *pGameData1 )
#if _DEBUG
{
	return ShouldCollide_2( pObj0, pObj1, pGameData0, pGameData1 );
}
int CCollisionEvent::ShouldCollide_2( IPhysicsObject *pObj0, IPhysicsObject *pObj1, void *pGameData0, void *pGameData1 )
#endif
{
	return 1;
}
"""
RULES = """
class CGameRules : public CAutoGameSystem
{
};
class CMyRules : public CGameRules
{
};
bool CGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	return collisionGroup0 != collisionGroup1;
}
bool CMyRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	return CGameRules::ShouldCollide( collisionGroup0, collisionGroup1 );
}
// A trace filter, not a game rules class: never collected.
bool CBaseEntity::ShouldCollide( int collisionGroup, int contentsMask )
{
	return RandomInt( 0, 1 ) != 0;
}
"""


class FixtureTree:
    def __init__(self):
        self.dir = tempfile.TemporaryDirectory()
        self.root = self.dir.name
        self.write("game/server/physics.cpp", SERVER)
        self.write("game/client/physics.cpp", CLIENT)
        self.write("game/shared/gamerules.cpp", RULES)
        for header in audit.ENTITY_HEADERS:
            self.write(header, "")

    def write(self, relative, text):
        path = os.path.join(self.root, relative)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w") as stream:
            stream.write(text)

    def declaration(self, sites):
        path = os.path.join(self.root, "decl.json")
        with open(path, "w") as stream:
            json.dump({"schema": audit.SCHEMA, "sites": sites}, stream)
        return path

    def reviewed(self):
        """A declaration built from the scaffold, with reasons filled in."""
        stubs = audit.scaffold(audit.collect(self.root), None)
        for stub in stubs.values():
            for rule in stub.get("accepted", {}):
                stub["accepted"][rule] = "reviewed"
        return stubs

    def check(self, sites):
        return audit.compare(audit.collect(self.root), audit.load_declaration(self.declaration(sites)))


class CollectTests(unittest.TestCase):
    def setUp(self):
        self.tree = FixtureTree()

    def tearDown(self):
        self.tree.dir.cleanup()

    def test_collects_roots_and_game_rules_only(self):
        sites = audit.collect(self.tree.root)
        names = sorted(k.split("/")[-1] for k in sites)
        self.assertIn("gamerules.cpp::ShouldCollide#0", names)
        self.assertIn("gamerules.cpp::ShouldCollide#1", names)
        self.assertEqual(2, sum(1 for n in names if n.startswith("gamerules.cpp")))
        self.assertIn("physics.cpp::CCollisionEvent::ShouldCollide_2#0", names)
        self.assertIn("physics.cpp::WheelCollidesWith#0", names)

    def test_call_sites_are_not_definitions(self):
        code = audit.strip_comments_and_strings("void f() { if ( p->ShouldCollide( 1, 2 ) ) { g(); } }")
        self.assertEqual([], audit.find_definitions(code, "ShouldCollide"))

    def test_comments_and_strings_do_not_count(self):
        sites = audit.collect(self.tree.root)
        body = sites["game/server/physics.cpp::CCollisionEvent::ShouldCollide_2#0"]
        self.assertEqual(["callback-context"], sorted(body["findings"]))

    def test_missing_root_is_an_error(self):
        self.tree.write("game/client/physics.cpp", "int nothing;")
        with self.assertRaises(audit.AuditError):
            audit.collect(self.tree.root)


class RuleTests(unittest.TestCase):
    CASES = {
        "thread-identity": "if ( ThreadInMainThread() ) return 1;",
        "engine-call": "engine->ServerCommand( x );",
        "entity-mutation": "pEntity->SetAbsOrigin( v );",
        "physics-mutation": "pObj->Wake();",
        "random": "return RandomInt( 0, 1 );",
        "prediction-state": "if ( GetPredictionPlayer() ) return 0;",
        "lazy-transform": "Vector v = pEntity->GetAbsOrigin();",
        "member-write": "m_count += 1;",
        "static-local": "static int s_cache = 0;",
        "allocation": "int *p = new int;",
        "console-output": "Msg( \"hi\" );",
        "convar-write": "sv_cheats.SetValue( 1 );",
        "callback-context": "CallbackContext check( this );",
    }

    def test_every_rule_fires_on_its_sample(self):
        self.assertEqual(set(audit.RULES), set(self.CASES))
        for rule, sample in self.CASES.items():
            body = audit.strip_comments_and_strings("{ " + sample + " }")
            fired = {r for r, _ in audit.scan_body(body)}
            self.assertIn(rule, fired, sample)

    def test_comparisons_are_not_member_writes(self):
        body = audit.strip_comments_and_strings("{ return m_count == 3 && m_other != 2 && m_x <= 1; }")
        self.assertNotIn("member-write", {r for r, _ in audit.scan_body(body)})

    def test_static_cast_is_not_a_static_local(self):
        body = audit.strip_comments_and_strings("{ CBaseEntity *p = static_cast<CBaseEntity *>( q ); }")
        self.assertNotIn("static-local", {r for r, _ in audit.scan_body(body)})


class CompareTests(unittest.TestCase):
    def setUp(self):
        self.tree = FixtureTree()

    def tearDown(self):
        self.tree.dir.cleanup()

    def kinds(self, sites):
        return sorted(kind for kind, _, _ in self.tree.check(sites))

    def test_reviewed_tree_passes(self):
        self.assertEqual([], self.kinds(self.tree.reviewed()))

    def test_new_override_is_unreviewed(self):
        sites = self.tree.reviewed()
        self.tree.write("game/shared/other.cpp", "class COther : public CMyRules {};\n"
                        "bool COther::ShouldCollide( int a, int b ) { return a < b; }\n")
        self.assertEqual(["unreviewed"], self.kinds(sites))

    def test_changed_body_needs_review(self):
        sites = self.tree.reviewed()
        self.tree.write("game/shared/gamerules.cpp", RULES.replace("collisionGroup0 != collisionGroup1",
                                                                   "collisionGroup0 == collisionGroup1"))
        self.assertEqual(["changed"], self.kinds(sites))

    def test_forbidden_rule_fails_even_when_reviewed(self):
        sites = self.tree.reviewed()
        self.tree.write("game/shared/gamerules.cpp", RULES.replace(
            "return collisionGroup0 != collisionGroup1;", "return RandomInt( 0, 1 ) != 0;"))
        kinds = self.kinds(sites)
        self.assertIn("forbidden", kinds)
        self.assertIn("changed", kinds)

    def test_unaccepted_finding(self):
        sites = self.tree.reviewed()
        key = "game/server/physics.cpp::CCollisionEvent::ShouldCollide_2#0"
        del sites[key]["accepted"]
        sites[key]["verdict"] = "safe"
        self.assertIn("unaccepted", self.kinds(sites))

    def test_stale_site_and_stale_acceptance(self):
        sites = self.tree.reviewed()
        sites["game/shared/gone.cpp::ShouldCollide#0"] = {"digest": "0", "verdict": "safe"}
        key = "game/shared/gamerules.cpp::ShouldCollide#0"
        sites[key] = dict(sites[key], verdict="accepted", accepted={"member-write": "no longer true"})
        self.assertEqual(["stale", "stale"], self.kinds(sites))

    def test_declaration_cannot_accept_forbidden_rules(self):
        sites = self.tree.reviewed()
        key = "game/shared/gamerules.cpp::ShouldCollide#0"
        sites[key] = dict(sites[key], verdict="accepted", accepted={"random": "seems fine"})
        with self.assertRaises(audit.AuditError):
            audit.load_declaration(self.tree.declaration(sites))

    def test_accepted_findings_need_reasons(self):
        sites = self.tree.reviewed()
        key = "game/server/physics.cpp::CCollisionEvent::ShouldCollide_2#0"
        sites[key]["accepted"]["callback-context"] = ""
        with self.assertRaises(audit.AuditError):
            audit.load_declaration(self.tree.declaration(sites))


class RepositoryTests(unittest.TestCase):
    def test_installed_declaration_matches_the_tree(self):
        self.assertEqual(0, audit.main(["check", "--root", str(ROOT)]))


if __name__ == "__main__":
    unittest.main()
