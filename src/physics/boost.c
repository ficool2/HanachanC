#include "../common.h"

#include "boost.h"

void boost_init(boost_t* boost)
{
	for (int i = 0; i < boost_max; i++)
		boost->duration[i] = 0;
	boost->mushroom_boost = 0;
}

void boost_update(boost_t* boost)
{
	for (int i = 0; i < boost_max; i++)
	{
		if (boost->duration[i] > 0)
			boost->duration[i]--;
	}

	if (boost->mushroom_boost > 0)
		boost->mushroom_boost--;
}

void boost_activate(boost_t* boost, int type, uint16_t duration)
{
	boost->duration[type] = max(duration + 1, boost->duration[type]);
}

int boost_type(boost_t* boost)
{
	for (int i = 0; i < boost_max; i++)
		if (boost->duration[i] > 0)
			return i;
	return boost_none;
}

float boost_factor(boost_t* boost)
{
	int type = boost_type(boost);
	switch (type)
	{
		case boost_medium: return 1.3f;
		case boost_strong: return 1.4f;
		case boost_weak:   return 1.2f;
		default:		   return 1.0f;
	}
}

float boost_acceleration(boost_t* boost)
{
	int type = boost_type(boost);
	switch (type)
	{
		case boost_medium: return 6.0f;
		case boost_strong: return 7.0f;
		case boost_weak:   return 3.0f;
		default:		   return 0.0f;
	}
}

float boost_limit(boost_t* boost)
{
	int type = boost_type(boost);
	switch (type)
	{
		case boost_strong: return 115.0f;
		default:		   return 0.0f;
	}
}