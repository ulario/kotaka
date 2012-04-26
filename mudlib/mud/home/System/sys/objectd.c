/** Object manager

Keeps things straight wrt versions and all, and triggers the proper hooks.

Please see "About ObjectD" in the general docs for more information.
*/
#include <status.h>

#include <kernel/kernel.h>
#include <kernel/access.h>
#include <kernel/rsrc.h>

#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/bigstruct.h>

inherit SECOND_AUTO;

/*************/
/* Variables */
/*************/

string *includes;

int forcing;	/* If the current destruct or compile is being forced */
int discovery;	/* If inheritance discovery is being performed */
/* the above flag suppresses compilation messages */
int upgrading;	/* distinguishes between fresh compile and upgrade */
int grpending;	/* global recompile pending */

int sys_protect;	/* Prevent recompile and destruct of system */
int klib_protect;	/* Prevent recompile and destruct of klib */

/* audit buttons */
int audit_programs;	/* audits the compilation, recompilation, or destruction of a master object */
int audit_clones;	/* audits the cloning or destruction of a clone object */

/****************/
/* Declarations */
/****************/

/***************/
/* Definitions */
/***************/

/* basic */

static void create()
{
}

/** admin **/

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

void enable_programs_audit()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	audit_programs = 1;
}

void disable_programs_audit()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	audit_programs = 0;
}

void enable_clones_audit()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	audit_clones = 1;
}

void disable_clones_audit()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	audit_clones = 0;
}

void klib_recompile()
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

/* common */
private void compiled_program(string path, string *inherited, string *includes, string ctor, string dtor)
{
	int *indices;
	int oindex;
	int index;
	int sz;
	
	sz = sizeof(inherited);

	indices = allocate(sz);
	oindex = status(path)[O_INDEX];

	for (index = 0; index < sz; index++) {
		indices[index] = status(inherited[index])[O_INDEX];

		if (!PROGRAMD->query_program_info(indices[index])) {
			if (audit_programs) {
				LOGD->post_message("program",
					LOG_INFO,
					"Discovered ghost program: "
					+ inherited[index]
				);
			}

			PROGRAMD->build_program_info(indices[index],
				inherited[index], nil, nil, nil, nil);
		}
	}

	PROGRAMD->build_program_info(oindex,
		path, indices, includes, ctor, dtor);

	if (audit_programs) {
		LOGD->post_message("program", LOG_NOTICE, "Compiled " + path);
	}
}

private void destructed_program(string path)
{
	if (audit_programs) {
		LOGD->post_message("program", LOG_INFO,
			"Destructed " + path);
	}
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
		return "/home/System/include/second_auto.h";
	}

	if (initd = find_object(USR_DIR + "/" + creator + "/initd")) {
		return initd->include_file(compiled, from, path);
	}

	return path;
}

void global_recompile()
{
	ACCESS_CHECK(PRIVILEGED() || KADMIN());

	LOGD->post_message("system", LOG_NOTICE,
		"Global recompile beginning...");

	rlimits(0; -1) {
		object libs;
		object objs;

		libs = new_object(BIGSTRUCT_DEQUE_LWO);
		objs = new_object(BIGSTRUCT_DEQUE_LWO);

		catch {
			int i;
			object pids;

			int l;
			int o;
			int sz;

			grpending = 0;
			pids = PROGRAMD->query_program_numbers();
			sz = pids->get_size();
			
			for (i = 0; i < sz; i++) {
				string path;
				object pinfo;

				pinfo = PROGRAMD->query_program_info(pids->get_element(i));
				path = pinfo->query_path();

				/* do not tamper with the klib */
				if (sscanf(path, "/kernel/%*s")) {
					continue;
				}

				if (sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
					libs->push_back(path);
				} else {
					objs->push_back(path);
				}
			}

			while (!libs->empty()) {
				string path;

				path = libs->get_front();
				libs->pop_front();

				if (status(path)) {
					destruct_object(path);
				}
			}

			while (!objs->empty()) {
				string path;

				path = objs->get_front();
				objs->pop_front();

				if (status(path)) {
					catch {
						compile_object(path);
					}
				}
			}
		}
	}

	LOGD->post_message("system", LOG_NOTICE,
		"Global recompile completed.");
}

void force_destruct(mixed obj)
{
	ACCESS_CHECK(KADMIN());

	forcing = 1;

	catch {
		destruct_object(obj);
	}

	forcing = 0;
}

void force_compile(string path)
{
	ACCESS_CHECK(KADMIN());

	forcing = 1;

	catch {
		compile_object(path);
	}

	forcing = 0;
}

private void scan(string path, object queue)
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
			scan(path + "/" + name, queue);
			continue;
		}
		
		if (strlen(name) > 2 && name[strlen(name) - 2 ..] == ".c") {
			string opath;
			mixed *status;
			int oindex;
			
			opath = path + "/" + name[0 .. strlen(name) - 3];
			
			status = status(opath);
			
			/* unregistered */
			if (status &&
				!PROGRAMD->query_program_info(
				oindex = status[O_INDEX])) {
				
				queue->push_back(opath);
				PROGRAMD->build_program_info(oindex, opath, nil, nil, nil, nil);
			}
		}
	}
}

void register_programs(object queue);

void scan_dirs(string path)
{
	object queue;

	ACCESS_CHECK(SYSTEM() || KADMIN());

	queue = new_object(BIGSTRUCT_DEQUE_LWO);

	scan(path, queue);

	register_programs(queue);
}

private void write_preload(string path);

void register_programs(object queue)
{
	int lc;
	int cc;
	int rc;
	int sc;
	
	object lq; /* library */
	object cq; /* compile */
	object rq; /* recompile */
	object sq; /* save */
	
	ACCESS_CHECK(SYSTEM());

	lq = new_object(BIGSTRUCT_DEQUE_LWO);
	rq = new_object(BIGSTRUCT_DEQUE_LWO);
	cq = new_object(BIGSTRUCT_DEQUE_LWO);

	while (!queue->empty()) {
		string path;
		
		path = queue->get_front();
		queue->pop_front();
		
		if (sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
			lq->push_back(path);
			rq->push_back(path);
		} else {
			cq->push_back(path);
		}
	}

	while (!lq->empty()) {
		string path;

		lc++;

		path = lq->get_front();
		lq->pop_front();

		destruct_object(path);
	}

	while (!cq->empty()) {
		string path;
		
		cc++;
		
		path = cq->get_front();
		cq->pop_front();
		
		compile_object(path);
	}

	while (!rq->empty()) {
		string path;
		
		rc++;
		
		path = rq->get_front();
		rq->pop_front();
		
		if (!status(path)) {
			compile_object(path);
		}
	}
}

private void write_preload(string path)
{
	object pinfo;
	string file;
	int *inh;
	string *inhp;
	string *inc;
	int sz;
	int index;
	string ctor;
	string dtor;
	
	string buf;
	
	buf = "";
	
	if (sscanf(path, "/kernel/%*s")) {
		file = "~/data/kernel.programd";
	} else {
		file = "~/data/system.programd";
	}
	
	pinfo = PROGRAMD->query_program_info(status(path)[O_INDEX]);
	
	if (!pinfo) {
		error("No program info for " + path);
	}
	
	inh = pinfo->query_inherits();

	sz = sizeof(inh);
	inhp = allocate(sz);
	
	for (index = 0; index < sz; index++) {
		inhp[index] = PROGRAMD->
			query_program_info(inh[index])->
			query_path();
	}

	inc = pinfo->query_includes();
	
	ctor = pinfo->query_constructor();
	dtor = pinfo->query_destructor();
	
	buf += "Program " + path;

	if (sizeof(inh)) {
		buf += "\nInherit " + implode(inhp, "\n\t");
	}

	if (sizeof(inc)) {
		buf += "\nInclude " + implode(inc, "\n\t");
	}
	
	if (ctor) {
		buf += "\nConstructor " + ctor;
	}
	
	if (dtor) {
		buf += "\nDestructor " + dtor;
	}
	
	buf += "\n\n";
	
	write_file(file, buf);
}

/**************/
/* Klib hooks */
/**************/

/* sorted */

void compiling(string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		object user;
		object obj;
		
		int by_kadmin;
		int is_kernel;
		int is_system;
		
		user = this_user();

		by_kadmin = user && (user <- "/kernel/obj/user")
			&& (user->query_name() == "admin");
		
		is_kernel = sscanf(path, "/kernel/%*s");
		is_system = sscanf(path, USR_DIR + "/System/%*s");
		
		obj = find_object(path);
		
		if (is_kernel) {
		} else if (is_system) {
		} else {
		}

		if (obj) {
			if (!forcing && obj <- SECOND_AUTO) {
				catch {
					obj->upgrading();
				}
			}
			upgrading = 1;
		}

		if (path == DRIVER || path == AUTO) {
			includes = ({ });
		} else {
			includes = ({ "/include/std.h" });
		}
	} : {
		error("Internal error in ObjectD");
	}
}

void compile(string owner, object obj, string *sources,
	string inherited ...)
{
	int *indices;
	int index;
	string path;
	int upgraded;

	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		upgraded = upgrading;
		upgrading = 0;

		path = object_name(obj);

		if (upgraded) {
			if (!forcing && obj <- SECOND_AUTO) {
				obj->upgrade();
			}
		}

		if (object_name(obj) != DRIVER) {
			inherited |= ({ AUTO });
		}

		compiled_program(path, inherited, includes, nil, nil);

		includes = nil;
	} : {
		error("Internal error in ObjectD");
	}
}

void compile_lib(string owner, string path, string *sources,
	string inherited ...)
{
	int *indices;
	int index;

	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		string ctor;
		string dtor;
		string err;
		object initd;
		
		upgrading = 0;

		if (path != AUTO) {
			inherited |= ({ AUTO });
		}
		
		initd = find_object(USR_DIR + "/" + owner + "/initd");

		if (initd) {
			catch {
				ctor = initd->query_constructor(path);
			}
			catch {
				dtor = initd->query_destructor(path);
			}
		}
		
		err = catch(compiled_program(path, inherited, includes, ctor, dtor));
		includes = nil;
		
		if (err) {
			destruct_object(path);
			error(err);
		}
	} : {
		destruct_object(path);
		error("Internal error in ObjectD");
	}
}

void compile_failed(string owner, string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		if (upgrading) {
			object obj;

			upgrading = 0;
			obj = find_object(path);

			if (!forcing && obj <- SECOND_AUTO) {
				catch {
					obj->upgrade_failed();
				}
			}
		}

		LOGD->post_message("program", LOG_ERR,
			"Failed to compile " + path);

		includes = ({ });
	} : {
		error("Internal error in ObjectD");
	}
}

void clone(string owner, object obj)
{
	ACCESS_CHECK(previous_program() == DRIVER);
}

void destruct(string owner, object obj)
{
	string base;
	string name;
	int index;

	ACCESS_CHECK(previous_program() == DRIVER);
	
	catch {
		index = status(obj)[O_INDEX];
		name = object_name(obj);

		if (!sscanf(name, "%s#%*d", base)) {
			base = name;
		}

		if (!forcing) {
			switch (base) {
			case USR_DIR + "/System/sys/freezed":
			case USR_DIR + "/System/sys/objectd":
			case USR_DIR + "/System/sys/programd":
				error("Permission denied");
			}

			if (obj <- SECOND_AUTO) {
				obj->_F_sys_destruct();
			}
		}

		if (!sscanf(name, "%s#%*d", base)) {
			PROGRAMD->set_destructed(index);
			/* latent */
			destructed_program(base);
		} else {
			if (audit_clones) {
				/* INITD->message("ObjectD: destructed " + name); */
			}
		}
	} : {
		error("Internal error in ObjectD");
	}
}

void destruct_lib(string owner, string path)
{
	int index;

	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		index = status(path)[O_INDEX];

		PROGRAMD->set_destructed(index);

		destructed_program(path);
	} : {
		error("Internal error in ObjectD");
	}
}

void remove_program(string owner, string path, int timestamp, int index)
{
	object pinfo;
	
	ACCESS_CHECK(previous_program() == DRIVER);
	pinfo = PROGRAMD->query_program_info(index);
	
	if (audit_programs) {
		if (pinfo) {
			if (pinfo->query_ghost()) {
				LOGD->post_message("program", LOG_INFO,
					"Removed " + pinfo->query_path() + " (ghost)");
			} else {
				LOGD->post_message("program", LOG_INFO,
					"Removed " + pinfo->query_path());
			}
		} else {
			LOGD->post_message("program", LOG_INFO,
				"Removal of unknown program, object index " + index);
		}
	}

	PROGRAMD->clear_program_info(index);
}

mixed include_file(string compiled, string from, string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	path = query_include_file(compiled, from, path);

	includes |= ({ path });

	return path;
}

int touch(object obj, string func)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		if (obj <- SECOND_AUTO) {
			return 0;
		} else {
			return 0;
		}
	} : {
		error("Internal error in ObjectD");
	}
}

int forbid_call(string path)
{
	string creator;
	object initd;

	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		creator = find_object(DRIVER)->creator(path);

		if (creator == nil) {
			return 0;
		}

		initd = find_object("~" + creator + "/initd");

		if (initd) {
			return !!initd->forbid_call(path);
		}

		return 0;
	} : {
		error("Internal error in ObjectD");
	}
}

int forbid_inherit(string from, string path, int priv)
{
	string creator;
	object initd;

	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		creator = find_object(DRIVER)->creator(path);

		if (creator == nil) {
			return 0;
		}

		initd = find_object("~" + creator + "/initd");

		if (initd) {
			return !!initd->forbid_inherit(from, path,
				priv);
		}

		return 0;
	} : {
		error("Internal error in ObjectD");
	}
}

void scan_clones(string name)
{
	call_out("scan_clones_step", 0, name, 0);
}

static void scan_clones_step(string name, int index)
{
	int max;
	int quota;

	max = status()[ST_OTABSIZE];
	quota = (int)sqrt((float)(max - index));

	for (;
		quota > 0 && index < max;
		quota--, index++)
	{
		if (find_object(name + "#" + index)) {
			LOGD->post_message("clone", LOG_INFO,
				"Found " + name + "#" + index);
		}
	}

	if (index < max) {
		call_out("scan_clones_step", 0, name, index);
	} else {
		LOGD->post_message("clone", LOG_INFO, "Clone scan for " + name + " completed");
	}
}

void purge_clones(string name)
{
	int max;

	ACCESS_CHECK(KADMIN());

	max = status()[ST_OTABSIZE];

	call_out("purge_clones_step", 0, name, max, 0);
}

static void purge_clones_step(string name, int index, int npurge)
{
	int quota;

	quota = (int)sqrt((float)index);

	while (index > 0 && quota > 0) {
		object obj;
		index--;
		quota--;

		if (obj = find_object(name + "#" + index)) {
			destruct_object(obj);
			npurge++;
		}
	}

	if (index > 0) {
		call_out("purge_clones_step", 0, name, index, npurge);
		LOGD->post_message("clone", LOG_INFO, "Clone purge for "
			+ name + " at " + index + ", "
			+ npurge + " clones purged");
	} else {
		LOGD->post_message("clone", LOG_INFO, "Clone scan for " + name + " completed");
	}
}
