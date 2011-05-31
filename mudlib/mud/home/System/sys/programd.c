#include <kernel/kernel.h>
#include <kernel/access.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>

#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/bigstruct.h>

#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object programs;

object query_program_info(int index);
void build_program_info(int index, string path, int *inherits, string *includes, string constructor, string destructor);

string program;
string *includes;
string *inherits;
int mode;
string ctor;
string dtor;

int audit_preloads;

#define PRELOAD_UNDEF	0
#define PRELOAD_PROG	1
#define PRELOAD_INC	2
#define PRELOAD_INH	3
#define PRELOAD_CTOR	4
#define PRELOAD_DTOR	5

static void create()
{
	programs = clone_object(BIGSTRUCT_MAP_OBJ);
	programs->grant_global_access(READ_ACCESS);
	programs->set_type(T_INT);
}

private void end_program()
{
	string out;
	int i;
	int *indices;

	if (!program) {
		return;
	}

	if (!status(program)) {
		/* object doesn't exist, ignore it */
		/* objectd will capture current info */
		/* when it's compiled */
		program = nil;
		return;
	}

	if (!inherits) {
		inherits = ({ });
	}

	if (!includes) {
		includes = ({ });
	}

	indices = allocate(sizeof(inherits));

	for (i = 0; i < sizeof(indices); i++) {
		ASSERT(inherits[i]);
		ASSERT(status(inherits[i]));
		indices[i] = status(inherits[i])[O_INDEX];
	}

	build_program_info(status(program)[O_INDEX],
		program, indices, includes, ctor, dtor);

	if (audit_preloads) {	
		LOGD->post_message("program", LOG_NOTICE, "Preloaded " + program);
	}

	program = nil;
}

private void process_token(string token)
{
	int ok;
	
	switch (mode) {
	case PRELOAD_UNDEF:
		break;

	case PRELOAD_PROG:
		if (token[0] == '/') {
			if (program) {
				error("Too many programs");
			}
			program = token;

			includes = nil;
			inherits = nil;
			ctor = nil;
			dtor = nil;
			ok = 1;
		}
		break;

	case PRELOAD_INC:
		if (sizeof(includes & ({ token }) )) {
			error("Duplicate include: " + token);
		}
		
		if (token[0] == '/') {
			includes |= ({ token });

			ok = 1;
		}
		break;

	case PRELOAD_INH:
		if (sizeof(inherits & ({ token }) )) {
			error("Duplicate inherit: " + token);
		}
		
		if (token[0] == '/') {
			inherits |= ({ token });

			ok = 1;
		}

		break;
	
	case PRELOAD_CTOR:
		if (ctor) {
			error("Multiple constructors specified");
		}
		
		ctor = token;
		
		ok = 1;
		mode = PRELOAD_PROG;
		
		break;
	
	case PRELOAD_DTOR:
		if (dtor) {
			error("Multiple destructors specified");
		}
		
		dtor = token;

		ok = 1;
		mode = PRELOAD_PROG;
		
		break;
	}

	if (ok) return;

	switch (token) {
	case "Program":
		if (program) {
			end_program();
		}

		mode = PRELOAD_PROG;
		program = nil;
		break;
	case "Include":
		if (!program) {
			error("Include without Program");
		}

		if (includes) {
			error("Duplicate directive: Include ("
				+ implode(includes, "-") + ")");
		}

		mode = PRELOAD_INC;
		includes = ({ });
		break;

	case "Inherit":
		if (!program) {
			error("Inherit without Program");
		}

		if (inherits) {
			error("Duplicate directive: Inherit ("
				+ implode(inherits, "-") + ")");
		}

		mode = PRELOAD_INH;
		inherits = ({ });
		break;
	
	case "Constructor":
		if (!program) {
			error("Constructor without Program");
		}
		
		mode = PRELOAD_CTOR;
		
		break;
	
	case "Destructor":
		if (!program) {
			error("Destructor without Program");
		}
		
		mode = PRELOAD_DTOR;
		
		break;
		
	default:
		error("Invalid directive: " + token);
	}
}

private void end_file()
{
	if (program) {
		end_program();
	}
}

void preload()
{
	string grammar;
	string *list;
	int i;

	ACCESS_CHECK(SYSTEM());
	
	list = get_dir("~/data/*.programd")[0];

	for (i = 0; i < sizeof(list); i++) {
		string contents;
		string *words;
		string *lines;
		int j;
		string line;

		contents = read_file("~/data/" + list[i]);
		
		while (strlen(contents)) {
			string line;
			string *tokens;
			int k;
			
			sscanf(contents, "%s\n%s", line, contents);
			
			if (!line) {
				LOGD->post_message("program", LOG_WARNING,
					"Warning: no terminating newline");
				line = contents;
			}
			
			if (!strlen(line)) {
				continue;
			}

			if (line[0] == '#') {
				continue;
			}

			line = implode(explode(line, "\t"), " ");
			tokens = explode(line, " ") - ({ "" });

			for (k = 0; k < sizeof(tokens); k++) {
				process_token(tokens[k]);
			}
		}

		end_file();
	}
}

void set_destructed(int index)
{
	ACCESS_CHECK(SYSTEM());

	if (!query_program_info(index)) {
		return;
	}

	query_program_info(index)->set_destructed();
}

void clear_program_info(int index)
{
	ACCESS_CHECK(SYSTEM());

	programs->set_element(index, nil);
}

object query_program_info(int index)
{
	return programs->get_element(index);
}

void build_program_info(int pindex, string path, int *inherits, string *includes, string constructor, string destructor)
{
	int index;
	int sz;
	object pinfo;
	
	ACCESS_CHECK(SYSTEM());
	
	pinfo = new_object("~/lwo/program_info");
	
	pinfo->set_path(path);
	pinfo->set_inherits(inherits);
	pinfo->set_includes(includes);
	pinfo->set_constructor(constructor);
	pinfo->set_destructor(destructor);

	if (!inherits) {
		ASSERT(!includes);
		pinfo->set_ghost();
	} else {
		string *ctors;
		string *dtors;
		
		object subpinfo;
		
		int index;
		int sz;
		
		sz = sizeof(inherits);
		
		ctors = ({ });
		dtors = ({ });

		for (index = 0; index < sz; index++) {
			string *tmp;
			subpinfo = query_program_info(inherits[index]);
			
			if (subpinfo) {
				tmp = subpinfo->query_inherited_constructors();

				if (tmp) {
					ctors |= tmp;
				}

				tmp = subpinfo->query_inherited_destructors();

				if (tmp) {
					dtors |= tmp;
				}

				ctors |= ({ subpinfo->query_constructor() });
				dtors |= ({ subpinfo->query_destructor() });
			}
		}
		
		pinfo->set_inherited_constructors(ctors - ({ nil }));
		pinfo->set_inherited_destructors(dtors - ({ nil }));
	}

	programs->set_element(pindex, pinfo);
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
	
	buf += "Program " + path;
	
	ctor = pinfo->query_constructor();
	
	if (ctor) {
		buf += "\nConstructor " + ctor;
	}
	
	dtor = pinfo->query_destructor();
	
	if (dtor) {
		buf += "\nDestructor " + dtor;
	}
	
	if (sizeof(inh)) {
		buf += "\nInherit " + implode(inhp, "\n\t");
	}

	if (sizeof(inc)) {
		buf += "\nInclude " + implode(inc, "\n\t");
	}
	
	buf += "\n\n";
	
	write_file(file, buf);
}

object query_program_map()
{
	return programs;
}

object query_program_numbers()
{
	object indices;
	
	indices = programs->get_indices();
	indices->grant_global_access(0);
	indices->grant_access(previous_object(), FULL_ACCESS);
	indices->grant_access(this_object(), 0);

	return indices;
}

void defragment()
{
	programs->rebalance_heavyweight();
}
