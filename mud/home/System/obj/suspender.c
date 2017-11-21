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
#include <limits.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

static void create(int clone)
{
	if (clone) {
		call_out("self_destruct", 0);
	}
}

static void self_destruct()
{
	destruct_object(this_object());
}

static void dispatch_call(object obj, string func, mixed *args)
{
	call_other(obj, func, args...);
}

void call(object obj, string func, mixed *args)
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	rlimits(INT_MAX / 2; -1) {
		call_limited("dispatch_call", obj, func, args);
	}
}