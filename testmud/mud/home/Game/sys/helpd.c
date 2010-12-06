#include <kotaka/bigstruct.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>

inherit "~Kotaka/lib/helpd";

void load();

static void create()
{
	::create();
	
	load();
}

static void destruct()
{
	::destruct();
}

static void load_dir(string path)
{
	mixed **dir;
	string *names;
	string *parts;
	int *sizes;
	int i;
	int sz;
	
	if (path == "") {
		dir = get_dir(USR_DIR + "/Game/data/help/*");
	} else {
		dir = get_dir(USR_DIR + "/Game/data/help/" + path + "/*");
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

int compare_topics(string *ap, string *bp)
{
	int asz, bsz;
	int i, j, k;
	int reverse;
	
	asz = sizeof(ap);
	bsz = sizeof(bp);
	
	if (asz > bsz) {
		mixed temp;
		
		temp = ap;
		ap = bp;
		bp = temp;
		
		temp = asz;
		asz = bsz;
		bsz = temp;
		
		reverse = 1;
	}
	
	for (i = 0; i < bsz; i++) {
		if (bp[i] == ap[j]) {
			j++;
		}
	}
	
	if (j == i) {
		/* perfect match (?!) */
		return 0;
	} else if (j == asz) {
		/* perfect subset */
		return reverse ? -1 : 1;
	} else {
		return 0;
	}
}

string *filter_topics(string *topics)
{
	string **tparts;
	int *tsizes;
	int tsz;
	mapping candidates;
	int *contenders;
	int csz;
	string *finalists;
	
	int i, j;
	
	tsz = sizeof(topics);
	tparts = allocate(tsz);
	tsizes = allocate(tsz);
	candidates = ([ ]);
	
	for (i = 0; i < tsz; i++) {
		tparts[i] = explode(topics[i], "/");
		tsizes[i] = sizeof(tparts[i]);
		candidates[i] = 1;
	}
	
	for (i = 0; i < tsz - 1; i++) {
		if (!candidates[i]) {
			continue;
		}
		for (j = i + 1; j < tsz; j++) {
			int result;
			if (!candidates[j]) {
				continue;
			}
			
			result = compare_topics(tparts[i], tparts[j]);
			
			switch(result) {
			case -1:
				candidates[i] = nil;
				j = tsz;
				continue;
			case 1:
				candidates[j] = nil;
				continue;
			}
		}
	}
	
	contenders = map_indices(candidates);
	csz = sizeof(contenders);
	finalists = allocate(csz);
	
	for (i = 0; i < csz; i++) {
		finalists[i] = topics[contenders[i]];
	}
	
	return finalists;
}

string *choose_topics(string key)
{
	string *parts;
	string *finalists;
	string *topics;
	int **gaps;
	int tsz, psz, i, j, k;
	
	mapping candidates;
	int *contenders;
	int csz;
	
	topics = fetch_topics(key);
	tsz = sizeof(topics);

	parts = explode(key, "/");
	psz = sizeof(parts);
	
	gaps = allocate(tsz);
	candidates = ([ ]);
	
	if (!tsz) {
		return ({ });
	}
	
	for (i = 0; i < tsz; i++) {
		string *tparts;
		int j;
		int tpsz;
		int step;
		
		tparts = explode(topics[i], "/");
		tpsz = sizeof(tparts);
		gaps[i] = allocate_int(psz);
		step = 0;
		
		for (j = 0; j < tpsz; j++) {
			if (tparts[j] == parts[step]) {
				step++;
			} else {
				gaps[i][step]++;
			}
		}
		
		candidates[i] = 1;
	}
	
	for (i = 0; i < tsz - 1; i++) {
		if (!candidates[i]) {
			continue;
		}
		for (j = i + 1; j < tsz; j++) {
			if (!candidates[j]) {
				continue;
			}
			for (k = 0; k < psz; k++) {
				int ig, jg;
				
				ig = gaps[i][k];
				jg = gaps[j][k];
				
				if (ig > jg) {
					candidates[i] = nil;
					break;
				} else if (ig < jg) {
					candidates[j] = nil;
					break;
				}
			}
		}
	}
	
	contenders = map_indices(candidates);
	csz = sizeof(contenders);
	finalists = allocate(csz);
	
	for (i = 0; i < csz; i++) {
		finalists[i] = topics[contenders[i]];
	}
	
	return finalists;
}
