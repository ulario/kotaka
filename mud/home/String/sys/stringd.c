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
#include <kotaka/paths/text.h>
#include <kotaka/paths/system.h>
#include <status.h>
#include <type.h>

inherit "~System/lib/string/escape";
inherit "~System/lib/string/sprint";
inherit "/lib/string/base";
inherit "/lib/string/bitops";
inherit "/lib/string/case";
inherit "/lib/string/char";
inherit "/lib/string/format";
inherit "/lib/string/int";
inherit "/lib/string/replace";

int regex_match(string in, string regex)
{
	regex = replace(regex, "/", "\\/");

	return !!parse_string("regstring = /" + regex + "/\n"
		+ "notreg = nomatch\n" + "full : regstring", in);
}
