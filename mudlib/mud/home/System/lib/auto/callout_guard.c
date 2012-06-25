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

	return ::call_out("_F_sys_callout", delay, func, args);
}

static nomask void _F_sys_callout(string func, mixed *args)
{
	call_other(this_object(), func, args ...);
}

private mixed **process_callouts(mixed **callouts)
{
	int i, sz;

	if (callouts) {
		mixed *co;
		sz = sizeof(callouts);

		for (i = 0; i < sz; i++) {
			co = callouts[i];
			callouts[i] = ({ co[CO_HANDLE], co[CO_FIRSTXARG], co[CO_DELAY] }) + co[CO_FIRSTXARG + 1];
		}
	}

	return callouts;
}

static mixed status(varargs mixed obj, mixed index)
{
	mixed status;

	if (!this_object()) {
		return nil;
	}

	switch (typeof(obj)) {
	case T_NIL:
		CHECKARG(index == nil, 1, "status");
		status = ::status();
		if (status[ST_STACKDEPTH] >= 0) {
			status[ST_STACKDEPTH]++;
		}
		break;

	case T_INT:
		CHECKARG(index == nil, 1, "status");
		status = ::status(obj);
		if (obj == ST_STACKDEPTH && status >= 0) {
			status++;
		}
		break;

	case T_STRING:
	case T_OBJECT:
		if (!obj) {
			return nil;
		}

		switch (typeof(index)) {
		case T_NIL:
			status = ::status(obj);
			if (!status) {
				return nil;
			}
			status[O_CALLOUTS] = process_callouts(status[O_CALLOUTS]);
			break;

		case T_INT:
			status = ::status(obj, index);
			if (index == O_CALLOUTS) {
				status = process_callouts(status);
			}
			break;

		default:
			error("Bad argument");
		}
		break;
	}

	return status;
}
#endif
