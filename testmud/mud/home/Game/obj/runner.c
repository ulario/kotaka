#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit base "~/lib/object";

string data;

static void create(int clone)
{
	if (clone) {
		base::create();
		call_out("heartbeat", 1.0 + SUBD->rnd() * 10.0);
		call_out("die", 120.0 + SUBD->rnd() * 6.0);
	}
}

static void die()
{
	destruct_object(this_object());
}

static void heartbeat()
{
	float chance;
	int size;

	if ("~/sys/mazed"->difference() < 0) {
		call_out("die", 2.0 + SUBD->rnd() * 5.0);
		return;
	}

	call_out("heartbeat", 5.0 + SUBD->rnd() * 35.0);

	size = "~/sys/mazed"->query_size();
	data = STRINGD->chars(random(256), 10240 + random(64));

	{
		object target;
		int newroomid;

		newroomid = (int)floor((float)size * SUBD->rnd() * SUBD->rnd() * SUBD->rnd());
		target = "~/sys/mazed"->query_room(newroomid);

		move(target);
	}
}

static void destruct(int clone)
{
	if (clone) {
		base::destruct();
	}
}
