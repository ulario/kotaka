#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit base "~/lib/object";

static void create(int clone)
{
	if (clone) {
		base::create();
		call_out("heartbeat", 5.0 + SUBD->rnd() * 10.0);
	}
}

static void heartbeat()
{
	float chance;
	int size;

	size = "~/sys/mazed"->query_size();

	chance = 1.0 - 1.0 / (float)size;

	if (SUBD->rnd() < chance) {
		object target;

		target = "~/sys/mazed"->query_room(
			(int)floor((float)size * SUBD->rnd() * SUBD->rnd())
		);
		move(target);
	} else {
		destruct_object(this_object());
	}
}

static void destruct(int clone)
{
	if (clone) {
		base::destruct();
	}
}
