#include <kotaka/paths.h>

inherit SECOND_AUTO;

#define LOAD_RECURSE	1
#define LOAD_NOFAIL	2

static void load_dir(string dir, varargs int flags)
{
	mixed **files;
	string *names;
	mixed *objs;
	int *sizes;
	int index;
	
	names = get_dir(dir + "/*")[0];

	for (index = 0; index < sizeof(names); index++) {
		mixed *info;
		string name;
		
		name = names[index];
		
		info = file_info(dir + "/" + name);
		
		if (info[2]) {
			continue;
		}
		
		if (info[0] == -2 && (flags & 1)) {
			load_dir(dir + "/" + name, flags);
			continue;
		}
		
		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			continue;
		}
		
		name = name[ .. strlen(name) - 3];
		
		if (flags & 2) {
			catch {
				load_object(dir + "/" + name);
			}
		} else {
			load_object(dir + "/" + name);
		}
	}
}
