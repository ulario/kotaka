#include <kotaka/paths.h>
#include <kotaka/privilege.h>

static void load_helpfile(string dir, string entry)
{
	HELPD->add_topic(dir + "/" + entry,
		read_file("~/data/help/" + (dir ? dir + "/" : "") + entry + ".hlp")
	);
}

static void load_helpdir(string dir)
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	HELPD->add_category(dir);

	dirlist = get_dir("~/data/help/" + dir + "/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			call_out("load_helpdir", 0, dir + "/" + entry);
		} else if (sscanf(names[i], "%s.hlp", entry)) {
			call_out("load_helpfile", 0, dir, entry);
		}
	}
}

static void load_rootdir()
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	dirlist = get_dir("~/data/help/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			call_out("load_helpdir", 0, entry);
		} else if (sscanf(entry, "%s.hlp", entry)) {
			call_out("load_helpfile", 0, nil, entry);
		}
	}
}

void load_help()
{
	ACCESS_CHECK(PRIVILEGED());

	HELPD->reset();
	load_rootdir();
}
