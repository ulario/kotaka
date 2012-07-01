#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/property.h>

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

string ptime(int time)
{
	string c;
	string hms;
	string w;
	string m;
	string d;
	string y;

	string dt;

	c = ctime(time);

	hms = c[11 .. 18];

	w = c[0 .. 2];
	m = c[4 .. 6];
	d = c[8 .. 9];
	y = c[20 .. 23];

	if (d[0] == ' ')
		d[0] = '0';

	dt = m + " " + d + ", " + y;
	return dt + " " + hms;
}

string pmtime(mixed *mtime)
{
	int msec;
	string ms;

	msec = (int) (mtime[1] * 1000.0);
	ms = "000" + (string) msec;
	ms = ms[strlen(ms) - 3..];
	return ptime(mtime[0]) + "." + ms;
}

/* random number generation */

float rnd()
{
	/* use 30 bits of randomness */
	return ldexp((float)random(1 << 30), -30);
}

/* object */

object query_common_container(object a, object b)
{
	mapping la, ra;
	object le, re;
	
	le = a;
	re = b;
	
	la = ([ ]);
	ra = ([ ]);
	
	while (le && re) {
		la[le] = 1;
		ra[re] = 1;

		if (la[re]) {
			return re;
		}

		if (ra[le]) {
			return le;
		}

		le = le->_F_query_environment();
		re = re->_F_query_environment();
	}

	while (le) {
		if (ra[le]) {
			return le;
		}

		le = le->_F_query_environment();
	}

	while (re) {
		if (la[re]) {
			return re;
		}

		re = re->_F_query_environment();
	}
}

float pi()
{
	return atan(1.0) * 4.0;
}
