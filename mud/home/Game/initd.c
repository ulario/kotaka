/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/thing.h>
#include <kotaka/paths/verb.h>
#include <kotaka/privilege.h>
#include <kotaka/property.h>
#include <type.h>

#define M 1000000
#define G 1000000000

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	MODULED->boot_module("Account");
	MODULED->boot_module("Action");
	MODULED->boot_module("Channel");
	MODULED->boot_module("Geometry");
	MODULED->boot_module("Http");
	MODULED->boot_module("Intermud");
	MODULED->boot_module("Kotaka");
	MODULED->boot_module("Text");
	MODULED->boot_module("Thing");

	load_dir("lwo");
	load_dir("obj");
	load_dir("sys");
}

private void set_limits()
{
	reset_limits();

	KERNELD->rsrc_set_limit("Game", "ticks", 10 * M);
}

private void create_channels()
{
	if (!CHANNELD->test_channel("chat")) {
		CHANNELD->add_channel("chat");
	}
}

void build_world();

static void create()
{
	KERNELD->set_global_access("Game", 1);

	set_limits();

	load();

	create_channels();

	if (VARD->file_info("save")) {
		"sys/saveload"->load_world();
	} else {
		build_world();
	}
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	set_limits();
}

void booted_module(string module)
{
	ACCESS_CHECK(previous_program() == MODULED);

	if (module == "Kotaka") {
		"sys/helpd"->load_help();
	}
}

object create_thing()
{
	return clone_object("~/obj/thing");
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
	world->set_object_name("planets:aerth");
	world->set_id("aerth");
	world->set_mass(5.972e+24);
	world->set_density(6.5);
	world->set_capacity(1e+12);
	world->set_max_mass(5.972e+24);
	world->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/fill/grass");

	world->add_local_detail(nil);
	world->set_local_snouns(nil, ({ "planet", "aerth" }) );

	master = create_thing();
	master->set_object_name("class:race:humanoid:human");
	master->set_id("human");
	master->set_property("event:paint",
		USR_DIR + "/Game/sys/handler/paint/human");

	master->add_local_detail(nil);
	master->set_local_snouns(nil, ({ "human", "man" }) );
	master->set_local_description(nil, "brief", "simple man");
	master->set_local_description(nil, "look", "A simple, boring man.");
}

string query_patcher(string path)
{
	switch(path) {
	case USR_DIR + "/Game/lib/thing/detail":
		return "patch_detail";
	}
}
