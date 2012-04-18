#include <kernel/tls.h>

#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

#include <kotaka/bigstruct.h>

#include <type.h>
#include <status.h>

inherit SECOND_AUTO;

private inherit tls API_TLS;

static void create()
{
	tls::create();
}

private void test_bigstruct_array()
{
	object arr;
	object arrslice;
	int i, j;
	i = 1;

	arr = new_object(BIGSTRUCT_ARRAY_LWO);
	arr->set_size(0x40000000);
	
	for (i = 1; i < 0x40000000; i <<= 1) {
		arr->set_element(i, i);
		ASSERT(arr->get_element(i) == i);
	}
	
	for (j = 0x40000000; j > 0; j /= 3, j <<= 1) {
		arr->set_size(j);
		arr->set_size(0x40000000);

		for (i = 1; i < 0x40000000; i <<= 1) {
			if (i < j) {
				ASSERT(arr->get_element(i) == i);
			} else {
				ASSERT(arr->get_element(i) == nil);
			}
		}
	}

	LOGD->post_message("test", LOG_DEBUG, "Testing array slicing");
	
	arr->set_element(49, 1337);
	arr->set_element(50, 2337);
	arr->set_element(150, 3337);
	arr->set_element(151, 4337);
	
	arrslice = arr->slice(50, 150);
	ASSERT(arrslice->get_size() == 101);
	ASSERT(arrslice->get_element(0) == 2337);
	ASSERT(arrslice->get_element(100) == 3337);
	
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
	
	LOGD->post_message("test", LOG_DEBUG, "Array test passed");
}

private void test_bigstruct_deque()
{
	int x, y;
	object deque;
	
	deque = new_object(BIGSTRUCT_DEQUE_LWO);

	for (x = 0; x < 3000; x++) {
		deque->push_back(x);
	}
	ASSERT(deque->get_size() == 3000);

	for (x = 0; x < 3000; x++) {
		ASSERT(x == deque->get_front());
		deque->pop_front();
	}
	
	ASSERT(deque->get_size() == 0);
	ASSERT(deque->empty());
	
	for (x = 0; x < 3000; x++) {
		deque->push_front(x);
	}

	ASSERT(deque->get_size() == 3000);

	for (x = 0; x < 3000; x++) {
		ASSERT(x == deque->get_back());
		deque->pop_back();
	}
	
	ASSERT(deque->get_size() == 0);
	ASSERT(deque->empty());
	
	for (x = 0; x < 3000; x++) {
		deque->push_front(x);
	}

	ASSERT(deque->get_size() == 3000);

	for (x = 2999; x >= 0; x--) {
		ASSERT(x == deque->get_front());
		deque->pop_front();
	}
	
	ASSERT(deque->get_size() == 0);
	ASSERT(deque->empty());
	
	for (x = 0; x < 3000; x++) {
		deque->push_back(x);
	}

	ASSERT(deque->get_size() == 3000);

	for (x = 2999; x >= 0; x--) {
		ASSERT(x == deque->get_back());
		deque->pop_back();
	}
	
	ASSERT(deque->empty());
	ASSERT(deque->get_size() == 0);
	
	LOGD->post_message("test", LOG_DEBUG, "Deque test passed");
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
		ASSERT(map->get_element(key) == i);
	}
	map->clear();
	map = new_object(BIGSTRUCT_MAP_LWO);
	map->set_type(T_INT);

	for (i = 0; i < 1000; i++) {
		map->set_element(i, i);
		ASSERT(map->get_element(i) == i);
	}

	map->clear();

	LOGD->post_message("test", LOG_DEBUG,
		"Testing defrag");
	
	for (i = 0; i < 2500; i++) {
		map->set_element(i, 0);
	}
	
	for (i = 0; i < 50; i++) {
		map->set_element(i * i, i);
	}

	LOGD->post_message("test", LOG_DEBUG,
		"Lightweight defrag complete");
	
	map->rebalance();

	for (i = 0; i < 50; i++) {
		ASSERT(map->get_element(i * i) == i);
	}

	LOGD->post_message("test", LOG_DEBUG,
		"Lightweight defrag test complete");

	map->reindex();

	LOGD->post_message("test", LOG_DEBUG,
		"Heavyweight defrag complete");

	for (i = 0; i < 50; i++) {
		ASSERT(map->get_element(i * i) == i);
	}

	LOGD->post_message("test", LOG_DEBUG,
		"Heavyweight defrag test complete");

	LOGD->flush();

	map->clear();
	LOGD->post_message("test", LOG_DEBUG, "Map test passed");
}

static void tls_test(varargs mixed args...)
{
	set_tlvar(0, "foobar");

	ASSERT(get_tlvar(0) == "foobar");

	LOGD->post_message("test", LOG_DEBUG, "TLS on callout test passed");
}

void test()
{
	string err;

	ACCESS_CHECK(SYSTEM());

#if 0
	LOGD->post_message("test", LOG_DEBUG, "TestD test battery beginning");

#ifdef SYS_NETWORKING
	LOGD->post_message("test", LOG_DEBUG, "There are " + sizeof(ports()) + " ports open.");
#endif

#	if 1
	LOGD->post_message("test", LOG_DEBUG, "Starting array test");
	rlimits(200; -1) {
		test_bigstruct_array();
	}
#	endif
#	if 1
	LOGD->post_message("test", LOG_DEBUG, "Starting deque test");
	rlimits(200; -1) {
		test_bigstruct_deque();
	}
#	endif
#	if 1
	LOGD->post_message("test", LOG_DEBUG, "Starting map test");
	rlimits(200; -1) {
		test_bigstruct_map();
	}
#	endif
	LOGD->post_message("test", LOG_DEBUG, "TestD test battery completed");
#endif
}
