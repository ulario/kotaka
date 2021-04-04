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

atomic void configure_properties()
{
	PROPERTYD->reset_properties();

	/* descriptions */
	PROPERTYD->set_property("pronoun", T_STRING, PROP_INHERIT);

	PROPERTYD->set_property("is_definite", T_INT, PROP_INHERIT, 0);
	PROPERTYD->set_property("is_proper", T_INT, PROP_INHERIT, 0);
	PROPERTYD->set_property("is_transparent", T_INT, PROP_INHERIT, 0);

	/* inventory */
	PROPERTYD->set_property("is_immobile", T_INT, PROP_INHERIT, 0);
	PROPERTYD->set_property("is_invisible", T_INT, PROP_INHERIT, 0); /* prevents targeting */

	PROPERTYD->set_property("environment_detail", T_STRING, PROP_MAGIC, ({ "query_environment_detail", "set_environment_detail" }) );
	PROPERTYD->set_property("prox", T_OBJECT, PROP_MAGIC, ({ "query_prox", "set_prox" }) );
	PROPERTYD->set_property("prep", T_STRING, PROP_MAGIC, ({ "query_prep", "set_prep" }) );

	/* economics */
	PROPERTYD->set_property("value", T_INT, PROP_INHERIT, 0);
	PROPERTYD->set_property("currency", T_STRING, PROP_INHERIT);

	/* geometry */
	PROPERTYD->set_property("pos_x", T_INT, PROP_MAGIC, ({ "query_x_position", "set_x_position" }) );
	PROPERTYD->set_property("pos_y", T_INT, PROP_MAGIC, ({ "query_y_position", "set_y_position" }) );
	PROPERTYD->set_property("pos_z", T_INT, PROP_MAGIC, ({ "query_z_position", "set_z_position" }) );

	PROPERTYD->set_property("size_x", T_INT, PROP_MAGIC, ({ "query_x_size", "set_x_size" }) );
	PROPERTYD->set_property("size_y", T_INT, PROP_MAGIC, ({ "query_y_size", "set_y_size" }) );
	PROPERTYD->set_property("size_z", T_INT, PROP_MAGIC, ({ "query_z_size", "set_z_size" }) );

	PROPERTYD->set_property("coordinate_system", T_STRING, PROP_INHERIT);
	PROPERTYD->set_property("boundary_type", T_INT, PROP_INHERIT);

	/* body/mobile handling */
	PROPERTYD->set_property("mobiles", T_ARRAY, PROP_SIMPLE, ({ }) );
	PROPERTYD->set_property("possessee", T_OBJECT, PROP_MAGIC, ({ "query_possessee", "possess" }) );

	/* exits */
	PROPERTYD->set_property("exit_return", T_OBJECT, PROP_MAGIC, ({ "query_exit_return", "set_exit_return" }));
	PROPERTYD->set_property("exit_destination", T_OBJECT, PROP_MAGIC, ({ "query_exit_destination", "set_exit_destination" }));
	PROPERTYD->set_property("exit_direction", T_STRING, PROP_SIMPLE);
	PROPERTYD->set_property("default_exit", T_OBJECT, PROP_INHERIT);
	PROPERTYD->set_property("default_entrance", T_OBJECT, PROP_INHERIT);

	/* text painting */
	PROPERTYD->set_property("event:paint", T_STRING, PROP_INHERIT);
	PROPERTYD->set_property("paint_color", T_INT, PROP_INHERIT);
	PROPERTYD->set_property("paint_character", T_STRING, PROP_INHERIT);

	/* clothing */
	PROPERTYD->set_property("is_clothing", T_INT, PROP_INHERIT);
	PROPERTYD->set_property("fills_slot", T_INT, PROP_INHERIT);
	PROPERTYD->set_property("parts_covered", T_ARRAY, PROP_INHERIT);

	PROPERTYD->set_property("is_worn", T_INT, PROP_SIMPLE);
	PROPERTYD->set_property("clothing_layer", T_INT, PROP_SIMPLE);

	/* head, neck, chest */
	/* left arm, left wrist, left hand */
	/* right arm, right wrist, right hand */
	/* belly, hips, groin */
	/* left leg, left ankle, left foot */
	/* right leg, right ankle, right foot */

	/* combat */
	PROPERTYD->set_property("is_weapon", T_INT, PROP_INHERIT);
	PROPERTYD->set_property("is_armor", T_INT, PROP_INHERIT);
	PROPERTYD->set_property("attack_value", T_INT, PROP_INHERIT);
	PROPERTYD->set_property("defense_value", T_INT, PROP_INHERIT);

	PROPERTYD->set_property("is_wielded", T_INT, PROP_SIMPLE);
	PROPERTYD->set_property("initiative", T_ARRAY, PROP_SIMPLE);

	/* catch all */
	PROPERTYD->set_property("data", T_MAPPING, PROP_SIMPLE, ([ ]) );
	PROPERTYD->set_property("random", T_INT, PROP_SIMPLE, -1);
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	configure_properties();
}
