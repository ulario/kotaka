#include <kotaka/privilege.h>
#include <kotaka/paths.h>

#include <trace.h>
#include <type.h>
#include <status.h>

inherit "call_guard";
inherit "touch";

/**********/
/* status */
/**********/

static int free_objects()
{
	return status()[ST_OTABSIZE] - status()[ST_NOBJECTS];
}

static int free_callouts()
{
	return status()[ST_COTABSIZE] -
		(status()[ST_NCOSHORT] + status()[ST_NCOLONG]);
}

/*********/
/* calls */
/*********/

static mixed call_other_unprotected(varargs mixed args ...)
{
	return ::call_other(args ...);
}

static mixed **call_trace_unprotected()
{
	return ::call_trace();
}

static string previous_program_unprotected(varargs int steps)
{
	return ::previous_program(steps);
}

static object calling_object(varargs int steps)
{
	mixed **trace;
	
	trace = call_trace();
	
	return find_object(trace[sizeof(trace) - (3 + steps)][TRACE_OBJNAME]);
}

#if 1
static int call_out(string function, mixed delay, mixed args ...)
{
	if (!this_object()) {
		error("Cannot call_out from destructed object");
	}
	
	if (!function_object(function, this_object())) {
		error("Callout to undefined function " + function);
	}
	
	if (!SYSTEM() && free_callouts() < 20) {
		error("Too many callouts");
	}
	
	return ::call_out(function, delay, args ...);
}
#endif

/**********/
/* object */
/**********/

static object compile_object(mixed args ...)
{
	object obj;
	
	obj = find_object(args[0]);
	
	if (!SYSTEM() && !obj && free_objects() < 50) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

static object load_object(mixed args ...)
{
	object obj;
	
	obj = find_object(args[0]);
	
	if (obj) {
		return obj;
	}

	if (!SYSTEM() && !find_object(args[0]) && free_objects() < 50) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

static object clone_object(mixed args ...)
{
	if (!SYSTEM() && free_objects() < 500) {
		error("Too many objects");
	}

	return ::clone_object(args ...);
}

/*********/
/* hooks */
/*********/

private void _F_call_constructors()
{
	object pinfo;
	string *ctors;
	int index;
	int sz;
	
	string base;
	
	pinfo = PROGRAMD->query_program_info(status(this_object())[O_INDEX]);
	
	if (!pinfo) {
		INITD->message("No program info for " + object_name(this_object()));
		return;
	}
	
	ctors = pinfo->query_inherited_constructors();
	sz = sizeof(ctors);
	
	for (index = 0; index < sz; index++) {
		call_other(this_object(), ctors[index]);
	}
}

private void _F_call_destructors()
{
	object pinfo;
	string *dtors;
	int index;
	int sz;
	
	string base;
	
	pinfo = PROGRAMD->query_program_info(status(this_object())[O_INDEX]);

	if (!pinfo) {
		INITD->message("No program info for " + object_name(this_object()));
		return;
	}

	dtors = pinfo->query_inherited_destructors();
	
	sz = sizeof(dtors);
	
	for (index = sz - 1; index > 0; index--) {
		call_other(this_object(), dtors[index]);
	}
}

static int _F_sys_create(int clone)
{
	object pinfo;
	string base;
	string oname;
	
	string *ctors;
	
	int index;
	int sz;
	
	oname = object_name(this_object());
	base = oname;
	sscanf(base, "%s#%*d", base);
	
	if (DRIVER->creator(base) != "System") {
		_F_call_constructors();
	}
}

static void destruct(varargs int clone)
{
}

nomask void _F_sys_destruct()
{
	object pinfo;
	string oname;
	string base;
	
	string *dtors;
	
	int clone;
	
	int index;
	int sz;

	ACCESS_CHECK(previous_program() == OBJECTD);

	oname = object_name(this_object());
	base = oname;
	sscanf(base, "%s#%*d", base);
	clone = !!sscanf(oname, "%*s#");

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		destruct();
	} else {
		destruct(clone);
	}

	if (DRIVER->creator(base) != "System") {
		_F_call_destructors();
	}
}

void upgrading()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

void upgrade_failed()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

nomask void _F_dummy()
{
}
