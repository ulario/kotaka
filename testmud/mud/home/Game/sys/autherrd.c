#include <kotaka/privilege.h>

mapping quota;
int ticking;

static void create()
{
	quota = ([ ]);
}

int increment(int ipn, int amount)
{
	ACCESS_CHECK(GAME());

	ipn &= ~0xFF;

	if (!quota[ipn]) {
		quota[ipn] = 0;
	}

	quota[ipn] += amount;

	if (!ticking) {
		call_out("tick", 5 * 60);

		ticking = 1;
	}

	return quota[ipn];
}

static void tick()
{
	int *ind;
	int *val;
	int i;

	ticking = 0;

	ind = map_indices(quota);

	for (i = 0; i < sizeof(ind); i++) {
		quota[ind[i]]--;
		
		if (quota[ind[i]] <= 0) {
			quota[ind[i]] = nil;
		}
	}

	if (map_sizeof(quota)) {
		call_out("tick", 60 * 15);

		ticking = 1;
	}
}
