#include <kotaka/paths.h>
#include <game/paths.h>
#include <kotaka/constant.h>

inherit GAME_LIB_OBJECT;

/* soil */

/* simple container for stuff that's buried */
static void create(int clone)
{
	if (clone) {
		::create();
		
		call_out("duplicate", SUBD->rnd() * 40.0 + 20.0);
	}
}

static void destruct(int clone)
{
	if (clone) {
		::destruct();
	}
}

static void duplicate()
{
	float x, y, z;
	float randangle;
	float randrad;
	
	object seed;
	
	destruct_object(this_object());
	
	return;
	
	call_out("duplicate", SUBD->rnd() * 10.0 + 10.0);
	
	/*
	x = query_property("position:x");
	y = query_property("position:y");
	z = query_property("position:z");
	*/
	
	randangle = SUBD->rnd() * PI * 2.0;
	randrad = SUBD->rnd() * 5.0 + 5.0;
	
	x += randrad * sin(randangle);
	y += randrad * cos(randangle);
	
	seed = clone_object("soil");
	
	seed->move(this_object());
	
	seed->set_property("position:x", x);
	seed->set_property("position:y", y);
	seed->set_property("position:z", z);
}
