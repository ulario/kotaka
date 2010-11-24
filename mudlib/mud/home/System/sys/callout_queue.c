#include <status.h>

#include <kotaka/assert.h>
#include <kotaka/bigstruct.h>
#include <kotaka/privilege.h>

object cmap;	/* ([ oindex : ([ handle : iterator ]) ]) */
object cqueue;	/* ({ iterator : ({ obj, handle }) }) */
int begin, end;
int callouts;

static void create()
{
	cmap = clone_object(BIGSTRUCT_ARRAY_OBJ);
	cqueue = clone_object(BIGSTRUCT_ARRAY_OBJ);
	
	cmap->set_size(0x7FFFFFFF);
	cqueue->set_size(0x10000000);
	
	begin = end = 0;
	callouts = 0;
}

private int object_index(object obj)
{
	int oindex;
	
	if (sscanf(object_name(obj), "%*s#%d", oindex)) {
		return oindex;
	}
	
	return status(obj, O_INDEX);
}

int empty()
{
	ACCESS_CHECK(SYSTEM());
	
	return begin == end;
}

int callouts()
{
	ACCESS_CHECK(SYSTEM());
	
	return callouts;
}

void suspend(object obj, int handle)
{
	mapping map;
	int oindex;
	
	ACCESS_CHECK(SYSTEM());
	
	if ((end + 1) & 0x0FFFFFFF == begin) {
		error("Suspension queue overflow");
	}
	
	oindex = object_index(obj);
	
	map = cmap->get_element(oindex);
	
	if (!map) {
		map = ([ ]);
		cmap->set_element(oindex, map);
	}
	
	map[handle] = end;
	
	cqueue->set_element(end++, ({ obj, handle }) );
	end &= 0x0FFFFFFF;
	callouts++;
}

int remove_callout(object obj, int handle)
{
	mapping map;
	int oindex;
	int qindex;
	
	ACCESS_CHECK(SYSTEM());

	oindex = object_index(obj);
	
	map = cmap->get_element(oindex);
	
	if (!map || map[handle]) {
		return FALSE;
	}
	
	qindex = map[handle];
	map[handle] = nil;
	
	if (!map_sizeof(map)) {
		cmap->set_element(oindex, nil);
	}
	
	callouts--;
	
	return TRUE;
}

int remove_callouts(object obj)
{
	mapping map;
	int oindex;
	int *handles;
	int *qindices;
	int sz;
	int osz;
	
	ACCESS_CHECK(SYSTEM());

	oindex = object_index(obj);
	
	map = cmap->get_element(oindex);
	
	if (!map) {
		return 0;
	}
	
	cmap->set_element(oindex, nil);
	
	handles = map_indices(map);
	qindices = map_values(map);
	
	osz = sizeof(handles);
	
	for (sz = osz; --sz >= 0; ) {
		cqueue->set_element(qindices[sz], nil);
	}
	
	callouts -= sz;
	
	return osz;
}

mixed *release()
{
	ACCESS_CHECK(SYSTEM());

	for (;;) {
		mixed *callout;
		mixed map;
		int oindex;
		
		if (begin == end) {
			cqueue->clear();
			begin = end = 0;
			return nil;
		}
		
		callout = cqueue->get_element(begin);
		cqueue->set_element(begin++, nil);
		
		begin &= 0x0FFFFFFF;
		
		if (!callout) {
			continue;
		}
		
		oindex = object_index(callout[0]);
		map = cmap->get_element(oindex);
		
		ASSERT(map);
		ASSERT(map[callout[1]] != nil);
		
		map[callout[1]] = nil;
		
		if (!map_sizeof(map)) {
			cmap->set_element(oindex, nil);
		}
		
		callouts--;
		
		return callout;
	}
}
