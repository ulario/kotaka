/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	INITD->boot_subsystem("Account");
	INITD->boot_subsystem("Algorithm");
	INITD->boot_subsystem("Ansi");
	INITD->boot_subsystem("Bigstruct");
	INITD->boot_subsystem("Channel");
	INITD->boot_subsystem("Geometry");
	INITD->boot_subsystem("Help");
	INITD->boot_subsystem("Http");
	INITD->boot_subsystem("String");
	INITD->boot_subsystem("Text");
	INITD->boot_subsystem("Thing");
	INITD->boot_subsystem("Utility");
}

static void create()
{
	KERNELD->set_global_access("Kotaka", 1);

	load();
}

void upgrade_subsystem()
{
	ACCESS_CHECK(previous_program() == INITD);

	load();

	purge_orphans("Kotaka");
}
