#include "common.h"
#include <stddef.h>

//#include "resources.h"
#include "weapon.h"
#include "vdp.h"

#include "tables.h"

const weapon_info_def weapon_info[WEAPON_COUNT] = {
	{ NULL,       PAL0, { 0, 0, 0} },
	{ NULL, PAL1, {10,20,10} }, // Snake
	{ NULL, PAL1, {10,20,10} }, // Polar Star
	{ NULL, PAL1, {10,20,20} }, // Fireball
	{ NULL,  PAL0, {30,40,10} }, // Machine Gun
	{ NULL, PAL1, {10,20,10} }, // Missiles
	{ NULL,       PAL0, { 0, 0, 0} },
	{ NULL, PAL1, {10,20,10} }, // Bubbler
	{ NULL,       PAL0, { 0, 0, 0} }, // N/A - but used for Blade AOE
	{ NULL, PAL1, {10,20,10} }, // Blade
	{ NULL,PAL0, {10,20,10} }, // Super Missiles
	{ NULL,       PAL0, { 0, 0, 0} },
	{ NULL, PAL0, {10,20,10} }, // Nemesis
	{ NULL, PAL1, {10,20,30} }, // Spur
};
