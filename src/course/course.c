#include "../common.h"
#include "course.h"
#include "../fs/yaz.h"
#include "../fs/arc.h"

const char* course_ids[] =
{
	"castle_course",
	"farm_course",
	"kinoko_course",
	"volcano_course",
	"factory_course",
	"shopping_course",
	"boardcross_course",
	"truck_course",
	"beginner_course",
	"senior_course",
	"ridgehighway_course",
	"treehouse_course",
	"koopa_course",
	"rainbow_course",
	"desert_course",
	"water_course",
	"old_peach_gc",
	"old_mario_gc",
	"old_waluigi_gc",
	"old_donkey_gc",
	"old_falls_ds",
	"old_desert_ds",
	"old_garden_ds",
	"old_town_ds",
	"old_mario_sfc",
	"old_obake_sfc",
	"old_mario_64",
	"old_sherbet_64",
	"old_koopa_64",
	"old_donkey_64",
	"old_koopa_gba",
	"old_heyho_gba",
};

void course_init(course_t* course)
{
	kmp_parser.init(&course->kmp);
	kcl_parser.init(&course->kcl);
}

void course_free(course_t* course)
{
	kmp_parser.free(&course->kmp);
	kcl_parser.free(&course->kcl);
}

int course_parse(course_t* course, bin_t* course_buffer)
{
	bin_t input;
	arc_t arc;
	int ret = 0;

	if (yaz_decompress(course_buffer, &input) != YAZ_OK)
	{
		printf("Error decompressing input data\n");
		return ret;
	}

	arc_parser.init(&arc);
	if (!arc_parser.parse(&arc, &input))
	{
		bin_free(&input);
		goto cleanup;
	}

	const char* kmp_filename = "course.kmp";
	bin_t* kmp_buffer = arc_find_data(&arc, kmp_filename);
	if (!kmp_buffer)
	{
		printf("Failed to find %s\n", kmp_filename);
		goto cleanup;
	}

	const char* kcl_filename = "course.kcl";
	bin_t* kcl_buffer = arc_find_data(&arc, kcl_filename);
	if (!kcl_buffer)
	{
		printf("Failed to find %s\n", kcl_filename);
		goto cleanup;
	}

	if (!kmp_parser.parse(&course->kmp, kmp_buffer))
	{
		goto cleanup;
	}

	if (!kcl_parser.parse(&course->kcl, kcl_buffer))
	{
		goto cleanup;
	}

	ret = 1;

cleanup:
	if (ret != 1)
		course_free(course);
	arc_parser.free(&arc);
	bin_free(&input);

	return ret;
}

const char* course_name_by_id(uint8_t id)
{
	if (id > ARRAY_LEN(course_ids))
		return "";
	return course_ids[id];
}

parser_t course_parser =
{
	course_init,
	course_free,
	course_parse
};