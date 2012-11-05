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
#include <game/paths.h>
#include <text/paths.h>
#include <status.h>
#include <kotaka/bigstruct.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object world;
	object list;

	int ticks, ticks2;
	int i, sz;

	list = TEXT_SUBD->mega_inventory(world = GAME_INITD->query_world());

	sz = list->get_size();

	BULKD->reset();

	for (i = 0; i < sz; i++) {
		list->get_element(i)->bulk_invalidate(1);
	}
	for (i = 0; i < sz; i++) {
		list->get_element(i)->bulk_queue(1);
	}
	for (i = 1; i < sz; i++) {
		list->get_element(i)->move(world);
	}
}
