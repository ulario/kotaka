/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2005, 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/help.h>
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/thing.h>
#include <kotaka/privilege.h>
#include <kotaka/property.h>
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
void build_world();

/****************/
/* Constructors */
/****************/

private void load()
{
	MODULED->boot_module("Account");
	MODULED->boot_module("Action");
	MODULED->boot_module("Channel");
	MODULED->boot_module("Geometry");
	MODULED->boot_module("Help");
	MODULED->boot_module("Text");
	MODULED->boot_module("Thing");

	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("open", 1);
	load_dir("sys", 1);
}

private void create_channels()
{
	CHANNELD->add_channel("chat");
}

void configure_channels()
{
}

void configure_properties()
{
	PROPERTYD->clear_properties();

	/* identification */
	PROPERTYD->add_property("id", T_STRING, PROP_MAGIC, ({ "query_id", "set_id" }) );
	PROPERTYD->add_property("name", T_STRING, PROP_MAGIC, ({ "query_object_name", "set_object_name" }) );

	/* inheritance */
	PROPERTYD->add_property("archetypes", T_ARRAY, PROP_MAGIC, ({ "query_archetypes", "set_archetypes" }) );
	PROPERTYD->add_property("first_instance", T_OBJECT, PROP_MAGIC, ({ "query_first_instance", nil }) );
	PROPERTYD->add_property("prev_instances", T_MAPPING, PROP_MAGIC, ({ "query_next_instances", nil }) );
	PROPERTYD->add_property("next_instances", T_MAPPING, PROP_MAGIC, ({ "query_prev_instances", nil }) );

	/* descriptions */
	PROPERTYD->add_property("snouns", T_ARRAY, PROP_COMBO, ({ "local_snouns", "removed_snouns" }) );
	PROPERTYD->add_property("pnouns", T_ARRAY, PROP_COMBO, ({ "local_pnouns", "removed_pnouns" }) );
	PROPERTYD->add_property("adjectives", T_ARRAY, PROP_COMBO, ({ "local_adjectives", "removed_adjectives" }) );

	PROPERTYD->add_property("local_snouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("local_pnouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("local_adjectives", T_ARRAY, PROP_SIMPLE, ({ }));

	PROPERTYD->add_property("removed_snouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("removed_pnouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("removed_adjectives", T_ARRAY, PROP_SIMPLE, ({ }));

	PROPERTYD->add_property("brief", T_STRING, PROP_INHERIT, "nondescript object");
	PROPERTYD->add_property("look", T_STRING, PROP_INHERIT, "A nondescript object.");
	PROPERTYD->add_property("examine", T_STRING, PROP_INHERIT, "A nondescript object with no detail.");

	PROPERTYD->add_property("pronoun", T_STRING, PROP_INHERIT);

	PROPERTYD->add_property("is_definite", T_INT, PROP_INHERIT, 0);
	PROPERTYD->add_property("is_transparent", T_INT, PROP_INHERIT, 0);

	/* inventory */
	PROPERTYD->add_property("environment", T_OBJECT, PROP_MAGIC, ({ "query_environment", "move" }) );
	PROPERTYD->add_property("inventory", T_ARRAY, PROP_MAGIC, ({ "query_inventory", nil }) );
	PROPERTYD->add_property("is_immobile", T_INT, PROP_INHERIT, 0);

	/* bulk */
	PROPERTYD->add_property("mass", T_FLOAT, PROP_MAGIC, ({ "query_mass", "set_mass" }) );
	PROPERTYD->add_property("total_mass", T_FLOAT, PROP_MAGIC, ({ "query_total_mass", nil }) );
	PROPERTYD->add_property("density", T_FLOAT, PROP_MAGIC, ({ "query_density", "set_density" }) );

	PROPERTYD->add_property("volume", T_FLOAT, PROP_MAGIC, ({ "query_volume", nil }) );
	PROPERTYD->add_property("total_volume", T_FLOAT, PROP_MAGIC, ({ "query_total_volume", nil }) );

	PROPERTYD->add_property("capacity", T_FLOAT, PROP_MAGIC, ({ "query_capacity", "set_capacity" }) );
	PROPERTYD->add_property("max_mass", T_FLOAT, PROP_MAGIC, ({ "query_max_mass", "set_max_mass" }) );
	PROPERTYD->add_property("is_flexible", T_INT, PROP_MAGIC, ({ "query_flexible", "set_flexible" }) );
	PROPERTYD->add_property("is_virtual", T_INT, PROP_MAGIC, ({ "query_virtual", "set_virtual" }) );

	/* economics */
	PROPERTYD->add_property("value", T_INT, PROP_INHERIT, 0);
	PROPERTYD->add_property("currency", T_STRING, PROP_INHERIT);

	/* geometry */
	PROPERTYD->add_property("pos_x", T_INT, PROP_MAGIC, ({ "query_x_position", "set_x_position" }) );
	PROPERTYD->add_property("pos_y", T_INT, PROP_MAGIC, ({ "query_y_position", "set_y_position" }) );
	PROPERTYD->add_property("pos_z", T_INT, PROP_MAGIC, ({ "query_z_position", "set_z_position" }) );

	PROPERTYD->add_property("size_x", T_INT, PROP_MAGIC, ({ "query_x_size", "set_x_size" }) );
	PROPERTYD->add_property("size_y", T_INT, PROP_MAGIC, ({ "query_y_size", "set_y_size" }) );
	PROPERTYD->add_property("size_z", T_INT, PROP_MAGIC, ({ "query_z_size", "set_z_size" }) );

	/* catch all */
	PROPERTYD->add_property("data", T_MAPPING, PROP_SIMPLE, ([ ]) );

	/* body/mobile handling */
	PROPERTYD->add_property("mobiles", T_ARRAY, PROP_SIMPLE, ({ }) );

	/* exits */
	PROPERTYD->add_property("exit_return", T_OBJECT, PROP_MAGIC, ({ "query_exit_return", "set_exit_return" }));
	PROPERTYD->add_property("exit_destination", T_OBJECT, PROP_MAGIC, ({ "query_exit_destination", "set_exit_destination" }));
	PROPERTYD->add_property("exit_direction", T_STRING, PROP_SIMPLE);
	PROPERTYD->add_property("default_exit", T_OBJECT, PROP_INHERIT);
	PROPERTYD->add_property("default_entrance", T_OBJECT, PROP_INHERIT);

	/* text painting */
	PROPERTYD->add_property("event:paint", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("paint_color", T_INT, PROP_INHERIT);
	PROPERTYD->add_property("paint_character", T_STRING, PROP_INHERIT);

	/* clothing */
	PROPERTYD->add_property("is_clothing", T_INT, PROP_INHERIT);
	PROPERTYD->add_property("fills_slot", T_INT, PROP_INHERIT);
	PROPERTYD->add_property("parts_covered", T_ARRAY, PROP_INHERIT);

	PROPERTYD->add_property("is_worn", T_INT, PROP_SIMPLE);
	PROPERTYD->add_property("clothing_layer", T_INT, PROP_SIMPLE);

	/* head, neck, chest */
	/* left arm, left wrist, left hand */
	/* right arm, right wrist, right hand */
	/* belly, hips, groin */
	/* left leg, left ankle, left foot */
	/* right leg, right ankle, right foot */
}

static void create()
{
	load();

	configure_properties();
	create_channels();
	configure_channels();

	KERNELD->set_global_access("Game", 1);

	load_help();
	build_world();
}

object create_object()
{
	return clone_object("~/obj/thing");
}

void destroy_object(object obj)
{
	destruct_object(obj);
}

void build_world()
{
	object master;
	object trinket;

	world = create_object();
	world->set_property("id", "world");
	world->set_object_name("world");

	master = create_object();
	master->set_property("id", "human");
	master->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/human");
	master->set_local_property("nouns", ({ "human", "man" }) );
	master->set_object_name("class:race:humanoid:human");

	world->set_capacity(1000000.0);
	world->set_mass(5.972e+24);
	world->set_density(6.5);
	world->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/fill/grass");
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

		category = cqueue->query_front();
		cqueue->pop_front();

		load_helpdir(category, cqueue, tqueue);

		call_out("load_tick", 0, cqueue, tqueue);
	} else if (!tqueue->empty()) {
		string dir;
		string entry;

		({ dir, entry }) = tqueue->query_front();
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

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	load();

	purge_orphans("Game");
}

void booted_module(string module)
{
	ACCESS_CHECK(previous_program() == MODULED);

	if (module == "Help") {
		load_help();
	}
}
