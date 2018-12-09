/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/paths/thing.h>
#include <kotaka/paths/system.h>
#include <kotaka/property.h>
#include <type.h>

void configure_properties();

static void create()
{
	configure_properties();
}

void configure_properties()
{
	PROPERTYD->clear_properties();

	/* identification */
	PROPERTYD->add_property("id", T_STRING, PROP_MAGIC, ({ "query_id", "set_id" }) );
	PROPERTYD->add_property("name", T_STRING, PROP_MAGIC, ({ "query_object_name", "set_object_name" }) );

	/* inheritance */
	PROPERTYD->add_property("archetype", T_ARRAY, PROP_MAGIC, ({ "query_archetype", "set_archetype" }) );

	/* descriptions */
	PROPERTYD->add_property("snouns", T_ARRAY, PROP_COMBO, ({ "local_snouns", "removed_snouns" }) );
	PROPERTYD->add_property("pnouns", T_ARRAY, PROP_COMBO, ({ "local_pnouns", "removed_pnouns" }) );
	PROPERTYD->add_property("adjectives", T_ARRAY, PROP_COMBO, ({ "local_adjectives", "removed_adjectives" }) );

	PROPERTYD->add_property("local_snouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("local_pnouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("local_adjectives", T_ARRAY, PROP_SIMPLE, ({ }));

	PROPERTYD->add_property("removed_snouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("removed_pnouns", T_ARRAY, PROP_SIMPLE, ({ }));
	PROPERTYD->add_property("removed_adjectives", T_ARRAY, PROP_SIMPLE, ({ }));

	PROPERTYD->add_property("brief", T_STRING, PROP_INHERIT, "nondescript object");
	PROPERTYD->add_property("look", T_STRING, PROP_INHERIT, "A nondescript object.");
	PROPERTYD->add_property("examine", T_STRING, PROP_INHERIT, "A nondescript object with no detail.");

	PROPERTYD->add_property("pronoun", T_STRING, PROP_INHERIT);

	PROPERTYD->add_property("is_definite", T_INT, PROP_INHERIT, 0);
	PROPERTYD->add_property("is_proper", T_INT, PROP_INHERIT, 0);
	PROPERTYD->add_property("is_transparent", T_INT, PROP_INHERIT, 0);

	/* inventory */
	PROPERTYD->add_property("environment", T_OBJECT, PROP_MAGIC, ({ "query_environment", "move" }) );
	PROPERTYD->add_property("inventory", T_ARRAY, PROP_MAGIC, ({ "query_inventory", nil }) );
	PROPERTYD->add_property("is_immobile", T_INT, PROP_INHERIT, 0);
	PROPERTYD->add_property("is_invisible", T_INT, PROP_INHERIT, 0); /* prevents targeting */

	/* bulk */
	PROPERTYD->add_property("mass", T_FLOAT, PROP_MAGIC, ({ "query_mass", "set_mass" }) );
	PROPERTYD->add_property("total_mass", T_FLOAT, PROP_MAGIC, ({ "query_total_mass", nil }) );
	PROPERTYD->add_property("density", T_FLOAT, PROP_MAGIC, ({ "query_density", "set_density" }) );

	PROPERTYD->add_property("volume", T_FLOAT, PROP_MAGIC, ({ "query_volume", nil }) );
	PROPERTYD->add_property("total_volume", T_FLOAT, PROP_MAGIC, ({ "query_total_volume", nil }) );

	PROPERTYD->add_property("capacity", T_FLOAT, PROP_MAGIC, ({ "query_capacity", "set_capacity" }) );
	PROPERTYD->add_property("max_mass", T_FLOAT, PROP_MAGIC, ({ "query_max_mass", "set_max_mass" }) );
	PROPERTYD->add_property("is_flexible", T_INT, PROP_MAGIC, ({ "query_flexible", "set_flexible" }) );
	PROPERTYD->add_property("is_virtual", T_INT, PROP_MAGIC, ({ "query_virtual", "set_virtual" }) );

	/* economics */
	PROPERTYD->add_property("value", T_INT, PROP_INHERIT, 0);
	PROPERTYD->add_property("currency", T_STRING, PROP_INHERIT);

	/* geometry */
	PROPERTYD->add_property("pos_x", T_INT, PROP_MAGIC, ({ "query_x_position", "set_x_position" }) );
	PROPERTYD->add_property("pos_y", T_INT, PROP_MAGIC, ({ "query_y_position", "set_y_position" }) );
	PROPERTYD->add_property("pos_z", T_INT, PROP_MAGIC, ({ "query_z_position", "set_z_position" }) );

	PROPERTYD->add_property("size_x", T_INT, PROP_MAGIC, ({ "query_x_size", "set_x_size" }) );
	PROPERTYD->add_property("size_y", T_INT, PROP_MAGIC, ({ "query_y_size", "set_y_size" }) );
	PROPERTYD->add_property("size_z", T_INT, PROP_MAGIC, ({ "query_z_size", "set_z_size" }) );

	PROPERTYD->add_property("coordinate_system", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("boundary_type", T_INT, PROP_INHERIT);

	/* catch all */
	PROPERTYD->add_property("data", T_MAPPING, PROP_SIMPLE, ([ ]) );
	PROPERTYD->add_property("random", T_INT, PROP_SIMPLE, -1);

	/* body/mobile handling */
	PROPERTYD->add_property("mobiles", T_ARRAY, PROP_SIMPLE, ({ }) );

	/* exits */
	PROPERTYD->add_property("exit_return", T_OBJECT, PROP_MAGIC, ({ "query_exit_return", "set_exit_return" }));
	PROPERTYD->add_property("exit_destination", T_OBJECT, PROP_MAGIC, ({ "query_exit_destination", "set_exit_destination" }));
	PROPERTYD->add_property("exit_direction", T_STRING, PROP_SIMPLE);
	PROPERTYD->add_property("default_exit", T_OBJECT, PROP_INHERIT);
	PROPERTYD->add_property("default_entrance", T_OBJECT, PROP_INHERIT);

	/* text painting */
	PROPERTYD->add_property("event:paint", T_STRING, PROP_INHERIT);
	PROPERTYD->add_property("paint_color", T_INT, PROP_INHERIT);
	PROPERTYD->add_property("paint_character", T_STRING, PROP_INHERIT);

	/* clothing */
	PROPERTYD->add_property("is_clothing", T_INT, PROP_INHERIT);
	PROPERTYD->add_property("fills_slot", T_INT, PROP_INHERIT);
	PROPERTYD->add_property("parts_covered", T_ARRAY, PROP_INHERIT);

	PROPERTYD->add_property("is_worn", T_INT, PROP_SIMPLE);
	PROPERTYD->add_property("clothing_layer", T_INT, PROP_SIMPLE);

	/* head, neck, chest */
	/* left arm, left wrist, left hand */
	/* right arm, right wrist, right hand */
	/* belly, hips, groin */
	/* left leg, left ankle, left foot */
	/* right leg, right ankle, right foot */
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	configure_properties();
}
