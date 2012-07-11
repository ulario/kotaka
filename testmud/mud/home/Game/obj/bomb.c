#include <kotaka/paths.h>
#include <limits.h>

string kaboom;

static void create(int clone)
{
	kaboom = STRINGD->chars('\0', MAX_STRING_SIZE);
}
