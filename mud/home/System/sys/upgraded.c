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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/version.h>
#include <status.h>

inherit SECOND_AUTO;

static void create()
{
	call_out("self_destruct", 0);
}

void upgrade_system()
{
	string *safe_versions;
	string *users;
	int sz;

	ACCESS_CHECK(SYSTEM());

	safe_versions = explode(read_file("~/data/safe_upgrade_versions"), "\n");

	for (sz = sizeof(safe_versions) - 1; sz >= 0; --sz) {
		if (safe_versions[sz] == KOTAKA_VERSION) {
			break;
		}
	}

	if (sz == -1) {
		error("Cannot safely upgrade from version " + KOTAKA_VERSION);
	}

	SUSPENDD->suspend_system();
	SUSPENDD->queue_work("upgrade_system_2");
}

void upgrade_system_2()
{
	ACCESS_CHECK(SYSTEM());

	destruct_object(LIB_INITD);
	compile_object(PROGRAM_INFO);
	compile_object(PROGRAMD);
	compile_object(OBJECTD);

	SUSPENDD->queue_work("upgrade_system_3");
}

void upgrade_system_3()
{
	ACCESS_CHECK(SYSTEM());

	MODULED->upgrade_modules();
}

static void self_destruct()
{
	destruct_object(this_object());
}
