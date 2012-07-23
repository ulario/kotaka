#include <kernel/kernel.h>
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <status.h>
#include <type.h>

static int free_callouts()
{
	return status(ST_COTABSIZE) -
		(status(ST_NCOSHORT) + status(ST_NCOLONG));
}

#if 1
static int call_out(string func, mixed delay, mixed args...)
{
	if (!this_object()) {
		error("Cannot call_out from destructed object");
	}

	if (!function_object(func, this_object())) {
		error("Call_out to undefined function " + func);
	}

	if (!SYSTEM() && free_callouts() < 20) {
		error("Too many callouts");
	}

	return ::call_out(func, delay, args...);
}
#endif
