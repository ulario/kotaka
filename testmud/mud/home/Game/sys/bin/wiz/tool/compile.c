#include <kotaka/paths.h>
#include <game/paths.h>
#include <kotaka/log.h>

inherit LIB_WIZBIN;

void main(string args)
{
	proxy_call("compile_object", args);
}
