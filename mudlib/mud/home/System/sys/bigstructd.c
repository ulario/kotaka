#include <kotaka/bigstruct.h>
#include <kotaka/privilege.h>
#include <kernel/access.h>

/*
TODO:  Track all bigstruct objects
*/

object new_bigstruct(string type)
{
	object bigstruct;
	
	switch (type) {
	case "array":
		bigstruct = clone_object(BIGSTRUCT_ARRAY_OBJ);
		break;
	case "deque":
		bigstruct = clone_object(BIGSTRUCT_DEQUE_OBJ);
		break;
	case "map":
		bigstruct = clone_object(BIGSTRUCT_MAP_OBJ);
		break;
	}
	
	bigstruct->grant_access(previous_object(), FULL_ACCESS);
	bigstruct->grant_access(this_object(), 0);
	
	return bigstruct;
}

void dispose_bigstruct(object bigstruct)
{
	ACCESS_CHECK(bigstruct->access_of(previous_object()) == FULL_ACCESS);

	destruct_object(bigstruct);
}
