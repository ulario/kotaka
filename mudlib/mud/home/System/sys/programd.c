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

static void create()
{
	programs = clone_object(BIGSTRUCT_MAP_OBJ);
	programs->grant_global_access(READ_ACCESS);
	programs->set_type(T_INT);
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
