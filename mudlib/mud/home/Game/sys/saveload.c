/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <status.h>
#include <type.h>

object oindex2onum;
object objlist;	/* ({ idnum: obj, data }) */

/* public helper functions */

object *parse_object(string input)
{
	int onum;

	if (sscanf(input, "O:%d", onum)) {
		return ({ objlist->query_element(onum - 1)[0] });
	} else {
		error("Cannot parse: " + input);
	}
}

string sprint_object(object obj, varargs mapping seen)
{
	string path;
	int oindex;

	if (sscanf(object_name(obj), "%s#%d", path, oindex)) {
		switch(path) {
		case USR_DIR + "/Game/obj/thing":
			return "<O:" + oindex2onum->query_element(oindex) + ">";
		case USR_DIR + "/Text/obj/user":
			return "nil";
		default:
			error("Clone is not a thing (" + path + ")");
		}
	} else {
		return STRINGD->sprint_object(obj, seen);
	}
}

/* private helper functions */

private void put_object(object obj)
{
	int oindex;
	int sz;
	object *inv;

	ASSERT(sscanf(object_name(obj), "%*s#%d", oindex));

	if (oindex2onum->query_element(oindex) != nil) {
		return;
	}

	sz = objlist->query_size();
	oindex2onum->set_element(oindex, sz + 1);
	objlist->push_back( ({ obj, obj->save() }) );

	inv = obj->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		put_object(inv[sz]);
	}
}

private void purge_savedir()
{
	string *names;

	do {
		int sz;

		names = CONFIGD->get_dir("save/*")[0];

		if (names) {
			for (sz = sizeof(names); --sz >= 0; ) {
				CONFIGD->remove_file("save/" + names[sz]);
			}
		}
	} while (names && sizeof(names));
}

private void put_directory(string dir)
{
	mapping list;
	int *keys;
	string *names;
	int i, sz;

	list = CATALOGD->list_directory(dir);

	names = map_indices(list);
	keys = map_values(list);

	sz = sizeof(keys);

	for (i = 0; i < sz; i++) {
		if (keys[i] == 2) {
			if (dir) {
				put_directory(dir + ":" + names[i]);
			} else {
				put_directory(names[i]);
			}
		} else {
			if (dir) {
				put_object(CATALOGD->lookup_object(dir + ":" + names[i]));
			} else {
				put_object(CATALOGD->lookup_object(names[i]));
			}
		}
	}
}

/* public functions */

void save_world()
{
	int sz;

	oindex2onum = new_object(BIGSTRUCT_MAP_LWO);
	oindex2onum->set_type(T_INT);

	objlist = new_object(BIGSTRUCT_ARRAY_LWO);
	purge_savedir();

	put_directory(nil);

	CONFIGD->make_dir(".");
	CONFIGD->make_dir("save");

	for (sz = objlist->query_size(); --sz >= 0; ) {
		CONFIGD->write_file("save/" + (sz + 1) + ".obj",
			STRINGD->hybrid_sprint(
				objlist->query_element(sz)[1]
			) + "\n"
		);
	}
}

void load_world()
{
	int sz;
	int count;

	oindex2onum = new_object(BIGSTRUCT_MAP_LWO);
	oindex2onum->set_type(T_INT);

	objlist = new_object(BIGSTRUCT_ARRAY_LWO);

	for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
		object obj;

		if (obj = find_object("~Game/obj/thing#" + sz)) {
			destruct_object(obj);
		}
	}

	sz = 1;

	for (;;) {
		if (CONFIGD->file_info("save/" + sz + ".obj")) {
			object obj;
			int i;
			obj = clone_object("~Game/obj/thing");
			sscanf(object_name(obj), "%*s#%d", i);

			oindex2onum->set_element(i, sz);

			objlist->push_back( ({ obj,
				CONFIGD->read_file("save/" + sz + ".obj") }) );
			sz++;
		} else {
			break;
		}
	}

	count = sz;

	for (sz = 1; sz < count; sz++) {
		object obj;
		mixed *arr;
		mixed data;

		({ obj, data }) = objlist->query_element(sz - 1);

		data = PARSER_VALUE->parse(data);

		obj->move(data["environment"]);
		data["environment"] = nil;

		obj->set_object_name(data["name"]);
		data["name"] = nil;

		objlist->set_element(sz - 1, ({ obj, data }) );
	}

	for (sz = 1; sz < count; sz++) {
		object obj;
		mixed data;

		({ obj, data }) = objlist->query_element(sz - 1);

		obj->load(data);
	}
}
