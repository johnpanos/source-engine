// Self-test fixture: a `"link": "shared"` unit, built into its own library.
extern "C" __attribute__( ( visibility( "default" ) ) ) int SharedUnitFixtureValue()
{
	return 7;
}
