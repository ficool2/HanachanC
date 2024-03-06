#pragma once

#include "../fs/kmp.h"
#include "../fs/kcl.h"

typedef struct course_t
{
	kmp_t	kmp;
	kcl_t	kcl;
} course_t;

extern parser_t course_parser;

const char* course_name_by_id(uint8_t id);