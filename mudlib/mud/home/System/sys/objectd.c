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

/* internal */

static void create()
{
	progdb = clone_object(BIGSTRUCT_
	progdb = 
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
