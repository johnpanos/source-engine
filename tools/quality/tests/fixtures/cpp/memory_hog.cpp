// Self-test fixture: touches far more memory than the self-test profile's cap
// allows. The runner's per-process address-space limit must stop it (the
// allocation fails and the process aborts) instead of letting it exhaust the host.
#include "testing/conformance_result.h"
#include <cstddef>
#include <cstring>
#include <new>

int main()
{
	const std::size_t size = std::size_t( 1 ) << 30; // 1 GiB against a 64 MiB cap
	char *block = new char[size];
	std::memset( block, 1, size );
	const int value = block[size - 1];
	delete[] block;
	return testing::ReportConformance( 1, value == 1 ? 0 : 1 );
}
