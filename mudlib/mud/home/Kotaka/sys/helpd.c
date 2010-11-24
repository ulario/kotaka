#include <config.h>

inherit "~/lib/helpd";

static void load_dir(string path)
{
	mixed **dir;
	string *names;
	string *parts;
	int *sizes;
	int i;
	int sz;
	
	if (path == "") {
		dir = get_dir(USR_DIR + "/Kotaka/data/help/*");
	} else {
		dir = get_dir(USR_DIR + "/Kotaka/data/help/" + path + "/*");
	}
	
	names = dir[0];
	sizes = dir[1];
	
	sz = sizeof(names);
	
	for (i = 0; i < sz; i++) {
		string name;
		int isdir;
		
		if (sizes[i] == -2) {
			name = names[i];
			isdir = 1;
		} else if (!sscanf(names[i], "%s.hlp", name)) {
			continue;
		}
		
		if (path != "") {
			name = path + "/" + name;
		}
		
		if (isdir) {
			load_dir(name);
		} else {
			add_topic(name);
		}
	}
}

void load()
{
	contents->clear();
	index->clear();
	
	load_dir("");
}
