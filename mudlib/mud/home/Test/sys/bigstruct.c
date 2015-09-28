/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

private void test_bigstruct_array()
{
	object arr;
	object arrslice;
	int i, j;
	i = 1;

	arr = new_object(BIGSTRUCT_ARRAY_LWO);
	arr->set_size(0x40000000);

	for (i = 1; i < 0x40000000; i <<= 1) {
		arr->set_element(i - 1, i - 1);
		arr->set_element(i, i);
		ASSERT(arr->query_element(i - 1) == i - 1);
		ASSERT(arr->query_element(i) == i);
	}

	for (j = 0x40000000; j > 0; j /= 3, j <<= 1) {
		arr->set_size(j);
		arr->set_size(0x40000000);

		for (i = 1; i < 0x40000000; i <<= 1) {
			if (i < j) {
				ASSERT(arr->query_element(i) == i);
			} else {
				ASSERT(arr->query_element(i) == nil);
			}
		}
		for (i = 1; i < 0x40000000; i <<= 1) {
			if ((i - 1) < j) {
				ASSERT(arr->query_element(i - 1) == i - 1);
			} else {
				ASSERT(arr->query_element(i - 1) == nil);
			}
		}
	}

	arr->set_element(49, 1337);
	arr->set_element(50, 2337);
	arr->set_element(150, 3337);
	arr->set_element(151, 4337);

	arrslice = arr->slice(50, 150);
	ASSERT(arrslice->query_size() == 101);
	ASSERT(arrslice->query_element(0) == 2337);
	ASSERT(arrslice->query_element(100) == 3337);

	arr->clear();

	/* leak test */
	arr->set_size(0);

	i = status(ST_NOBJECTS);
	arr->set_size(0x7FFFFFFF);
	arr->set_element(49, 1337);
	arr->set_element(50, 2337);
	arr->set_element(150, 3337);
	arr->set_element(151, 4337);
	arr->set_element(1500, 3337);
	arr->set_element(1510, 4337);
	arr->set_element(15000, 3337);
	arr->set_element(15100, 4337);
	arr->set_size(0);

	ASSERT(status(ST_NOBJECTS) == i);
}

private void test_bigstruct_deque()
{
	int x, y;
	object deque;

	deque = new_object(BIGSTRUCT_DEQUE_LWO);

	for (x = 0; x < 1500; x++) {
		deque->push_back(x);
		deque->pop_front();
		ASSERT(deque->empty());
	}

	for (x = 0; x < 1500; x++) {
		deque->push_front(x);
		deque->pop_back();
		ASSERT(deque->empty());
	}

	for (x = 0; x < 1500; x++) {
		deque->push_back(x);
		ASSERT(!deque->empty());
	}

	ASSERT(deque->query_size() == 1500);

	for (x = 0; x < 1500; x++) {
		ASSERT(x == deque->query_front());
		deque->pop_front();
	}

	ASSERT(deque->query_size() == 0);
	ASSERT(deque->empty());

	for (x = 0; x < 1500; x++) {
		deque->push_front(x);
		ASSERT(!deque->empty());
	}

	ASSERT(deque->query_size() == 1500);

	for (x = 0; x < 1500; x++) {
		ASSERT(x == deque->query_back());
		deque->pop_back();
	}

	ASSERT(deque->query_size() == 0);
	ASSERT(deque->empty());

	for (x = 0; x < 1500; x++) {
		deque->push_front(x);
		ASSERT(!deque->empty());
	}

	ASSERT(deque->query_size() == 1500);

	for (x = 1499; x >= 0; x--) {
		ASSERT(x == deque->query_front());
		deque->pop_front();
	}

	ASSERT(deque->query_size() == 0);
	ASSERT(deque->empty());

	for (x = 0; x < 1500; x++) {
		deque->push_back(x);
		ASSERT(!deque->empty());
	}

	ASSERT(deque->query_size() == 1500);

	for (x = 1499; x >= 0; x--) {
		ASSERT(x == deque->query_back());
		deque->pop_back();
	}

	ASSERT(deque->empty());
	ASSERT(deque->query_size() == 0);
}

private void test_bigstruct_map()
{
	int i;
	object map;

	map = new_object(BIGSTRUCT_MAP_LWO);
	map->set_type(T_STRING);

	for (i = 0; i < 1000; i++) {
		string key;
		key = hash_string("crypt", "" + i);
		map->set_element(key, i);
		ASSERT(map->query_element(key) == i);
	}

	map->clear();
	map = new_object(BIGSTRUCT_MAP_LWO);
	map->set_type(T_INT);

	for (i = 0; i < 1000; i++) {
		map->set_element(i, i);
		ASSERT(map->query_element(i) == i);
	}

	map->clear();

	for (i = 0; i < 2500; i++) {
		map->set_element(i, 0);
	}

	for (i = 0; i < 50; i++) {
		map->set_element(i * i, i);
	}

	map->rebalance();

	for (i = 0; i < 50; i++) {
		ASSERT(map->query_element(i * i) == i);
	}

	map->reindex();

	for (i = 0; i < 50; i++) {
		ASSERT(map->query_element(i * i) == i);
	}

	map->clear();
}

void test()
{
	ACCESS_CHECK(TEST());

	LOGD->post_message("debug", LOG_DEBUG, "Starting array test...");
	test_bigstruct_array();

	LOGD->post_message("debug", LOG_DEBUG, "Starting deque test...");
	test_bigstruct_deque();

	LOGD->post_message("debug", LOG_DEBUG, "Starting map test...");
	test_bigstruct_map();
}
