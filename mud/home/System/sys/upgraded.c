/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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

void upgrade_system(varargs int stage)
{
	ACCESS_CHECK(SYSTEM());

	switch(stage) {
	case 0:
		compile_object(INITD);
		SUSPENDD->queue_work("upgrade_system", 1);
		break;
	case 1:
		INITD->upgrade_system_upgraded_hook();
	}
}

static void self_destruct()
{
	destruct_object(this_object());
}
