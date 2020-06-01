mapping details;
string *vetoed_details;

/* private */

private void patch_details_init()
{
	if (!details) {
		details = ([ ]);
	}

	if (!vetoed_details) {
		vetoed_details = ({ });
	}
}

/* creator */

static void create()
{
	details = ([ ]);
	vetoed_details = ({ });
}

/* setup */

/* details */

string *query_local_details()
{
	patch_details_init();

	return map_indices(details);
}

string *query_vetoed_details()
{
	patch_details_init();

	return vetoed_details[..];
}

void add_local_detail(string detail)
{
	patch_details_init();

	details[detail] = new_object("~/lwo/detail");
}

void remove_local_detail(string detail)
{
	patch_details_init();

	details[detail] = nil;
}

void veto_detail(string detail)
{
	patch_details_init();

	vetoed_details |= ({ detail });
}

void unveto_detail(string detail)
{
	patch_details_init();

	vetoed_details -= ({ detail });
}

int has_local_detail(string detail)
{
	patch_details_init();

	return !!details[detail];
}

/* nouns */

void set_local_snouns(string detail, string *snouns)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->set_snouns(snouns);
}

string *query_local_snouns(string detail)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	return details[detail]->query_snouns();
}

void add_local_snoun(string detail, string snoun)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->add_snoun(snoun);
}

void remove_local_snoun(string detail, string snoun)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->remove_snoun(snoun);
}

void set_local_pnouns(string detail, string *pnouns)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->set_pnouns(pnouns);
}

string *query_local_pnouns(string detail)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	return details[detail]->query_pnouns();
}

void add_local_pnoun(string detail, string pnoun)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->add_pnoun(pnoun);
}

void remove_local_pnoun(string detail, string pnoun)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->remove_pnoun(pnoun);
}

/* adjectives */

void set_local_adjectives(string detail, string *adjectives)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->set_adjectives(adjectives);
}

string *query_local_adjectives(string detail)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	return details[detail]->query_adjectives();
}

void add_local_adjective(string detail, string adjective)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->add_adjective(adjective);
}

void remove_local_adjective(string detail, string adjective)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->remove_adjective(adjective);
}

/* descriptions */

string *query_local_descriptions(string detail)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	return details[detail]->query_descriptions();
}

void set_local_description(string detail, string description, string text)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	details[detail]->set_description(description, text);
}

string query_local_description(string detail, string description)
{
	patch_details_init();

	if (!details[detail]) {
		error("No such detail");
	}

	return details[detail]->query_description(description);
}

/* saveload */

mapping detail_save()
{
	mapping map;
	string *dnames;
	int sz;

	dnames = map_indices(details);
	sz = sizeof(dnames);

	if (!sz) {
		return nil;
	}

	map = ([ ]);

	for (; --sz >= 0; ) {
		string dname;
		string *arr;

		object detail;
		string *descriptions;
		int dsz;

		dname = dnames[sz];
		detail = details[dname];

		map[dname] = detail->save();
	}
}

void detail_load(mapping data)
{
	details = ([ ]);

	if (data) {
		string *dnames;
		int sz;

		dnames = map_indices(data);

		for (sz = sizeof(dnames); --sz >= 0; ) {
			string dname;
			object detail;

			dname = dnames[sz];

			detail = new_object("~/lwo/detail");
			details[dname] = detail;
			detail->load(data["dname"]);
		}
	}
}
