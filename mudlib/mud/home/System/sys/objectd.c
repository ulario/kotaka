#include <kernel/kernel.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

/****************/
/* Declarations */
/****************/

/* variables */

string compiling;	/* path of object we are currently compiling */
string *includes;	/* include files of currently compiling object */
int upgrading;		/* are we upgrading or making a new compile? */

object progdb;		/* program database */

/* external functions */

void recompile_kernel_library();
void recompile_everything();
void disable();
void enable();
void discover_programs();

/* internal functions */

static void create();
private void scan_programs(string path);
private void register_program(string path, string *inherits, string *includes);
private string *fetch_from_initd(object initd, string path);

/* kernel library hooks */

void compiling(string path);
void compile(string owner, object obj, string *source, string inherited ...);
void compile_lib(string owner, string path, string *source, string inherited ...);
void compile_failed(string owner, string path);
void clone(string owner, object obj);
void destruct(string owner, object obj);
void destruct_lib(string owner, string path);
void remove_program(string owner, string path, int timestamp, int index);
mixed include_file(string compiled, string from, string path);
int touch(object obj, string function);
int forbid_call(string path);
int forbid_inherit(string from, string path, int priv);

/***************/
/* Definitions */
/***************/

/* external */

void recompile_kernel_library()
{
	string *names;
	mixed **dir;
	int i;

	ACCESS_CHECK(PRIVILEGED());

	dir = get_dir("/kernel/lib/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0 .. strlen(name) - 3];

		destruct_object("/kernel/lib/" + name);
	}

	dir = get_dir("/kernel/lib/api/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0..strlen(name) - 3];

		destruct_object("/kernel/lib/api/" + name);
	}

	dir = get_dir("/kernel/obj/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0..strlen(name) - 3];

		if (find_object("/kernel/obj/" + name)) {
			compile_object("/kernel/obj/" + name);
		}
	}

	dir = get_dir("/kernel/sys/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0..strlen(name) - 3];

		if (find_object("/kernel/sys/" + name)) {
			compile_object("/kernel/sys/" + name);
		}
	}
}

void recompile_everything()
{
	ACCESS_CHECK(PRIVILEGED());

	error("Not yet implemented");
}

/* internal */

static void create()
{
	progdb = clone_object(BIGSTRUCT_MAP_OBJ);
	progdb->set_type(T_INT);
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	DRIVER->set_object_manager(nil);
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	DRIVER->set_object_manager(this_object());
}

private void register_program(string path, string *inherits,
	string *includes, string constructor, string destructor)
{
	int i;
	int sz;
	int oindex;
	object pinfo;
	int *oindices;
	string *ctors;
	string *dtors;

	ACCESS_CHECK(SYSTEM());

	oindex = status(path)[O_INDEX];

	sz = sizeof(inherits);
	oindices = allocate(sz);
	ctors = ({ });
	dtors = ({ });

	for (i = 0; i < sz; i++) {
		object subpinfo;
		int suboindex;

		oindices[i] = suboindex = status(inherits[i])[O_INDEX];
		subpinfo = progdb->get_element(suboindex);
		ctors |= subpinfo->query_inherited_constructors();
		ctors |= ({ subpinfo->query_constructor() });
		dtors |= subpinfo->query_inherited_destructors();
		dtors |= ({ subpinfo->query_destructor() });
	}

	pinfo = new_object(PROGRAM_INFO);
	pinfo->set_path(path);
	pinfo->set_inherits(inherits);
	pinfo->set_includes(includes);

	pinfo->set_inherited_constructors(ctors);
	pinfo->set_constructor(constructor);
	pinfo->set_inherited_destructors(dtors);
	pinfo->set_destructor(destructor);
	progdb->set_element(oindex, pinfo);
}

private string *fetch_from_initd(object initd, string path)
{
	string ctor;
	string dtor;
	string err;

	err = catch(ctor = initd->query_constructor(path));

	if (!err) {
		err = catch(dtor = initd->query_destructor(path));
	}

	return ({ err, ctor, dtor });
}

/* kernel library hooks */

void compiling(string path)
{
	object obj;

	ACCESS_CHECK(KERNEL());

	if (path == DRIVER || path == AUTO) {
		includes = ({ });
	} else {
		includes = ({ "/include/std.h" });
	}

	if (find_object(path)) {
		upgrading = 1;
	}
}

void compile(string owner, object obj, string *source, string inherited ...)
{
	object initd;
	string err;
	string ctor;
	string dtor;
	string path;

	ACCESS_CHECK(KERNEL());

	path = object_name(obj);

	if (upgrading) {
		obj->upgrading();
	}

	initd = find_object(USR_DIR + "/" + owner + "/initd");

	if (initd) {
		string *ret;

		ret = fetch_from_initd(initd, path);

		err = ret[0];
		ctor = ret[1];
		dtor = ret[2];
	}

	register_program(object_name(obj), inherited, includes, ctor, dtor);

	if (err) {
		error(err);
	}
}

void compile_lib(string owner, string path, string *source, string inherited ...)
{
	ACCESS_CHECK(KERNEL());

	register_program(object_name(obj), inherited, includes, nil, nil);
}

void compile_failed(string owner, string path)
{
	upgrading = 0;
}

void clone(string owner, object obj)
{
}

void destruct(string owner, object obj)
{
}

void destruct_lib(string owner, string path)
{
}

void remove_program(string owner, string path, int timestamp, int index)
{
}

mixed include_file(string compiled, string from, string path)
{
}

int touch(object obj, string function)
{
}

int forbid_call(string path)
{
}

int forbid_inherit(string from, string path, int priv)
{
}
