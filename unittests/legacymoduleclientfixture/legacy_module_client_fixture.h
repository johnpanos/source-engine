#ifndef LEGACY_MODULE_CLIENT_FIXTURE_H
#define LEGACY_MODULE_CLIENT_FIXTURE_H

#define PHASE_A_LEGACY_LOADER_CLIENT_INTERFACE "PhaseALegacyLoaderClient001"

class IPhaseALegacyLoaderClient
{
public:
	virtual bool RunLegacyLoaderAbiProbe() = 0;
};

#endif // LEGACY_MODULE_CLIENT_FIXTURE_H
