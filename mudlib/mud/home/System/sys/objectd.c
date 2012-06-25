#include <kernel/kernel.h>
#include <kernel/access.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <type.h>
#include <status.h>

inherit SECOND_AUTO;

/****************/
/* Declarations */
/****************/

/* variables */

string compiling;	/* path of object we are currently compiling */
string *includes;	/* include files of currently compiling object */
int upgrading;		/* are we upgrading or making a new compile? */

object objdb;		/* object database */

/* internal functions */

static void create();
private void scan_objects(string path, object libqueue, object objqueue);
private void register_object(string path, string *inherits, string *includes, string constructor, string destructor);
private string *fetch_from_initd(object initd, string path);
private mixed query_include_file(string compiled, string from, string path);

/* external functions */

void recompile_kernel_library();
void recompile_everything();
void disable();
void enable();
void discover_objects();
object query_object_info(int oindex);
object query_inheritors(int oindex);

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

/* internal */

static void create()
{
	objdb = clone_object(BIGSTRUCT_MAP_OBJ);
	objdb->set_type(T_INT);
}

private void register_object(string path, string *inherits,
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

		suboindex = status(inherits[i])[O_INDEX];
		oindices[i] = suboindex;
		subpinfo = objdb->get_element(suboindex);

		if (subpinfo) {
			ctors |= subpinfo->query_inherited_constructors();
			ctors |= ({ subpinfo->query_constructor() });
			dtors |= subpinfo->query_inherited_destructors();
			dtors |= ({ subpinfo->query_destructor() });
		}
	}

	ctors -= ({ nil });
	dtors -= ({ nil });

	pinfo = new_object(OBJECT_INFO);
	pinfo->set_path(path);
	pinfo->set_inherits(oindices);
	pinfo->set_includes(includes);
	pinfo->set_inherited_constructors(ctors);
	pinfo->set_constructor(constructor);
	pinfo->set_inherited_destructors(dtors);
	pinfo->set_destructor(destructor);
	objdb->set_element(oindex, pinfo);
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

private mixed query_include_file(string compiled, string from, string path)
{
	string creator;
	object initd;

	/* don't allow bypass of standard file */
	if (path == "/include/std.h") {
		return path;
	}

	creator = find_object(DRIVER)->creator(compiled);

	/* System has to be direct */
	if (creator == "System") {
		return path;
	}

	/* don't allow bypass of standard auto */
	if (creator != "System" &&
		path == "/include/AUTO" &&
		from == "/include/std.h") {
		return USR_DIR + "/System/include/second_auto.h";
	}

	if (initd = find_object(USR_DIR + "/" + creator + "/initd")) {
		return initd->include_file(compiled, from, path);
	}

	return path;
}

private void scan_objects(string path, object libqueue, object objqueue)
{
	string *names;
	int *sizes;
	mixed **dir;
	int i;

	path = find_object(DRIVER)->normalize_path(path, "/");

	dir = get_dir(path + "/*");
	names = dir[0];
	sizes = dir[1];

	for (i = 0; i < sizeof(names); i++) {
		string name;
		string opath;

		name = names[i];

		if (sizes[i] == -2) {
			scan_objects(path + "/" + name, libqueue, objqueue);
			continue;
		}

		if (strlen(name) > 2 && name[strlen(name) - 2 ..] == ".c") {
			string opath;
			mixed *status;
			int oindex;

			opath = path + "/" + name[0 .. strlen(name) - 3];

			status = status(opath);

			if (!status) {
				continue;
			}

			if (sscanf(opath, "%*s" + INHERITABLE_SUBDIR)) {
				libqueue->push_back(opath);
			} else {
				objqueue->push_back(opath);
			}
		}
	}
}

/* external */

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	DRIVER->set_object_manager(this_object());
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	DRIVER->set_object_manager(nil);
}

object query_object_info(int oindex)
{
	return objdb->get_element(oindex);
}

object query_inheriters(int oindex)
{
	object inheriters;
	object indices;
	int i, sz;

	indices = objdb->get_indices();
	inheriters = new_object(BIGSTRUCT_ARRAY_LWO);
	inheriters->grant_access(previous_object(), READ_ACCESS);

	sz = indices->get_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		int suboindex;

		suboindex = indices->get_element(i);
		pinfo = objdb->get_element(suboindex);

		if (sizeof(pinfo->query_inherits() & ({ oindex }))) {
			inheriters->push_back(suboindex);
		}
	}

	return inheriters;
}

object query_includers(string path)
{
	object includers;
	object indices;
	int i, sz;

	indices = objdb->get_indices();
	includers = new_object(BIGSTRUCT_ARRAY_LWO);
	includers->grant_access(previous_object(), READ_ACCESS);

	sz = indices->get_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		int suboindex;

		suboindex = indices->get_element(i);
		pinfo = objdb->get_element(suboindex);

		if (sizeof(pinfo->query_includes() & ({ path }))) {
			includers->push_back(suboindex);
		}
	}

	return includers;
}

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
	object indices;
	object libqueue;
	object objqueue;
	int i;
	int sz;

	ACCESS_CHECK(PRIVILEGED());

	libqueue = new_object(BIGSTRUCT_DEQUE_LWO);
	objqueue = new_object(BIGSTRUCT_DEQUE_LWO);

	rlimits(0; -1) {
		indices = objdb->get_indices();
		sz = indices->get_size();

		for (i = 0; i < sz; i++) {
			int oindex;
			object pinfo;
			string path;

			oindex = indices->get_element(i);
			pinfo = objdb->get_element(oindex);
			path = pinfo->query_path();

			if (sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
				libqueue->push_back(path);
			} else {
				objqueue->push_back(path);
			}
		}

		while (!libqueue->empty()) {
			string path;

			path = libqueue->get_front();
			libqueue->pop_front();

			destruct_object(path);
		}

		while (!objqueue->empty()) {
			string path;

			path = objqueue->get_front();
			objqueue->pop_front();

			catch {
				compile_object(path);
			}
		}
	}
}

void discover_objects()
{
	object libqueue;
	object objqueue;

	ACCESS_CHECK(PRIVILEGED());

	rlimits(0; -1) {
		libqueue = new_object(BIGSTRUCT_DEQUE_LWO);
		objqueue = new_object(BIGSTRUCT_DEQUE_LWO);

		LOGD->post_message("object", LOG_DEBUG, "Discovering objects");

		scan_objects("/", libqueue, objqueue);

		LOGD->post_message("object", LOG_DEBUG, "Resetting objects");

		while (!libqueue->empty()) {
			string path;

			path = libqueue->get_front();
			libqueue->pop_front();
			
			destruct_object(path);
		}

		while (!objqueue->empty()) {
			string path;

			path = objqueue->get_front();
			objqueue->pop_front();

			compile_object(path);
		}
	}
}

void discover_clones()
{
	ACCESS_CHECK(PRIVILEGED());

	rlimits (0; -1) {
		object indices;
		string *owners;
		object first;
		int sz, i;

		indices = objdb->get_indices();
		sz = indices->get_size();

		for (i = 0; i < sz; i++) {
			object pinfo;
			int count;

			pinfo = objdb->get_element(indices->get_element(i));
			pinfo->reset_clones();
		}

		owners = KERNELD->query_owners();
		sz = sizeof(owners);

		for (i = 0; i < sz; i++) {
			object current;

			first = KERNELD->first_link(owners[i]);

			current = first;
			
			if (!current) {
				continue;
			}

			do {
				int oindex;
				string path;
				object pinfo;

				path = object_name(current);

				if (!sscanf(path, "%*s#%*d")) {
					current = KERNELD->next_link(current);
					continue;
				}

				LOGD->post_message("object", LOG_DEBUG, "Discovered clone: " + path);
				oindex = status(current, O_INDEX);
				pinfo = objdb->get_element(oindex);
				pinfo->add_clone(current);
				current = KERNELD->next_link(current);
			} while (current != first);
		}
	}
}

/* kernel library hooks */

void compiling(string path)
{
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
	string path;
	string err;
	int is_kernel;
	int is_auto;

	ACCESS_CHECK(KERNEL());

	path = object_name(obj);

	if (path != DRIVER) {
		inherited |= ({ AUTO });
	}

	register_object(path, inherited, includes, nil, nil);
	includes = nil;

	is_kernel = sscanf(path, "/kernel/%*s");

	if (is_kernel) {
		return;
	}

	is_auto = sscanf(path, USR_DIR + "/System"
		+ INHERITABLE_SUBDIR + "auto/%*s");

	if (!is_auto && !sizeof(({ SECOND_AUTO }) & inherited)) {
		error("Failure to inherit SECOND_AUTO: " + path);
	}

	if (upgrading) {
		upgrading = 0;

		obj->upgrading();
	}
}

void compile_lib(string owner, string path, string *source, string inherited ...)
{
	string err;
	string ctor;
	string dtor;
	object initd;
	int is_kernel;
	int is_auto;

	ACCESS_CHECK(KERNEL());

	if (path != AUTO) {
		inherited |= ({ AUTO });
	}

	is_kernel = sscanf(path, "/kernel/%*s");

	if (is_kernel) {
		return;
	}

	is_auto = sscanf(path, USR_DIR + "/System"
		+ INHERITABLE_SUBDIR + "auto/%*s");

	if (!is_auto && !sizeof(({ SECOND_AUTO }) & inherited)) {
		error("Failure to inherit SECOND_AUTO: " + path);
	}

	if (!is_kernel) {
		initd = find_object(USR_DIR + "/" + owner + "/initd");
	}

	if (initd) {
		string *ret;

		ret = fetch_from_initd(initd, path);

		err = ret[0];
		ctor = ret[1];
		dtor = ret[2];
	}

	register_object(path, inherited, includes, ctor, dtor);
	includes = nil;

	if (err) {
		error(err);
	}
}

void compile_failed(string owner, string path)
{
	ACCESS_CHECK(KERNEL());

	upgrading = 0;
	includes = nil;
}

void clone(string owner, object obj)
{
	object pinfo;

	ACCESS_CHECK(KERNEL());

	pinfo = objdb->get_element(status(obj)[O_INDEX]);
	ASSERT(pinfo);

	pinfo->add_clone(obj);
}

void destruct(string owner, object obj)
{
	int isclone;
	string path;
	object pinfo;

	ACCESS_CHECK(KERNEL());

	path = object_name(obj);
	isclone = sscanf(path, "%*s#%*d");

	pinfo = objdb->get_element(status(obj)[O_INDEX]);
	ASSERT(pinfo);

	if (!isclone) {
		if (pinfo->query_clone_count()) {
			error("Cannot destruct object with outstanding clones");
		}
	}

	if (!sscanf(path, "/kernel/%*s")) {
		obj->_F_sys_destruct();
	}

	if (isclone) {
		pinfo->remove_clone(obj);
	} else {
		pinfo->set_destructed();
	}
}

void destruct_lib(string owner, string path)
{
	object pinfo;

	ACCESS_CHECK(KERNEL());

	pinfo = objdb->get_element(status(path)[O_INDEX]);

	if (!pinfo) {
		return;
	}

	pinfo->set_destructed();
}

void remove_program(string owner, string path, int timestamp, int index)
{
	ACCESS_CHECK(KERNEL());

	objdb->set_element(index, nil);
}

mixed include_file(string compiled, string from, string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	path = query_include_file(compiled, from, path);

	includes |= ({ path });

	return path;
}

int touch(object obj, string function)
{
	ACCESS_CHECK(KERNEL());
}

int forbid_call(string path)
{
	ACCESS_CHECK(KERNEL());
}

int forbid_inherit(string from, string path, int priv)
{
	ACCESS_CHECK(KERNEL());
}
