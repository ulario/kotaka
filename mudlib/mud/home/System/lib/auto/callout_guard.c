/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <status.h>

static int free_callouts()
{
	return status(ST_COTABSIZE) -
		(status(ST_NCOSHORT) + status(ST_NCOLONG));
}

#if 1
static int call_out(string func, mixed delay, mixed args...)
{
	if (!this_object()) {
		error("Cannot call_out from destructed object");
	}

	if (!function_object(func, this_object())) {
		error("Call_out to undefined function " + func);
	}

	if (!SYSTEM() && free_callouts() < 20) {
		error("Too many callouts");
	}

	return ::call_out(func, delay, args...);
}
#endif
