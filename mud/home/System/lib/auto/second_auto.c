/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/privilege.h>

#include <trace.h>
#include <type.h>
#include <status.h>

inherit "call_guard";
inherit "callout_guard";
inherit "catalog";
inherit "object";
inherit "object_guard";
inherit "touch";
inherit "list";

/**********/
/* status */
/**********/

nomask void _F_dummy()
{
}

nomask mixed _F_status(mixed args ...)
{
	ACCESS_CHECK(SYSTEM());

	return status(this_object(), args ...);
}

#if 1
nomask void save_object(string filename)
{
	error("save_object is disabled");
}

nomask void restore_object(string filename)
{
	error("restore_object is disabled");
}
#endif
