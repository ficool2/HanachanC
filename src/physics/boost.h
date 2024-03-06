#pragma once

enum
{
	boost_none = -1,

	boost_medium,
	boost_strong,
	boost_weak,

	boost_max
};

typedef struct boost_t
{
	uint16_t duration[boost_max];
	uint16_t mushroom_boost;
} boost_t;

void  boost_init(boost_t* boost);
void  boost_update(boost_t* boost);
void  boost_activate(boost_t* boost, int type, uint16_t duration);
int   boost_type(boost_t* boost);
float boost_factor(boost_t* boost);
float boost_acceleration(boost_t* boost);
float boost_limit(boost_t* boost);
