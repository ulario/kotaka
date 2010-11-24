#include <kotaka/bigstruct.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>
#include <type.h>

object contents;
object index;

static void create()
{
	contents = clone_object(BIGSTRUCT_MAP_OBJ);
	index = clone_object(BIGSTRUCT_MAP_OBJ);
	
	contents->set_type(T_STRING);
	index->set_type(T_STRING);
	
	call_out("rebalance", 1800 + random(3600));
	call_out("reindex", 43200 + random(86400));
}

static void destruct()
{
	destruct_object(contents);
	destruct_object(index);
}

static void rebalance()
{
	call_out("rebalance", 1800 + random(3600));

	contents->rebalance();
	index->rebalance();
}

static void reindex()
{
	call_out("reindex", 43200 + random(86400));

	contents->reindex();
	index->reindex();
}

void toggle_sweep(string **toggles, int levels)
{
	int i;
	int j;
	
	for (i = 0; i < (1 << levels); i++) {
		for (j = 0; j < levels; j++) {
			if (i & (1 << j)) {
				toggles[i][j] = nil;
			}
			
		}

		toggles[i] -= ({ nil });
	}
}

string *indexes_of(string topic)
{
	string *parts;
	string *indices;
	string **toggles;
	
	int i, j;
	int sz;
	int tsz;
	
	parts = explode(topic, "/");
	sz = sizeof(parts);
	
	toggles = allocate((1 << sz) >> 1);
	tsz = sizeof(toggles);
	
	for (i = 0; i < tsz; i++) {
		toggles[i] = parts[0 .. sz - 2];
	}
	
	toggle_sweep(toggles, sz - 1);
	
	indices = allocate(tsz);
	
	for (i = 0; i < tsz; i++) {
		indices[i] = implode(toggles[i] + ({ parts[sz - 1] }), "/");
	}
	
	return indices;
}

void add_topic(string topic)
{
	mixed refcount;
 	string *indexes;
	int i;
	int sz;

	if (refcount = contents->get_element(topic)) {
		contents->set_element(topic, refcount + 1);
		return;
	} else {
		contents->set_element(topic, 1);
	}
	
	indexes = indexes_of(topic);
	sz = sizeof(indexes);
	
	for (i = 0; i < sz; i++) {
		mapping map;
		
		map = index->get_element(indexes[i]);
		
		if (!map) {
			index->set_element(indexes[i], map = ([ ]));
		}
		
		map[topic] = 1;
	}
}

void remove_topic(string topic)
{
	mixed refcount;
	string *indexes;
	int i;
	int sz;

	if (refcount = contents->get_element(topic)) {
		contents->set_element(topic, --refcount);

		if (refcount) {
			return;
		}
	} else {
		error("No such topic");
	}
	
	contents->set_element(topic, nil);

	indexes = indexes_of(topic);
	sz = sizeof(indexes);
	
	for (i = 0; i < sz; i++) {
		mapping map;
		
		map = index->get_element(indexes[i]);
		
		if (map) {
			map[topic] = nil;
			
			if (!map_sizeof(map)) {
				index->set_element(indexes[i], nil);
			}
		}
	}
}

void clear()
{
	contents->clear();
	index->clear();
}

string *fetch_topics(string topic)
{
	mapping map;
	
	map = index->get_element(topic);
	
	if (!map) {
		return ({ });
	}
	
	return map_indices(map);
}
