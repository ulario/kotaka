#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit "call_guard";

static int free_objects()
{
	return status(ST_OTABSIZE) - status(ST_NOBJECTS);
}

private void _F_call_constructors()
{
	object pinfo;
	string *ctors;
	int index;
	int sz;

	string base;

	pinfo = OBJECTD->query_object_info(status(this_object(), O_INDEX));

	if (!pinfo) {
		error("No object info for " + object_name(this_object()));
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

	pinfo = OBJECTD->query_object_info(status(this_object(), O_INDEX));

	if (!pinfo) {
		error("No object info for " + object_name(this_object()));
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

	if (!SYSTEM() && !obj && free_objects() < 50) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

static object clone_object(mixed args ...)
{
	if (!SYSTEM() && free_objects() < 100) {
		error("Too many objects");
	}

	return ::clone_object(args ...);
}
