/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <status.h>

inherit SECOND_AUTO;

object deque;
string tail;

static void create(int clone)
{
}

void push(string text)
{
	if (!tail) {
		tail = text;
		return;
	}

	if (strlen(tail) + strlen(text) > status(ST_STRSIZE)) {
		if (!deque) {
			deque = new_object(BIGSTRUCT_DEQUE_LWO);
		}

		deque->push_back(tail);
		tail = text;
	}
}

string pop()
{
	string temp;

	if (deque) {
		if (deque->empty()) {
			deque = nil;
		} else {
			string head;

			head = deque->query_front();
			deque->pop_front();

			return head;
		}
	}

	temp = tail;
	tail = nil;
	return temp;
}
