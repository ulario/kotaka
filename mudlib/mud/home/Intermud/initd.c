/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	load_dir("sys", 1);
}

static void reboot_self()
{
	INITD->reboot_subsystem("Intermud");
}

static void create()
{
	KERNELD->set_global_access("Intermud", 1);

	load();

	if (!CHANNELD->test_channel("dgd")) {
		CHANNELD->add_channel("dgd");
		CHANNELD->set_intermud("dgd", 1);
	}

	CHANNELD->set_channel_config("dgd", "channel_color", 0x0c);

	call_out("reboot_self", 300);
}

void upgrade_subsystem()
{
	ACCESS_CHECK(previous_program() == INITD);

	load();

	purge_orphans("Intermud");
}
