#include <kotaka/paths.h>

/* objects moved here will be incinerated */

inherit LIB_OBJECT;

int callout;

static void create()
{
}

void insert_notify(object obj)
{
	if (!callout) {
		callout = call_out("purge", 0);
	}
}

static void purge()
{
	callout = 0;
}
