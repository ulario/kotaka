/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object env;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	if (args != "") {
		send_out("Just type \"leave\".\n");
		return;
	}

	env = actor->query_environment();

	if (!env) {
		send_out("You're already in the void, how much more alone do you want to be?\n");
		return;
	}

	generic_emit(actor, ({ "leave", "leaves" }), env, nil);
	actor->move(env->query_environment());
}
