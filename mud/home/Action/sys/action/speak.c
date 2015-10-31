/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/action.h>
#include <kotaka/paths/text.h>

inherit LIB_USERIO;
inherit LIB_EMIT;
inherit LIB_ACTION;

int query_readiness_cost(mapping roles)
{
	return strlen(roles["evoke"]) * 20;
}

void action(mapping roles)
{
	object target;
	object actor;
	string text;

	actor = roles["actor"];
	target = roles["iob"];
	text = roles["evoke"];

	if (target) {
		emit_from(actor, actor, " ", ({ "say", "says" }), "to", target, ", \"" + text + "\".");
	} else {
		emit_from(actor, actor, " ", ({ "say", "says" }), ", \"" + text + "\".");
	}
}
