/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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

#include <game/paths.h>

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

object root;
int save_pending;
int wsave_pending;

/****************/
/* Declarations */
/****************/

string bits(string input);
static void do_test();
void build_world();

/****************/
/* Constructors */
/****************/

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

	PROXYD->get_proxy("Game")->set_global_access("~", 1);
	PROPERTYD->add_property("facing", T_INT, PROP_SIMPLE);
	PROPERTYD->add_property("holding", T_MAPPING, PROP_SIMPLE);
	SECRETD->make_dir(".");
	SECRETD->make_dir("log");

	"sys/helpd"->load_help();

	root = find_object("sys/root");

	build_world();

	"sys/testd"->test();

}

void reboot()
{
}

private void place_object(string base)
{
	object obj;
	float rnd1;
	float rnd2;

	rnd1 = SUBD->rnd() * SUBD->pi() * 2.0;
	rnd2 = SUBD->rnd() * 30.0;

	obj = clone_object("~/obj/object");
	obj->set_id_base(base);
	obj->move(root);
	obj->set_x_position(sin(rnd1) * rnd2);
	obj->set_y_position(cos(rnd1) * rnd2);
}

void build_world()
{
	int i;
	/* 50 soil spots */
	/* 10 rocks */
	/* 3 deer */
	/* 1 wolf */

	for (i = 0; i < 250; i++) {
		place_object("soil");
	}

	for (i = 0; i < 10; i++) {
		place_object("rock");
	}

	for (i = 0; i < 3; i++) {
		place_object("deer");
	}

	for (i = 0; i < 1; i++) {
		place_object("wolf");
	}
}

/****************/
/* System hooks */
/****************/

string query_destructor(string path)
{
	switch(path) {
	case USR_DIR + "/Game/lib/object":
		return "game_object_destruct";
	}
}
