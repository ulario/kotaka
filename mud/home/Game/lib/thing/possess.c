/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020  Raymond Jennings
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

private object possessor;
private object possessee;

void set_possessor(object new_possessor)
{
	ACCESS_CHECK(GAME());

	possessor = new_possessor;
}

object query_possessor()
{
	return possessor;
}

object query_possessee()
{
	return possessee;
}

atomic void possess(object new_possessee)
{
	if (possessee) {
		possessee->set_possessor(nil);
	}

	if (new_possessee) {
		object puncle;

		if (puncle = new_possessee->query_possessor()) {
			puncle->possess(nil);
		}
	}

	possessee = new_possessee;

	if (possessee) {
		possessee->set_possessor(this_object());
	}
}
