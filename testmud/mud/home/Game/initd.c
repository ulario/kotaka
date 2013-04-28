/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <config.h>

#include <kernel/kernel.h>
#include <kernel/access.h>

#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/property.h>
#include <kotaka/bigstruct.h>

#include <game/paths.h>
#include <thing/paths.h>

#include <catalog/paths.h>

#include <status.h>
#include <type.h>

/**************/
/* Directives */
/**************/

inherit LIB_INITD;
inherit UTILITY_COMPILE;

/*************/
/* Variables */
/*************/

object world;

int save_pending;
int wsave_pending;

/****************/
/* Declarations */
/****************/

void load_help();
static void do_test();
void build_world();

/****************/
/* Constructors */
/****************/

void configure_properties()
{
	PROPERTYD->clear_properties();

	PROPERTYD->add_property("id", T_STRING, PROP_SIMPLE);
	PROPERTYD->add_property("nouns", T_ARRAY, PROP_INHERIT);

	PROPERTYD->add_property("brief", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("look", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("examine", T_STRING, PROP_INHERIT);

	PROPERTYD->add_property("environment", T_OBJECT, PROP_MAGIC, ({ "query_environment", "move" }) );
	PROPERTYD->add_property("inventory", T_ARRAY, PROP_MAGIC, ({ "query_inventory", nil }) );

	PROPERTYD->add_property("mass", T_FLOAT, PROP_MAGIC, ({ "query_mass", "set_mass" }) );
	PROPERTYD->add_property("density", T_FLOAT, PROP_MAGIC, ({ "query_density", "set_density" }) );

	PROPERTYD->add_property("archetypes", T_ARRAY, PROP_MAGIC, ({ "query_archetypes", "set_archetypes" }) );
	PROPERTYD->add_property("name", T_STRING, PROP_MAGIC, ({ "query_object_name", "set_object_name" }) );

	PROPERTYD->add_property("adjectives", T_ARRAY, PROP_INHERIT);
	PROPERTYD->add_property("event:create", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("event:destroy", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("event:paint", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("event:timer", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("data", T_MAPPING, PROP_SIMPLE);
}

static void create()
{
	object proxy;
	string world;
	string dump;
	int index;
	object clone;
	object shape;
	string *bad_bins;
	string *bins;
	string test;

	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("open", 1);
	load_dir("sys", 1);

	configure_properties();

	KERNELD->set_global_access("Game", 1);

	"sys/testd"->test();

	INITD->boot_subsystem("Test");
	INITD->boot_subsystem("Help");
	INITD->boot_subsystem("Text");
	INITD->boot_subsystem("Account");
	INITD->boot_subsystem("Thing");
	INITD->boot_subsystem("Catalog");

	load_help();
	build_world();
}

void reboot()
{
}

int t1;
int t2;
int t3;

static void place_object(string base, int remainder)
{
	object obj;

	float rnd1;
	float rnd2;

	rnd1 = SUBD->rnd() * SUBD->pi() * 2.0;
	rnd2 = SUBD->rnd() * 30.0;

	obj = clone_object(OBJ_THING);
	obj->set_property("id", base);

	switch(base) {
	case "deer":
		obj->set_mass(50.0 + SUBD->rnd() * 7.5);
		obj->set_density(1.1);
		break;
	case "wolf":
		obj->set_mass(35.0 + SUBD->rnd() * 5.0);
		obj->set_density(0.9);
		break;
	case "soil":
		obj->set_mass(10.0 + SUBD->rnd() * 2.0);
		obj->set_density(1.5);
		break;
	case "rock":
		obj->set_mass(1.0 + SUBD->rnd() * 0.2);
		obj->set_density(6.3);
		break;
	}

	obj->set_property("event:paint",
		find_object("~/sys/handler/paint/room")
	);

	obj->move(world);

	obj->set_x_position(sin(rnd1) * rnd2);
	obj->set_y_position(cos(rnd1) * rnd2);

	if (remainder) {
		call_out("place_object", 0, base, remainder - 1);
	} else {
		LOGD->post_message("world", LOG_NOTICE, "Finished placing " + base);
	}
}

void build_world()
{
	object master;

	world = clone_object(OBJ_THING);
	world->set_property("id", "world");
	world->set_object_name("world");

	master = clone_object(OBJ_THING);
	master->set_property("id", "shack");
	master->set_property("event:create",
		USR_DIR + "/Game/sys/handler/create/shack");
	master->set_property("event:destroy",
		USR_DIR + "/Game/sys/handler/destroy/shack");
	master->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/shack");
	master->set_property("nouns", ({ "shack" }) );
	master->set_property("adjectives", ({ "wooden" }) );
	master->set_object_name("buildings:master:shack");

	master = clone_object(OBJ_THING);
	master->set_property("id", "human");
	master->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/human");
	master->set_property("nouns", ({ "human", "man" }) );
	master->set_object_name("human");

	master = clone_object(OBJ_THING);
	master->set_property("id", "tree");
	master->set_property("event:create",
		USR_DIR + "/Game/sys/handler/create/tree");
	master->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/tree");
	master->set_property("event:timer",
		USR_DIR + "/Game/sys/handler/timer/tree");
	master->set_property("nouns", ({ "tree" }) );
	master->set_object_name("tree");

	master = clone_object(OBJ_THING);
	master->set_property("id", "door");
	master->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/door");
	master->set_property("nouns", ({ "door" }) );
	master->set_property("adjectives", ({ "wooden" }) );
	master->set_object_name("buildings:parts:door");

	world->set_capacity(1000000.0);
	world->set_mass(1e+9);
	world->set_density(6.5);
	world->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/grass");
}

object create_object()
{
	return clone_object(OBJ_THING);
}

void destroy_object(object obj)
{
	destruct_object(obj);
}

/********/
/* Help */
/********/

static void load_helpfile(string dir, string entry)
{
	dir = (dir ? dir + "/" : "");

	HELPD->add_topic(dir + entry,
		read_file("~/data/help/" + dir + entry + ".hlp")
	);
}

static void load_helpdir(string dir, object cqueue, object tqueue)
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	HELPD->add_category(dir);

	dirlist = get_dir("~/data/help/" + dir + "/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			cqueue->push_back(dir + "/" + entry);
		} else if (sscanf(entry, "%s.hlp", entry)) {
			tqueue->push_back( ({ dir, entry }) );
		}
	}
}

static void load_tick(object cqueue, object tqueue)
{
	if (!cqueue->empty()) {
		string category;

		category = cqueue->get_front();
		cqueue->pop_front();

		load_helpdir(category, cqueue, tqueue);

		call_out("load_tick", 0, cqueue, tqueue);
	} else if (!tqueue->empty()) {
		string dir;
		string entry;

		({ dir, entry }) = tqueue->get_front();
		tqueue->pop_front();

		load_helpfile(dir, entry);

		call_out("load_tick", 0, cqueue, tqueue);
	} else {
		LOGD->post_message("help", LOG_NOTICE, "Help information loaded.");
	}
}

static void load_rootdir()
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	object cqueue;
	object tqueue;

	cqueue = new_object(BIGSTRUCT_DEQUE_LWO);
	tqueue = new_object(BIGSTRUCT_DEQUE_LWO);

	dirlist = get_dir("~/data/help/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			cqueue->push_back(entry);
		} else if (sscanf(entry, "%s.hlp", entry)) {
			tqueue->push_back( ({ nil, entry }) );
		}
	}

	call_out("load_tick", 0, cqueue, tqueue);
}

object query_world()
{
	return world;
}

void load_help()
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	HELPD->reset();
	load_rootdir();
}
