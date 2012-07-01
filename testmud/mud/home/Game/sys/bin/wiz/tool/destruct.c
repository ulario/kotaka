#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_WIZBIN;

void main(string args)
{
	proxy_call("destruct_object", args);
}
