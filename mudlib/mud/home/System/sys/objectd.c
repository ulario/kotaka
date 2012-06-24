#include <kernel/kernel.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

/****************/
/* Declarations */
/****************/

/* variables */

string compiling;	/* path of object we are currently compiling */
string *includes;	/* include files of currently compiling object */
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

/* internal */

static void create()
{
	progdb = clone_object(BIGSTRUCT_
	progdb = 
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

/* kernel library hooks */

void compiling(string path)
{
}

void compile(string owner, object obj, string *source, string inherited ...)
{
}

void compile_lib(string owner, string path, string *source, string inherited ...)
{
}

void compile_failed(string owner, string path)
{
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
