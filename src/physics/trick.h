#pragma once

typedef struct vehicle_t vehicle_t;

enum
{
	trick_none,
	trick_up,
	trick_down,
	trick_left,
	trick_right,
};

enum
{
	trick_state_idle,
	trick_state_ready,
	trick_state_started,
};

enum
{
	trick_kind_stunt,
	trick_kind_flip,
	trick_kind_flip_double,
};

float    trick_kind_dir_angle(int trick_kind, int weight_class);
float    trick_kind_max_dir_angle_diff(int trick_kind, int weight_class);
float    trick_kind_max_angle(int trick_kind);
float    trick_kind_initial_angle_diff(int trick_kind);
float    trick_kind_min_angle_diff(int trick_kind);
float    trick_kind_min_angle_diff_mul(int trick_kind);
float    trick_kind_angle_diff_mul_dec(int trick_kind);
uint16_t trick_kind_boost_duratiom(int trick_kind, bool is_bike);

typedef struct
{
	int			kind;
	float		angle;
	float		angle_diff;
	float		angle_diff_mul;
	float		rot_dir;
	quat_t		rot;
	uint8_t		cooldown;
	uint8_t		flip_axis;
} trick_act_t;

void trick_act_init_stunt(trick_act_t* act, uint8_t input, bool is_bike);
void trick_act_init_flip(trick_act_t* act, uint8_t input, bool is_bike, bool double_flip);
void trick_act_create(trick_act_t* act, float rot_dir);
void trick_act_update_rot(trick_act_t* act);

typedef struct
{
	uint8_t		next_input;
	uint8_t		next_timer;
	bool		boost_ramp_enabled;
	bool		has_diving_rot_bonus;
	int			state;
	trick_act_t act;
} trick_t;

void trick_init(trick_t* trick);
void trick_update_rot(trick_t* trick, vehicle_t* vehicle);
void trick_update_next(trick_t* trick, vehicle_t* vehicle, uint8_t input);
void trick_try_start(trick_t* trick, vehicle_t* vehicle);
void trick_try_end  (trick_t* trick, vehicle_t* vehicle);
bool trick_is_ready (trick_t* trick, vehicle_t* vehicle);