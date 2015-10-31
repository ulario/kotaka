/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2005, 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
	MODULED->boot_module("Http");
	MODULED->boot_module("Text");
	MODULED->boot_module("Intermud");
	MODULED->boot_module("Thing");

	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("open", 1);
	load_dir("sys", 1);
}

private void create_channels()
{
	if (!CHANNELD->test_channel("chat")) {
		CHANNELD->add_channel("chat");
	}
}

void configure_channels()
{
}

private void set_limits()
{
	KERNELD->rsrc_set_limit("Game", "ticks", 1000000);
}

static void create()
{
	load();

	KERNELD->set_global_access("Game", 1);

	set_limits();

	create_channels();
	configure_channels();

	if (CONFIGD->file_info("save")) {
		"sys/saveload"->load_world();
	} else {
		build_world();
	}
}

object create_thing()
{
	return clone_object("~/obj/thing");
}

object create_object()
{
	object thing;

	LOGD->post_message("debug", LOG_WARN, "Please update " + previous_program() + " to use GAME_INITD->create_thing()");

	thing = clone_object("~/obj/thing");
	thing->set_local_property("local_snouns", ({ "thing" }) );
	return thing;
}

void destroy_object(object obj)
{
	destruct_object(obj);
}

void build_world()
{
	object world;
	object master;
	object trinket;

	world = create_thing();
	world->set_property("id", "world");
	world->set_object_name("world");
	world->set_mass(5.972e+24);
	world->set_density(6.5);
	world->set_capacity(1e+12);
	world->set_max_mass(5.972e+24);
	world->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/fill/grass");
	world->set_property("local_snouns", ({ "world" }) );

	master = create_thing();
	master->set_property("id", "human");
	master->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/human");
	master->set_local_property("local_snouns", ({ "human", "man" }) );
	master->set_object_name("class:race:humanoid:human");
	master->set_local_property("brief", "simple man");
	master->set_local_property("look", "A simple, boring man.");
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	set_limits();

	load();

	purge_orphans("Game");
}

void booted_module(string module)
{
	ACCESS_CHECK(previous_program() == MODULED);

	if (module == "Help") {
		"sys/helpd"->load_help();
	}
}
