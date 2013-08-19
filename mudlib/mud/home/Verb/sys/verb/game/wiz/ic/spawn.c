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
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <game/paths.h>

inherit LIB_EMIT;
inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object master;
	object thing;

	if (query_user()->query_class() < 2) {
		send_out("You must be a wizard to use this command.\n");
		return;
	}

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	master = CATALOGD->lookup_object(args);

	if (!master) {
		send_out("No such object: " + args + ".\n");
		return;
	}

	thing = GAME_INITD->create_object();
	thing->add_archetype(master);
	thing->move(actor);

	emit_from(actor, ({ "spawn", "spawns" }), thing);
}
