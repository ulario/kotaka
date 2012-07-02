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
int ignore_bigstruct_clones;	/* don't track bigstruct clones */

object rqueue;		/* reentrance queue */
int in_objectd;		/* reentrance flag */

/* internal functions */

static void create();
private void scan_objects(string path, object libqueue, object objqueue, object notqueue);
private void register_object(string path, string *inherits, string *includes, string constructor, string destructor);
private string *fetch_from_initd(object initd, string path);
private mixed query_include_file(string compiled, string from, string path);
private void enter_objectd();
private void exit_objectd();
private void flush_rqueue();
private void discover_self();

/* external functions */

void recompile_kernel_library();
void recompile_everything();
void disable();
void enable();
void discover_objects();
object query_object_info(int oindex);
object query_inheriters(int oindex);
object query_includers(string path);

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

	rqueue = new_object(BIGSTRUCT_DEQUE_LWO);
}

private int is_bigstruct(string path)
{
	return sscanf(path, USR_DIR + "/System" + CLONABLE_SUBDIR + "bigstruct/%*s");
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

	if (upgrading) {
		pinfo = objdb->get_element(oindex);
	}

	if (!pinfo) {
		pinfo = new_object(OBJECT_INFO);
		pinfo->set_path(path);

		enter_objectd();
		objdb->set_element(oindex, pinfo);
		exit_objectd();
	}

	pinfo->set_inherits(oindices);
	pinfo->set_includes(includes);
	pinfo->set_inherited_constructors(ctors);
	pinfo->set_constructor(constructor);
	pinfo->set_inherited_destructors(dtors);
	pinfo->set_destructor(destructor);
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

private void scan_objects(string path, object libqueue, object objqueue, object notqueue)
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
			scan_objects(path + "/" + name, libqueue, objqueue, notqueue);
			continue;
		}

		if (strlen(name) > 2 && name[strlen(name) - 2 ..] == ".c") {
			string opath;
			mixed *status;
			int oindex;

			opath = path + "/" + name[0 .. strlen(name) - 3];

			status = status(opath);

			if (!status) {
				if (notqueue) {
					notqueue->push_back(opath);
				}
				continue;
			}

			if (sscanf(opath, "%*s" + INHERITABLE_SUBDIR)) {
				if (libqueue) {
					libqueue->push_back(opath);
				}
			} else {
				if (objqueue) {
					objqueue->push_back(opath);
				}
			}
		}
	}
}

private void enter_objectd()
{
	in_objectd++;
}

private void exit_objectd()
{
	in_objectd--;

	if (in_objectd) {
		return;
	}

	flush_rqueue();
}

private void flush_rqueue()
{
	while (!rqueue->empty()) {
		mixed *front;
		object pinfo;

		enter_objectd();

		front = rqueue->get_front();
		rqueue->pop_front();

		pinfo = objdb->get_element(front[1]);

		if (!pinfo) {
			return;
		}

		if (front[0]) {
			pinfo->add_clone(front[2]);
		} else {
			pinfo->remove_clone(front[2]);
		}

		exit_objectd();
	}
}

private void discover_self()
{
	object node;
	int oindex;

	rqueue->push_back( ({ 1, status(objdb, O_INDEX), objdb }) );
	oindex = status("~/obj/bigstruct/map/node", O_INDEX);

	node = objdb->query_first_node();

	while (node) {
		rqueue->push_back( ({ 1, oindex, node }) );
		node = objdb->query_next_node(node);
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

object query_object_indices()
{
	object indices;

	indices = objdb->get_indices();
	indices->grant_access(previous_object(), READ_ACCESS);

	return indices;
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

		scan_objects("/", libqueue, objqueue, nil);

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
		object selfqueue;
		int sz, i;

		indices = objdb->get_indices();
		sz = indices->get_size();

		enter_objectd();

		ignore_bigstruct_clones++;

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
			int nclones;

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
					/* not a clone */
					current = KERNELD->next_link(current);
					continue;
				}

				if (current == objdb || current <- "~/lib/bigstruct/map/node" && current->query_root() == objdb) {
					/* one of ours */
					/* get it during self discover */
					current = KERNELD->next_link(current);
					continue;
				}

				oindex = status(current, O_INDEX);
				pinfo = objdb->get_element(oindex);
				pinfo->add_clone(current);
				current = KERNELD->next_link(current);
				nclones++;
			} while (current != first);
		}

		ignore_bigstruct_clones--;

		exit_objectd();

		discover_self();
		flush_rqueue();
	}
}

void full_reset()
{
	ACCESS_CHECK(PRIVILEGED());

	rlimits (0; -1) {
		LOGD->post_message("objectd", LOG_DEBUG, "Resetting object manager");

		ignore_bigstruct_clones = 0;
		ignore_bigstruct_clones++;

		destruct_object(objdb);
		rqueue = new_object(BIGSTRUCT_DEQUE_LWO);

		objdb = clone_object(BIGSTRUCT_MAP_OBJ);
		objdb->set_type(T_INT);

		ignore_bigstruct_clones--;

		discover_objects();
		discover_clones();
	}
}

object query_orphans()
{
	object orphans;
	object indices;
	int sz, i;

	orphans = new_object(BIGSTRUCT_ARRAY_LWO);
	orphans->grant_access(previous_object(), READ_ACCESS);

	indices = objdb->get_indices();
	sz = indices->get_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		string path;

		pinfo = objdb->get_element(indices->get_element(i));

		if (!file_info(path = pinfo->query_path() + ".c")) {
			orphans->push_back(path);
		}
	}

	return orphans;
}

object query_dormant()
{
	object notlist;

	notlist = new_object(BIGSTRUCT_ARRAY_LWO);
	notlist->grant_access(previous_object(), READ_ACCESS);

	scan_objects("/", nil, nil, notlist);

	return notlist;
}

atomic void defragment()
{
	enter_objectd();
	objdb->reindex();
	exit_objectd();
}

void audit_clones()
{
	ACCESS_CHECK(PRIVILEGED());

	rlimits (0; -1) {
		object indices;
		string *owners;
		object first;
		int orsz, odsz, i;

		object truecounts;

		truecounts = new_object(BIGSTRUCT_MAP_LWO);
		truecounts->set_type(T_INT);
		indices = objdb->get_indices();
		odsz = indices->get_size();

		for (i = 0; i < odsz; i++) {
			truecounts->set_element(indices->get_element(i), 0);
		}

		owners = KERNELD->query_owners();
		orsz = sizeof(owners);

		for (i = 0; i < orsz; i++) {
			object current;
			int nclones;

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
					/* not a clone */
					current = KERNELD->next_link(current);
					continue;
				}

				oindex = status(current, O_INDEX);
				truecounts->set_element(oindex,
					truecounts->get_element(oindex) + 1);
				current = KERNELD->next_link(current);
			} while (current != first);
		}

		ASSERT(truecounts->get_indices()->get_size() == odsz);

		for (i = 0; i < odsz; i++) {
			int mycount;
			int truecount;
			int oindex;
			object pinfo;

			oindex = indices->get_element(i);
			pinfo = objdb->get_element(oindex);
			mycount = pinfo->query_clone_count();
			truecount = truecounts->get_element(oindex);

			if (mycount != truecount) {
				DRIVER->message("ObjectD audit fail: " + pinfo->query_path() + ", recorded " + mycount + ", counted " + truecount + "\n");
			}
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
	string path;

	ACCESS_CHECK(KERNEL());

	path = object_name(obj);

	if (ignore_bigstruct_clones && is_bigstruct(path)) {
		return;
	}

	if (in_objectd) {
		rqueue->push_back( ({ 1, status(obj, O_INDEX), obj }) );
		return;
	}

	pinfo = objdb->get_element(status(obj, O_INDEX));

	if (!pinfo) {
		return;
	};

	enter_objectd();
	pinfo->add_clone(obj);
	exit_objectd();
}

void destruct(string owner, object obj)
{
	int isclone;
	string name;
	string path;
	object pinfo;

	ACCESS_CHECK(KERNEL());

	name = object_name(obj);
	isclone = sscanf(name, "%s#%*d", path);

	pinfo = objdb->get_element(status(obj, O_INDEX));

	if (!pinfo) {
		return;
	}

	if (!isclone) {
		path = name;
	}

	if (!sscanf(path, "/kernel/%*s")) {
		obj->_F_sys_destruct();
	}

	if (isclone) {
		if (!ignore_bigstruct_clones || !is_bigstruct(path)) {
			if (in_objectd) {
				rqueue->push_back( ({ 0, status(obj, O_INDEX), obj }) );
				return;
			}

			enter_objectd();
			pinfo->remove_clone(obj);
			exit_objectd();
		}
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

	enter_objectd();
	objdb->set_element(index, nil);
	exit_objectd();
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
	object initd;

	ACCESS_CHECK(KERNEL());

	initd = find_object(USR_DIR + "/" + DRIVER->creator(path) + "/initd");

	if (!initd) {
		error("No initd loaded for " + path);
	}

	return initd->forbid_inherit(from, path, priv);
}
