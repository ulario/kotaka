float interval;
int callout;

static void create()
{
	interval = 1.0;
	callout = call_out("spark", interval);
}

atomic void set_interval(float new_interval)
{
	if (new_interval < 0.0) {
		error("Bad interval");
	}

	remove_call_out(callout);
	interval = new_interval;
}

static void spark()
{
	callout = call_out("spark", interval);
}
