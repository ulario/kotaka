#include <type.h>

mixed deep_copy(mixed in, varargs mapping dupes)
{
	switch (typeof(in)) {
	case T_NIL:
		return nil;
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		return in;
	default:
		if (typeof(in) == T_OBJECT
			&& sscanf(object_name(in), "%*s#-1") == 0) {
			return in;
		}
	}

	if (!dupes) {
		dupes = ([ ]);
	}

	if (!dupes[in]) {
		int index;
		mixed *ind;
		mixed *val;

		switch (typeof(in)) {
		case T_OBJECT:
			{
				mixed data;
				object new;
				
				new = new_object(in);
				dupes[in] = new;
				new->finish_deep_copy(dupes);
			}
			break;

		case T_ARRAY:
			dupes[in] = allocate(sizeof(in));

			for(index = 0; index < sizeof(in); index++) {
				dupes[in][index] =
					deep_copy(in[index], dupes);
			}

			break;

		case T_MAPPING:
			dupes[in] = ([ ]);

			ind = map_indices(in);
			val = map_values(in);
			
			for (index = 0; index < sizeof(ind); index++) {
				ind[index] = deep_copy(ind[index], dupes);
				val[index] = deep_copy(val[index], dupes);
				
				dupes[in][ind[index]] = val[index];
			}
		}
	}

	return dupes[in];
}
