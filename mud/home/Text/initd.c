/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kernel/access.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	MODULED->boot_module("Ansi");
	MODULED->boot_module("Verb");

	load_dir("lwo");
	load_dir("obj");
	load_dir("sys");
}

private void set_limits()
{
	reset_limits();

	KERNELD->rsrc_set_limit("Text", "ticks", 5000000);
}

static void create()
{
	set_limits();

	load();
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	set_limits();
}

/* to be called in 0.60 prior to upgrade to 0.61 */
void upgrade_check()
{
	ACCESS_CHECK(previous_program() == MODULED);

	if ("sys/logd"->busy()) {
		error("Text LogD not flushed yet");
	}
}

void prepare_reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	"sys/aliasd"->save();
}

void reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	"sys/aliasd"->restore();
}

void hotboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	"sys/aliasd"->restore();
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	set_limits();

	destruct_dir("lib");
	compile_dir("lwo");
	compile_dir("obj");
	compile_dir("sys");

	call_out("upgrade_module_post_recompile", 0);
}

static void upgrade_module_post_recompile()
{
	"sys/logd"->flush();
}

private void booted_channel()
{
	object *users;
	int sz;

	users = TEXT_USERD->query_users() + TEXT_USERD->query_guests();

	for (sz = sizeof(users); --sz >= 0; ) {
		users[sz]->subscribe_channels();
	}
}

void booted_module(string module)
{
	switch(module) {
	case "Channel":
		booted_channel();
	}
}

static void destruct()
{
	destruct_object("sys/userd");
}

string query_patcher(string path)
{
	switch(path) {
	case USR_DIR + "/obj/user":
		return "set_ip";
	}
}
