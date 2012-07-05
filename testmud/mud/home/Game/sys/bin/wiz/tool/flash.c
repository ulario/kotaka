#include <kotaka/paths.h>
#include <game/paths.h>
#include <kotaka/log.h>

inherit LIB_WIZBIN;

atomic void main(string args)
{
	proxy_call("destruct_object", args);
	proxy_call("compile_object", args);
}
