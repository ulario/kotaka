/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020, 2021, 2022  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
inherit "/lib/string/case";
inherit "/lib/sort";

mapping details;
string *vetoed_details;
string detail_environment;

mixed query_local_property(string property);
void set_local_property(string property, mixed value);
object query_archetype();

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

/* saveload */

static mapping detail_save()
{
	mapping map;
	string *dnames;
	int sz;

	dnames = map_indices(details);
	sz = sizeof(dnames);

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

	return ([
		"details": map_sizeof(map) ? map : nil,
		"vetoed_details": sizeof(vetoed_details) ? vetoed_details : nil
	]);
}

static void detail_load(mapping data)
{
	mapping map;

	vetoed_details = data["vetoed_details"];

	if (!vetoed_details) {
		vetoed_details = ({ });
	}

	details = ([ ]);

	if (map = data["details"]) {
		string *dnames;
		int sz;

		dnames = map_indices(map);

		for (sz = sizeof(dnames); --sz >= 0; ) {
			string dname;
			object detail;

			dname = dnames[sz];

			detail = new_object("~/lwo/detail");
			detail->load(map[dname]);

			details[dname] = detail;
		}
	}
}

/* setup */

/* details */

string *query_local_details()
{
	patch_details_init();

	return map_indices(details);
}

string *query_details()
{
	string *details;
	object arch;

	arch = query_archetype();

	if (arch) {
		details = arch->query_details() - vetoed_details;
	} else {
		details = ({ });
	}

	return details | query_local_details();
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

int has_detail(string detail)
{
	object arch;

	if (has_local_detail(detail)) {
		return 1;
	}

	if (sizeof(vetoed_details & ({ detail }))) {
		return 0;
	}

	if (arch = query_archetype()) {
		return arch->has_detail(detail);
	}

	return 0;
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

string *query_snouns(string detail)
{
	string *snouns;
	object arch;
	int good;

	snouns = ({ });

	if (!sizeof(vetoed_details & ({ detail }) )
		&& (arch = query_archetype())
		&& arch->has_detail(detail)
	) {
		good = 1;
		snouns = arch->query_snouns(detail);
	}

	if (details[detail]) {
		good = 1;
		snouns |= details[detail]->query_snouns();
	}

	if (good) {
		return snouns;
	} else {
		error("No such detail");
	}
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

string *query_pnouns(string detail)
{
	string *pnouns;
	object arch;
	int good;

	pnouns = ({ });

	if (!sizeof(vetoed_details & ({ detail }) )
		&& (arch = query_archetype())
		&& arch->has_detail(detail)
	) {
		good = 1;
		pnouns = arch->query_pnouns(detail);
	}

	if (details[detail]) {
		good = 1;
		pnouns |= details[detail]->query_pnouns();
	}

	if (good) {
		return pnouns;
	} else {
		error("No such detail");
	}
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

string *query_adjectives(string detail)
{
	string *adjectives;
	object arch;
	int good;

	adjectives = ({ });

	if (!sizeof(vetoed_details & ({ detail }) )
		&& (arch = query_archetype())
		&& arch->has_detail(detail)
	) {
		good = 1;
		adjectives = arch->query_adjectives(detail);
	}

	if (details[detail]) {
		good = 1;
		adjectives |= details[detail]->query_adjectives();
	}

	if (good) {
		return adjectives;
	} else {
		error("No such detail");
	}
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

string *query_descriptions(string detail)
{
	string *descriptions;
	object arch;
	int good;

	patch_details_init();

	descriptions = ({ });

	if (!sizeof(vetoed_details & ({ detail }) )
		&& (arch = query_archetype())
		&& arch->has_detail(detail)
	) {
		good = 1;
		descriptions = arch->query_descriptions(detail);
	}

	if (details[detail]) {
		good = 1;
		descriptions |= details[detail]->query_descriptions();
	}

	if (good) {
		return descriptions;
	} else {
		error("No such detail");
	}
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

string query_description(string detail, string description)
{
	object arch;
	string value;
	int good;

	patch_details_init();

	if (details[detail]) {
		good = 1;
		value = details[detail]->query_description(description);
	}

	if (value) {
		return value;
	}

	if (sizeof(vetoed_details & ({ detail }) )) {
		if (good) {
			return nil;
		} else {
			error("No such detail");
		}
	}

	arch = query_archetype();

	if (!arch) {
		return nil;
	}

	return arch->query_description(detail, description);
}

/* detail_environment */

void set_detail_environment(string detail)
{
	detail_environment = detail;
}

string query_detail_environment()
{
	return detail_environment;
}

/* patching */

void purge_empty_details()
{
	string *dlist;
	int sz;

	dlist = query_local_details();

	for (sz = sizeof(dlist); --sz >= 0; ) {
		string detail;

		detail = dlist[sz];

		if (sizeof(query_local_descriptions(detail))) {
			continue;
		}

		if (sizeof(query_local_snouns(detail))) {
			continue;
		}

		if (sizeof(query_local_pnouns(detail))) {
			continue;
		}

		if (sizeof(query_local_adjectives(detail))) {
			continue;
		}

		remove_local_detail(detail);
	}
}

atomic void patch_detail()
{
	string *snouns;
	string *pnouns;
	string *adjectives;
	string brief;
	string look;
	string examine;

	if (!sscanf(object_name(this_object()), "%*s#")) {
		return;
	}

	patch_details_init();

	snouns = query_local_property("local_snouns");
	pnouns = query_local_property("local_pnouns");
	adjectives = query_local_property("local_adjectives");
	brief = query_local_property("brief");
	look = query_local_property("look");
	examine = query_local_property("examine");

	if (!has_local_detail(nil)) {
		add_local_detail(nil);
	}

	if (snouns) {
		snouns |= query_local_snouns(nil);
		set_local_snouns(nil, snouns);
	}

	if (pnouns) {
		pnouns |= query_local_pnouns(nil);
		set_local_pnouns(nil, pnouns);
	}

	if (adjectives) {
		adjectives |= query_local_adjectives(nil);
		set_local_adjectives(nil, adjectives);
	}

	if (brief) {
		set_local_description(nil, "brief", brief);
	}

	if (look) {
		set_local_description(nil, "look", look);
	}

	if (examine) {
		set_local_description(nil, "examine", examine);
	}

	set_local_property("local_snouns", nil);
	set_local_property("local_pnouns", nil);
	set_local_property("local_adjectives", nil);
	set_local_property("brief", nil);
	set_local_property("look", nil);
	set_local_property("examine", nil);

	purge_empty_details();
}

atomic void patch_detail_downcase()
{
	int sz;
	string *new_vetoed_details;
	string *ind;

	new_vetoed_details = ({ });

	for (sz = sizeof(vetoed_details); --sz >= 0; ) {
		new_vetoed_details |= ({ vetoed_details[sz] });
	}

	quicksort(new_vetoed_details, 0, sizeof(new_vetoed_details));

	vetoed_details = new_vetoed_details;

	ind = map_indices(details - ({ nil }) );

	for (sz = sizeof(ind); --sz >= 0; ) {
		string dname;
		string ldname;

		dname = ind[sz];
		ldname = to_lower(dname);

		if (dname == ldname) {
			/* it's already lowercase, leave it alone */
			continue;
		}

		if (details[ldname]) {
			/* lowercased version already exists, delete the anomaly */
			details[dname] = nil;
			ind[sz] = nil;
			continue;
		}

		/* migrate to lowercased */
		details[ldname] = details[dname];
		details[dname] = nil;
		ind[sz] = ldname;
	}

	/* now, process each detail */
	ind -= ({ nil });
	for (sz = sizeof(ind); --sz >= 0; ) {
		details[ind[sz]]->patch_detail_downcase();
	}
}
