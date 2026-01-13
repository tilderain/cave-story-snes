#include <stdlib.h>


#include "common.h"
#include "audio.h"
#include "bank_data.h"
#include "camera.h"
//#include "dma.h"
#include "effect.h"
//#include "error.h"
#include "joy.h"
//#include "memory.h"
#include "npc.h"
#include "player.h"
//#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
//#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "weapon.h"

#include "entity.h"
#include "ai.h"

//#include "gba.h"

/* Linked List Macros */
#define MAX_ENTITIES 256
#define MAX_SPRITES_PER_ENTITY 8  // Adjust based on your needs

static uint8_t entity_pool[MAX_ENTITIES][sizeof(Entity) + sizeof(VDPSprite) * MAX_SPRITES_PER_ENTITY];
static uint8_t entity_pool_used[MAX_ENTITIES];


#define LIST_PUSH(list, obj) do {                                                             \
	obj->next = list;                                                                          \
	obj->prev = NULL;                                                                          \
	if(list) list->prev = obj;                                                                 \
	list = obj;                                                                                \
} while(0)

#define LIST_REMOVE(list, obj) do {                                                            \
	if(obj->next) obj->next->prev = obj->prev;                                                 \
	if(obj->prev) obj->prev->next = obj->next;                                                 \
	else list = obj->next;                                                                     \
} while(0)

#define LIST_MOVE(fromList, toList, obj) do {                                                  \
	LIST_REMOVE(fromList, obj);                                                                \
	LIST_PUSH(toList, obj);                                                                    \
} while(0)

#define LIST_CLEAR(list) do {                                                                  \
	Entity *temp;                                                                              \
	while(list) {                                                                              \
		temp = list;                                                                           \
		LIST_REMOVE(list, list);                                                               \
		for(uint16_t i = 0; i < MAX_ENTITIES; i++) {                                          \
			if((Entity*)entity_pool[i] == temp) {                                             \
				entity_pool_used[i] = 0;                                                      \
				break;                                                                         \
			}                                                                                  \
		}                                                                                      \
	}                                                                                          \
} while(0)

// Heightmaps for slopes
const uint8_t heightmap[4][16] = {
	{ 0x0,0x0,0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x5,0x6,0x6,0x7,0x7 },
	{ 0x8,0x8,0x9,0x9,0xA,0xA,0xB,0xB,0xC,0xC,0xD,0xD,0xE,0xE,0xF,0xF },
	{ 0xF,0xF,0xE,0xE,0xD,0xD,0xC,0xC,0xB,0xB,0xA,0xA,0x9,0x9,0x8,0x8 },
	{ 0x7,0x7,0x6,0x6,0x5,0x5,0x4,0x4,0x3,0x3,0x2,0x2,0x1,0x1,0x0,0x0 },
};

Entity *pieces[10] = {NULL}; // List for bosses to keep track of parts

Entity *water_entity = NULL;

uint16_t entity_active_count = 0;

uint8_t moveMeToFront = FALSE;

Entity *entityList = NULL, *inactiveList = NULL, *bossEntity = NULL;

AnimationFrame* get_animation_frame(uint16_t type)
{
	if(npc_info[type].sheet != NOSHEET)
	{
		uint8_t sheet = 0;
		SHEET_FIND(sheet, npc_info[type].sheet);
		return sheets[sheet].sprite->animations[0]->frames[0];
	}
	else if (npc_info[type].sprite)
	{
		return npc_info[type].sprite->animations[0]->frames[0];
	}
	else
	{
		//return SPR_Sue.animations[0]->frames[0];
		return 0;
	}
}

uint8_t entity_on_screen(Entity *e) {
	uint32_t x = e->x, y = e->y;
	return x - camera_xmin < camera_xsize && y - camera_ymin < camera_ysize;
}

// Move to inactive list, delete sprite
void entity_deactivate(Entity *e) {
	LIST_MOVE(entityList, inactiveList, e);
	// If we had tile allocation release it for future generations to use
	if(e->tiloc != NOTILOC) {
		TILOC_FREE(e->tiloc, e->framesize);
		e->tiloc = NOTILOC;
	}
}

// Move into active list, recreate sprite
void entity_reactivate(Entity *e) {
	LIST_MOVE(inactiveList, entityList, e);
	if(e->sheet == NOSHEET && npc_info[e->type].sprite) {
		// Try to allocate some VRAM
		TILOC_ADD(e->tiloc, e->framesize);
		if(e->tiloc != NOTILOC) {
		const AnimationFrame *f = get_animation_frame(e->type);
			//uint8_t sprite_count = f->numSprite;
			//if(npc_info[e->type].sprite == NULL) sprite_count = 0;
			e->vramindex = tiloc_index + (e->tiloc << 2);
			uint16_t tile_offset = 0;
			for(uint8_t i = 0; i < e->sprite_count; i++) {
				sprite_index(e->sprite[i], e->vramindex + tile_offset);
				tile_offset += f->vdpSpritesInf[i]->numTile;
			}
			e->oframe = 255;
		}
	}
}

Entity *entity_delete(Entity *e) {
	Entity *next = e->next;
	LIST_REMOVE(entityList, e);
	if(e->tiloc != NOTILOC) {
		TILOC_FREE(e->tiloc, e->framesize);
		e->tiloc = NOTILOC;
	}
	// Free from pool
	for(uint16_t i = 0; i < MAX_ENTITIES; i++) {
		if((Entity*)entity_pool[i] == e) {
			entity_pool_used[i] = 0;
			break;
		}
	}
	return next;
}
Entity *entity_delete_inactive(Entity *e) {
	Entity *next = e->next;
	LIST_REMOVE(inactiveList, e);
	// Free from pool
	for(uint16_t i = 0; i < MAX_ENTITIES; i++) {
		if((Entity*)entity_pool[i] == e) {
			entity_pool_used[i] = 0;
			break;
		}
	}
	return next;
}

Entity *entity_destroy(Entity *e) {
	sound_play(e->deathSound, 5);
	entity_drop_powerup(e);
	effect_create_smoke(e->x >> CSF, e->y >> CSF);
	effect_create_smoke(e->x >> CSF, e->y >> CSF);
	if(e->flags & NPC_EVENTONDEATH) tsc_call_event(e->event);
	if(e->flags & NPC_DISABLEONFLAG) system_set_flag(e->id, TRUE);
	return entity_delete(e);
}

void entities_clear() {
	LIST_CLEAR(entityList);
	LIST_CLEAR(inactiveList);
}

uint16_t entities_count_active() {
	uint16_t count = 0;
	Entity *e = entityList;
	while(e) {
		count++;
		e = e->next;
	}
	return count;
}

uint16_t entities_count_inactive() {
	uint16_t count = 0;
	Entity *e = inactiveList;
	while(e) {
		count++;
		e = e->next;
	}
	return count;
}

uint16_t entities_count() {
	return entities_count_active() + entities_count_inactive();
}

u8 id = 1;
// ---------------------------------------------------------
// SNES Optimization Globals
// We cache 32-bit values into 16-bit variables once per frame
// to avoid expensive shifting and 32-bit math inside loops.
// ---------------------------------------------------------
static int16_t cam_x_int, cam_y_int;
static uint8_t active_bullets_idx[MAX_BULLETS];

// Update this once per frame to avoid 32-bit global reads in loops
static void update_view_cache() {
    cam_x_int = camera.x_shifted;
    cam_y_int = camera.y_shifted;
}

void entities_update(uint8_t draw) {
    // 16-bit register vars
    register int16_t i; 
    uint16_t new_active_count = 0;
    Entity *e = entityList;
    Entity *next_e; 
    
    // Cache camera to local vars
    update_view_cache();
    int16_t cam_x = cam_x_int;
    int16_t cam_y = cam_y_int;

    // ------------------------------------------------------------------------
    // PHASE 1: PRE-CALCULATION
    // Build a list of active bullet indices. 
    // We do NOT cache X/Y coordinates here because bullets have width/height
    // and caching a single point causes collision errors with large bullets.
    // ------------------------------------------------------------------------
    uint8_t bullet_count = 0;
    uint8_t *bidx_ptr = active_bullets_idx;
    
    for(i = 0; i < MAX_BULLETS; ++i) {
        if(playerBullet[i].ttl) {
            *bidx_ptr++ = i;
            bullet_count++;
        }
    }

    // Cache Player Position (16-bit)
    int16_t p_x_i = (int16_t)(player.x >> CSF);
    int16_t p_y_i = (int16_t)(player.y >> CSF);

    // ------------------------------------------------------------------------
    // PHASE 2: ENTITY LOOP
    // ------------------------------------------------------------------------
    while(e) {
        // Cache next pointer immediately. 
        next_e = e->next;

        // 1. Culling
        if(!e->alwaysActive && !entity_on_screen(e)) {
            entity_deactivate(e);
            e = next_e;
            continue;
        }
        
        new_active_count++;

        // 2. AI Update
        ENTITY_ONFRAME(e);
        
        // State Machine Check
        if(e->state == STATE_DELETE) { e = entity_delete(e); e = next_e; continue; }
        if(e->state == STATE_DESTROY) { e = entity_destroy(e); e = next_e; continue; }

        // Cache Entity Data to locals/registers
        int16_t e_x_i = (int16_t)(e->x >> CSF);
        int16_t e_y_i = (int16_t)(e->y >> CSF);
        uint16_t flags = e->flags;

        // 3. Bullet Collision (Optimized Loop)
        if(flags & NPC_SHOOTABLE) {
            // Calculate Entity Bounds in World Space (16-bit)
            int16_t ex1 = e_x_i - e->hit_box.left;
            int16_t ex2 = e_x_i + e->hit_box.right;
            int16_t ey1 = e_y_i - e->hit_box.top;
            int16_t ey2 = e_y_i + e->hit_box.bottom;

            uint8_t cont = FALSE;
            uint8_t *cur_bidx = active_bullets_idx;

			int16_t ent_scr_x = e_x_i - cam_x; // Entity position relative to screen
			int16_t ent_scr_y = e_y_i - cam_y;

			for(i = 0; i < bullet_count; i++) {
			    Bullet *b = &playerBullet[active_bullets_idx[i]];
                if(!b->ttl) continue;

			    // Calculate bullet screen position
			    int16_t bul_scr_x = (int16_t)(b->x >> CSF) - cam_x;
			    int16_t bul_scr_y = (int16_t)(b->y >> CSF) - cam_y;

			    // Now check collision using these small, safe numbers
			    if(bul_scr_x + b->hit_box.right >= ent_scr_x - e->hit_box.left &&
			       bul_scr_x - b->hit_box.left <= ent_scr_x + e->hit_box.right &&
			       bul_scr_y + b->hit_box.bottom >= ent_scr_y - e->hit_box.top &&
			       bul_scr_y - b->hit_box.top <= ent_scr_y + e->hit_box.bottom) 
			    {
       				entity_handle_bullet(e, b);
                    if(e->state == STATE_DESTROY) {
                        entity_destroy(e); 
                        cont = TRUE;
                        break;
                    } else if(e->state == STATE_DELETE) {
                        entity_delete(e);
                        cont = TRUE;
                        break;
                    }
                }
            }
            if(cont) { e = next_e; continue; }
        }

        // 4. Physics / Solid Handling
        // Optimization: Unsigned comparison for "Near Player" check
        int16_t dx = e_x_i - p_x_i;
        int16_t dy = e_y_i - p_y_i;
        
        // Only run physics if near player (e.g. within 64 pixels)
        // Using unsigned casting trick: if (unsigned)dx > 64 includes negative values (large uint)
        // You can adjust the range as needed, or remove the 'true' wrapper
        if (true) { 
            uint8_t collided = FALSE;
            
            if(flags & (NPC_SOLID | NPC_SPECIALSOLID)) {
                 bounding_box collision = entity_react_to_collision(&player, e);
                 // Fast check if struct is not zero
                 collided = *((uint32_t*) &collision) > 0; 
                 
                 if(flags & NPC_SPECIALSOLID) {
                     player.x = player.x_next;
                     player.y = player.y_next;
                     if(collided && player.health > 0 && (e->type == OBJ_BLOCK_MOVEH || e->type == OBJ_BLOCK_MOVEV)) {
                         if(blk(player.x, 0, player.y, 0) == 0x41) player_inflict_damage(100);
                     }
                 } else {
                     // Standard Solid
                     if(collision.bottom && e->y > player.y) {
                         player.y -= 1<<CSF;
                         if(flags & NPC_BOUNCYTOP) {
                             player.y_speed = -(1 << CSF);
                             player.grounded = FALSE;
                         } else {
                             playerPlatform = e;
                             playerPlatformTime = 0;
                         }
                     } else if(collision.top && e->y < player.y) {
                         player.y += 1<<CSF;
                     } else if(collision.left && e->x < player.x) {
                         player.x += 1<<CSF;
                     } else if(collision.right && e->x > player.x) {
                         player.x -= 1<<CSF;
                     }
                 }
                 
                 // Shared platform logic
                 if((flags & NPC_SPECIALSOLID) && collision.bottom) {
                     if(flags & NPC_BOUNCYTOP) {
                         player.y_speed = -(1 << CSF);
                         player.grounded = FALSE;
                     } else {
                         playerPlatform = e;
                         playerPlatformTime = 0;
                     }
                 }
            }
            
            // Player Damage
            if(e->attack && !playerIFrames && !tscState) {
                if(!(flags & (NPC_SOLID | NPC_SPECIALSOLID))) {
                    // Soft hit box check
                    player.hit_box = PLAYER_SOFT_HIT_BOX;
                    collided = entity_overlapping(&player, e);
                    player.hit_box = PLAYER_HARD_HIT_BOX;
                }
                if(collided) {
                    if(flags & NPC_FRONTATKONLY) {
                         // Optimized vertical distance check
                         if(!PLAYER_DIST_Y(e, pixel_to_sub(e->hit_box.top + 3))) {
                             collided = FALSE;
                         } else {
                            if(e->dir) {
                                if(player.x < e->x) collided = FALSE;
                            } else {
                                if(player.x > e->x) collided = FALSE;
                            }
                        }
                    }
                    if(collided && player_inflict_damage(e->attack)) return;
                }
            }
        }

        // 5. Visual Effects
        if(e->damage_value) {
            e->damage_time--;
            // Bitwise optimization for shake
            if(e->shakeWhenHit) e->xoff = (e->damage_time & 3) - 1;
            if(!e->damage_time) {
                if(e->flags & NPC_SHOWDAMAGE) {
                    //effect_create_damage(e->damage_value, e, 0, 0);
                }
                e->damage_value = 0;
                e->xoff = 0;
            }
        }

        // 6. Drawing
        if(draw && !e->hidden) {
            int16_t base_x = e_x_i - cam_x + e->xoff;
            
            // Optimization: Rough screen bounds check to skip logic for off-screen sprites
            // SNES width 256. Allow generous buffer (-32 to 288)
            if((uint16_t)(base_x + 32) < 320) {
                int16_t base_y = e_y_i - cam_y;
                
                manual_oam_set(id++, base_x, base_y, 3, 0, 0, 0, 0, 1);

                if(e->sheet != NOSHEET) {
                    int16_t scr_x = base_x - e->display_box.left;
                    int16_t scr_y = base_y - e->display_box.top;
                    
                    sprite_pos(e->sprite[0], scr_x, scr_y);
                    sprite_index(e->sprite[0], e->vramindex + frameOffset[e->sheet][e->frame]);
                    sprite_hflip(e->sprite[0], e->dir);
                    vdp_sprites_add(e->sprite, e->sprite_count);
                } 
                else if(e->tiloc != NOTILOC) {
                    // Optimize pointer chasing
                    const AnimationFrame *f = npc_info[e->type].sprite->animations[0]->frames[e->frame];
                    
                    if(e->frame != e->oframe) {
                        e->oframe = e->frame;
                        TILES_QUEUE(f->tileset->tiles, e->vramindex, e->framesize);
                    }
                    
                    int16_t bx, by;
                    int16_t w = min(f->w, 32);

                    if(e->dir) {
                        bx = base_x + e->display_box.left - w;
                        by = base_y - e->display_box.top;
                        
                        for(i = 0; i < e->sprite_count; i++) {
                            sprite_pos(e->sprite[i], bx, by);
                            sprite_hflip(e->sprite[i], 1);
                            
                            if(w >= f->w) {
                                w = min(f->w, 32);
                                by += 32;
                                bx = base_x + e->display_box.left - w; // Reset X
                            } else {
                                int16_t diff = min(f->w - w, 32);
                                w += diff;
                                bx -= diff; // Move Left
                            }
                        }
                    } else {
                        bx = base_x - e->display_box.left;
                        by = base_y - e->display_box.top;
                        int16_t x_accum = 0;
                        
                        for(i = 0; i < e->sprite_count; i++) {
                            sprite_pos(e->sprite[i], bx + x_accum, by);
                            sprite_hflip(e->sprite[i], 0);

                            x_accum += 32;
                            if(x_accum >= f->w) {
                                x_accum = 0;
                                by += 32;
                            }
                        }
                    }
                    vdp_sprites_add(e->sprite, e->sprite_count);
                }
            }
        }

        // 7. List Management
        if(moveMeToFront) {
            moveMeToFront = FALSE;
            LIST_REMOVE(entityList, e);
            LIST_PUSH(entityList, e);
        }
        
        // Advance to the pre-calculated next pointer
        e = next_e;
    }
    entity_active_count = new_active_count;
}
  

void entity_handle_bullet(Entity *e, Bullet *b) {
	//uint16_t flags = e->flags | e->eflags;
	// Destroy the bullet, or if it is a missile make it explode
	if(b->type == WEAPON_MISSILE || b->type == WEAPON_SUPERMISSILE) {
		if(!b->state) {
			bullet_missile_explode(b);
			if((e->flags & NPC_INVINCIBLE) || e->type == OBJ_MA_PIGNON) {
				sound_play(SND_TINK, 5);
			} else {
				if(b->damage < e->health) sound_play(e->hurtSound, 5);
			}
		} else if(e->type == OBJ_MA_PIGNON) {
			// Ma Pignon is invulnerable to missiles
			return;
		}
	} else if(b->type == WEAPON_SPUR || b->type == WEAPON_SPUR_TAIL 
			|| (b->type == WEAPON_BLADE && b->level == 3)) {
		// Don't destroy Spur or Blade L3
		b->hits++;
		if(!(e->flags & NPC_INVINCIBLE) && !(e->damage_time) && b->damage < e->health) {
			sound_play(e->hurtSound, 5);
		}
	} else if(b->type == WEAPON_NEMESIS && b->level < 3) {
        if(e->flags & NPC_INVINCIBLE) {
            bullet_deactivate(b);
            sound_play(SND_TINK, 5);
            return;
        }
        if(b->damage < e->health) {
            if (b->last_hit[0] == e || b->last_hit[1] == e) {
                return;
            } else {
                if (b->last_hit[0] == NULL) {
                    b->last_hit[0] = e;
                } else if (b->last_hit[1] == NULL) {
                    b->last_hit[1] = e;
                } else {
                    b->last_hit[0] = e;
                }
                sound_play(e->hurtSound, 5);
            }
        }
	} else {
        bullet_deactivate(b);
		if(e->flags & NPC_INVINCIBLE) {
			sound_play(SND_TINK, 5);
		} else {
			if(b->damage < e->health) sound_play(e->hurtSound, 5);
		}
        if(b->type == WEAPON_POLARSTAR) {
            effect_create_misc(EFF_PSTAR_HIT, b->x >> CSF, b->y >> CSF, FALSE);
        }
	}
	if(!(e->flags & NPC_INVINCIBLE)) {
		if(e->health <= b->damage) {
			if(e->flags & NPC_SHOWDAMAGE) {
				//effect_create_damage(e->damage_value - b->damage, NULL, e->x >> CSF, e->y >> CSF);
				e->damage_time = e->damage_value = 0;
			}
			// Killed enemy
			e->health = 0;
			ENTITY_ONDEATH(e);
			if(b->type == WEAPON_SPUR || b->type == WEAPON_SPUR_TAIL) {
				if(--b->damage == 0) b->ttl = 0;
			}
			return;
		} else if((e->flags & NPC_SHOWDAMAGE) || e->shakeWhenHit) {
			e->damage_value -= b->damage;
			e->damage_time = 30;
		}
		e->health -= b->damage;
		if(b->type == WEAPON_SPUR || b->type == WEAPON_SPUR_TAIL) {
			if(--b->damage == 0) bullet_deactivate(b);
		}
	}
}

void entities_update_inactive() {
	Entity *e = inactiveList;
	while(e) {
		if(entity_on_screen(e)) {
			Entity *next = e->next;
			entity_reactivate(e);
			e = next;
		} else {
			e = e->next;
		}
	}
}

void entity_update_collision(Entity *e) {
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if (e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		e->grounded = collide_stage_floor_grounded(e);
	} else if(e->y_speed > 0) {
		e->grounded = collide_stage_floor(e);
		return;
	}
	if(e->y_speed < 0) collide_stage_ceiling(e);
}

uint8_t collide_stage_leftwall(Entity *e) {
	int16_t xoff = e->dir ? e->hit_box.right : e->hit_box.left;
	uint16_t pixel_x = (e->x_next >> CSF) - xoff;
	uint16_t pixel_y = (e->y_next >> CSF);
	uint16_t block_x = pixel_to_block(pixel_x);
	uint16_t block_y1 = pixel_to_block(pixel_y - e->hit_box.top + 3);
	uint16_t block_y2 = pixel_to_block(pixel_y + e->hit_box.bottom - 3);
	uint8_t pxa1 = stage_get_block_type(block_x, block_y1);
	uint8_t pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next &= ~0x1FF; // Align to pixel
		e->x_next += pixel_to_sub(min((pixel_x & ~0xF) + 16 - pixel_x, 3));
		return TRUE;
	}
	return FALSE;
}

uint8_t collide_stage_rightwall(Entity *e) {
	int16_t xoff = e->dir ? e->hit_box.left : e->hit_box.right;
	uint16_t pixel_x = (e->x_next >> CSF) + xoff;
	uint16_t pixel_y = (e->y_next >> CSF);
	uint16_t block_x = pixel_to_block(pixel_x);
	uint16_t block_y1 = pixel_to_block(pixel_y - e->hit_box.top + 3);
	uint16_t block_y2 = pixel_to_block(pixel_y + e->hit_box.bottom - 3);
	uint8_t pxa1 = stage_get_block_type(block_x, block_y1);
	uint8_t pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next &= ~0x1FF;
		e->x_next -= pixel_to_sub(min(pixel_x - (pixel_x & ~0xF), 3));
		return TRUE;
	}
	return FALSE;
}

uint8_t collide_stage_floor(Entity *e) {
	uint16_t pixel_x1, pixel_x2, pixel_x3, pixel_y;
	uint8_t pxa1, pxa2, pxa3;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_x3 = sub_to_pixel(e->x_next);
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	pxa3 = stage_get_block_type(pixel_to_block(pixel_x3), pixel_to_block(pixel_y + 2));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_speed = 0;
		e->y_next = pixel_to_sub((pixel_y&~0xF) - e->hit_box.bottom);
		return TRUE;
	}
	if(!e->enableSlopes) return FALSE;
	uint8_t result = FALSE;
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			(pixel_y&15) >= heightmap[pxa1&1][pixel_x1&15]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1&1][pixel_x1&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			(pixel_y&15) >= (uint16_t)(0xF - heightmap[pxa2&1][pixel_x2&15])) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 0xF + 1 -
				heightmap[pxa2&1][pixel_x2&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	// Extra check in the center
	if(!result && (pxa3 & 0x10)) {
		if((pxa3 & 0xF) >= 4 && ((pixel_y + 2) & 15) >= heightmap[pxa3&3][pixel_x3&15]) {
			if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
			e->y_next = e->y;
			e->y_speed = 0;
			result = TRUE;
		}
	}
	return result;
}

uint8_t collide_stage_slope_grounded(Entity *e) {
	uint16_t pixel_x1, pixel_x2, pixel_x3, pixel_y;
	uint8_t pxa1, pxa2, pxa3;
	uint8_t result = FALSE;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_x3 = sub_to_pixel(e->x_next);
	// If we are on flat ground and run up to a slope
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom - 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			(pixel_y&15) >= heightmap[pxa1&3][pixel_x1&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1&3][pixel_x1&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			(pixel_y&15) >= heightmap[pxa2&3][pixel_x2&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2&3][pixel_x2&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if(result) return TRUE;
	// If we're already on a slope
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom + 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	pxa3 = stage_get_block_type(pixel_to_block(pixel_x3), pixel_to_block(pixel_y + 2));
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			(pixel_y&15) >= heightmap[pxa1&3][pixel_x1&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1&3][pixel_x1&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			(pixel_y&15) >= heightmap[pxa2&3][pixel_x2&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2&3][pixel_x2&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	// Extra check in the center
	if(!result && (pxa3 & 0x10)) {
		if((pxa3 & 0xF) >= 4 && ((pixel_y + 2) & 15) >= heightmap[pxa3&3][pixel_x3&15]) {
			e->y_next = e->y;
			e->y_speed = 0;
			result = TRUE;
		}
	}
	return result;
}

uint8_t collide_stage_floor_grounded(Entity *e) {
	uint8_t result = FALSE;
	// First see if we are still standing on a flat block
	uint8_t pxa1 = stage_get_block_type(pixel_to_block(sub_to_pixel(e->x_next) - e->hit_box.left),
			pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom + 1));
	uint8_t pxa2 = stage_get_block_type(pixel_to_block(sub_to_pixel(e->x_next) + e->hit_box.right),
			pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom + 1));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
		(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		// After going up a slope and returning to flat land, we are one or
		// two pixels too low. This causes the player to ignore new upward slopes
		// which is bad, so this is a dumb hack to push us back up if we are
		// a bit too low
		if(((sub_to_pixel(e->y_next) + e->hit_box.bottom) & 15) < 4) {
			e->y_next = pixel_to_sub(((sub_to_pixel(e->y_next) + e->hit_box.bottom)&~0xF) -
				e->hit_box.bottom);
		}
		result = TRUE;
	}
	if(e->enableSlopes && collide_stage_slope_grounded(e)) {
		result = TRUE;
	}
	return result;
}

uint8_t collide_stage_ceiling(Entity *e) {
	uint16_t pixel_x1, pixel_x2, pixel_y;
	uint8_t pxa1, pxa2;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 2;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 2;
	// Without the +1 here, quote will clip to the left/right of ceiling tiles
	pixel_y = sub_to_pixel(e->y_next) - e->hit_box.top + 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	uint8_t result = FALSE;
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->y_next = pixel_to_sub((pixel_y&~0xF) + e->hit_box.top + 15) + 0x100;
		result = TRUE;
	} else {
		if((pxa1&0x10) && (pxa1&0xF) >= 0 && (pxa1&0xF) < 2 &&
				(pixel_y&15) <= (uint16_t)(0xF - heightmap[pxa1&1][pixel_x1&15])) {
			e->y_next = pixel_to_sub((pixel_y&~0xF) + 0xF -
					heightmap[pxa1&1][pixel_x1&15] + (e->hit_box.top - 1)) + 0x100;
			result = TRUE;
		}
		if((pxa2&0x10) && (pxa2&0xF) >= 2 && (pxa2&0xF) < 4 &&
				(pixel_y&15) <= heightmap[pxa2&1][pixel_x2&15]) {
			e->y_next = pixel_to_sub((pixel_y&~0xF) +
					heightmap[pxa2&1][pixel_x2&15] + (e->hit_box.top - 1)) + 0x100;
			result = TRUE;
		}
	}
	if(result) {
		if(e == &player) {
			e->jump_time = 0;
			if(!playerNoBump && e->y_speed < -SPEED_10(0x200)) {
				sound_play(SND_BONK_HEAD, 2);
				effect_create_misc(EFF_BONKL, (e->x >> CSF) - 4, (e->y >> CSF) - 6, FALSE);
				effect_create_misc(EFF_BONKR, (e->x >> CSF) + 4, (e->y >> CSF) - 6, FALSE);
				if(shoot_cooldown) {
					playerNoBump = TRUE;
				} else {
					e->y_speed = min(-e->y_speed >> 1, e->underwater ? SPEED_8(0x80) : SPEED_8(0xFF));
				}
			} else if(!shoot_cooldown || !joy_down(BUTTON_DOWN)) {
				e->y_speed = 0;
			}
		} else if(e->y_speed < -SPEED_10(0x200)) {
			e->y_speed = min(-e->y_speed >> 1, e->underwater ? SPEED_8(0x80) : SPEED_8(0xFF));
		} else {
			e->y_speed = 0;
		}
	} else if(e == &player) {
		playerNoBump = FALSE;
	}
	return result;
}


uint8_t entity_overlapping(Entity *a, Entity *b) {
    int16_t ax, bx, ay, by;
    int16_t a_width_ofs_1, a_width_ofs_2;
    int16_t b_width_ofs_1, b_width_ofs_2;

    // 1. Convert positions to 16-bit pixel coordinates first
    ax = sub_to_pixel(a->x);
    bx = sub_to_pixel(b->x);

    // 2. Resolve Hitbox X offsets based on direction
    //    Ideally, 'dir' should be 0 or 1 so we can use it as an array index 
    //    to avoid branching, but 'if' is better than ternary here.
    if (a->dir) {
        a_width_ofs_1 = a->hit_box.left;
        a_width_ofs_2 = a->hit_box.right;
    } else {
        a_width_ofs_1 = a->hit_box.right;
        a_width_ofs_2 = a->hit_box.left;
    }

    if (b->dir) {
        b_width_ofs_1 = b->hit_box.left;
        b_width_ofs_2 = b->hit_box.right;
    } else {
        b_width_ofs_1 = b->hit_box.right;
        b_width_ofs_2 = b->hit_box.left;
    }

    // 3. Early Exit on X Axis
    // Logic: (ax1 < bx2) && (ax2 > bx1)
    if ( (ax - a_width_ofs_1) >= (bx + b_width_ofs_2) ) return 0;
    if ( (ax + a_width_ofs_2) <= (bx - b_width_ofs_1) ) return 0;

    // 4. Calculate Y only if X overlapped
    ay = sub_to_pixel(a->y);
    by = sub_to_pixel(b->y);

    if ( (ay - a->hit_box.top) >= (by + b->hit_box.bottom) ) return 0;
    if ( (ay + a->hit_box.bottom) <= (by - b->hit_box.top) ) return 0;

    return 1;
}

bounding_box entity_react_to_collision(Entity *a, Entity *b) {
	bounding_box result = { 0, 0, 0, 0 };
	int16_t ax1 = sub_to_pixel(a->x_next) - (a->dir ? a->hit_box.right : a->hit_box.left),
		ax2 = sub_to_pixel(a->x_next) + (a->dir ? a->hit_box.left : a->hit_box.right),
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - (b->dir ? b->hit_box.right : b->hit_box.left),
		bx2 = sub_to_pixel(b->x) + (b->dir ? b->hit_box.left : b->hit_box.right),
		by1 = sub_to_pixel(b->y) - b->hit_box.top,
		by2 = sub_to_pixel(b->y) + b->hit_box.bottom;
	if(!(ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1)) return result;
	// This is an attempt to fix falling into platforms that are moving up
	if(abs(a->y_speed - b->y_speed) < SPEED_12(0x600)) {
		// Wall reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 1;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 1;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top + 2;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom - 3;
		if(ay1 < by2 && ay2 > by1) {
			int16_t move1 = pixel_to_sub(bx2 - ax1);
			int16_t move2 = pixel_to_sub(bx1 - ax2);
			if(abs(move1) < abs(move2)) {
				result.left = 1;
				a->x_next += move1;
				if(a->x_speed < 0) a->x_speed = 0;
			} else {
				result.right = 1;
				a->x_next += move2;
				if(a->x_speed > 0) a->x_speed = 0;
			}
		}
		// Floor reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 2;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 2;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom;
		if(ax1 < bx2 && ax2 > bx1) {
			int16_t move1 = pixel_to_sub(by2 - ay1);
			int16_t move2 = pixel_to_sub(by1 - ay2) + pixel_to_sub(1);
			if(abs(move1) < abs(move2)) {
				result.top = 1;
				a->y_next += move1;
				if(a->y_speed < 0) a->y_speed = 0;
			} else {
				result.bottom = 1;
				a->y_next += move2;
				if(a->y_speed > 0) a->y_speed = 0;
				a->grounded = TRUE;
			}
		}
	} else { // This is 100% copy paste but wall/floor reversed
		// Floor reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 2;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 2;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom;
		if(ax1 < bx2 && ax2 > bx1) {
			int16_t move1 = pixel_to_sub(by2 - ay1);
			int16_t move2 = pixel_to_sub(by1 - ay2) + pixel_to_sub(1);
			if(abs(move1) < abs(move2)) {
				result.top = 1;
				a->y_next += move1;
				if(a->y_speed < 0) a->y_speed = 0;
			} else {
				result.bottom = 1;
				a->y_next += move2;
				if(a->y_speed > 0) a->y_speed = 0;
				a->grounded = TRUE;
			}
		}
		// Wall reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 1;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 1;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top + 2;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom - 3;
		if(ay1 < by2 && ay2 > by1) {
			int16_t move1 = pixel_to_sub(bx2 - ax1);
			int16_t move2 = pixel_to_sub(bx1 - ax2);
			if(abs(move1) < abs(move2)) {
				result.left = 1;
				a->x_next += move1;
				if(a->x_speed < 0) a->x_speed = 0;
			} else {
				result.right = 1;
				a->x_next += move2;
				if(a->x_speed > 0) a->x_speed = 0;
			}
		}
	}
	return result;
}

Entity *entity_find_by_id(uint16_t id) {
	Entity *e = entityList;
	while(e) {
		if(e->id == id) return e;
		else e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->id == id) return e;
		else e = e->next;
	}
	return NULL;
}

Entity *entity_find_by_event(uint16_t event) {
	Entity *e = entityList;
	while(e) {
		if(e->event == event) return e;
		else e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) return e;
		else e = e->next;
	}
	return NULL;
}

Entity *entity_find_by_type(uint16_t type) {
	Entity *e = entityList;
	while(e) {
		if(e->type == type) return e;
		else e = e->next;
	}
	return NULL;
}

void entities_clear_by_event(uint16_t event) {
    Entity *e = entityList;
    while(e) {
		if(e->event == event) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
			e = entity_delete(e);
		} else {
            e = e->next;
        }
	}
    e = inactiveList;
    while(e) {
        if(e->event == event) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
            e = entity_delete_inactive(e);
        } else {
            e = e->next;
        }
    }
}

void entities_clear_by_type(uint16_t type) {
    Entity *e = entityList;
    while(e) {
        if(e->type == type) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
            e = entity_delete(e);
        } else {
            e = e->next;
        }
    }
    e = inactiveList;
    while(e) {
        if(e->type == type) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
            e = entity_delete_inactive(e);
        } else {
            e = e->next;
        }
    }
}

void entity_drop_powerup(Entity *e) {
	uint8_t chance = ((random() & 0x3FF) % 10) >> 1;
	if(chance >= 2) { // Weapon Energy
		if(e->experience > 0) {
			Entity *exp = entity_create(e->x, e->y, OBJ_XP,
					e->experience > 6 ? NPC_OPTION2 : 0);
			exp->experience = e->experience;
		}
	} else if(chance == 1 && (player_has_weapon(WEAPON_MISSILE) || 
		player_has_weapon(WEAPON_SUPERMISSILE))) { // Missiles
		if(e->experience > 6) {
			entity_create(e->x, e->y, 86, NPC_OPTION1 | NPC_OPTION2);
		} else {
			entity_create(e->x, e->y, 86, NPC_OPTION1);
		}
	} else { // Heart
		if(e->experience > 6) {
			Entity *heart = entity_create(e->x, e->y, 87, NPC_OPTION1 | NPC_OPTION2);
			heart->health = 5;
		} else {
			Entity *heart = entity_create(e->x, e->y, 87, NPC_OPTION1);
			heart->health = 2;
		}
	}
}

void entity_default(Entity *e, uint16_t type, uint16_t flags) {
	// Depending on the NPC type, apply default values
	e->type = type;
	e->enableSlopes = TRUE;
	e->shakeWhenHit = TRUE;
	e->tiloc = NOTILOC;
	e->sheet = NOSHEET;
	if(type < NPC_COUNT) {
		e->flags = npc_flags(type);
		e->health = npc_hp(type);
		e->attack = npc_attack(type);
		e->experience = npc_xp(type);
		e->deathSound = npc_diesfx(type);
		e->hurtSound = npc_hurtsfx(type);
		e->hit_box = npc_hitbox(type);
		e->display_box = npc_displaybox(type);
	} else {
		e->health = 1;
		e->hit_box = (bounding_box) { 8, 8, 8, 8 };
		e->display_box = (bounding_box) { 8, 8, 8, 8 };
	}
    e->flags |= flags;
}

uint8_t get_sprite_count(uint16_t type)
{
	if(npc_info[type].sheet != NOSHEET)
	{
		uint8_t sheet = 0;
		SHEET_FIND(sheet, npc_info[type].sheet);
		return sheets[sheet].sprite->animations[0]->frames[0]->numSprite;
	}
	else
	{
		return npc_info[type].sprite->animations[0]->frames[0]->numSprite;
	}
}

Entity *entity_create_ext(int32_t x, int32_t y, uint16_t type, uint16_t flags, uint16_t id, uint16_t event) {
	// Find free entity in pool
	Entity *e = NULL;
	for(uint16_t i = 0; i < MAX_ENTITIES; i++) {
		if(!entity_pool_used[i]) {
			e = (Entity*)entity_pool[i];
			entity_pool_used[i] = 1;
			break;
		}
	}
	if(!e) return NULL; // Pool exhausted
	
	memset(e, 0, sizeof(Entity) + sizeof(VDPSprite) * MAX_SPRITES_PER_ENTITY);
	e->x = x;
	e->y = y;
	e->id = id;
	e->event = event;

	const AnimationFrame *f = get_animation_frame(type);
	e->sprite_count = f->numSprite;
	if(!npc_info[type].sprite_count) e->sprite_count = 0;
	
	entity_default(e, type, flags);

	if(npc_info[type].sprite_count) {
		if(npc_info[type].sheet != NOSHEET) {
			SHEET_FIND(e->sheet, npc_info[type].sheet);
			e->vramindex = sheets[e->sheet].index;
			e->framesize = sheets[e->sheet].w * sheets[e->sheet].h;
			uint16_t tile_offset = 0;
			uint8_t i;
			for( i = 0; i < e->sprite_count; i++) {
				e->sprite[i] = (VDPSprite) {
					.size = f->vdpSpritesInf[i]->size,
					.attr = TILE_ATTR(npc_info[type].palette,0,0,0,
							e->vramindex + tile_offset)
				};
				tile_offset += f->vdpSpritesInf[i]->numTile;
			}
			e->oframe = 255;
		} else if(npc_info[type].sprite) {
			e->framesize = f->tileset->numTile;
			TILOC_ADD(e->tiloc, e->framesize);
			if(e->tiloc != NOTILOC) {
				e->vramindex = tiloc_index + e->tiloc * 4;
				uint16_t tile_offset = 0;
				uint8_t i;
				for( i = 0; i < e->sprite_count; i++) {
					e->sprite[i] = (VDPSprite) {
						.size = f->vdpSpritesInf[i]->size,
						.attr = TILE_ATTR(npc_info[type].palette,0,0,0,
								e->vramindex + tile_offset)
					};
					tile_offset += f->vdpSpritesInf[i]->numTile;
				}
				e->oframe = 255;
			}
		}
	}

	/*if(type == OBJ_DOOR)
	{

		const AnimationFrame *f = get_animation_frame(type);
		while(true)	{
			for(uint8_t i = 0; i < sprite_count; i++) {
				printf("Door Size %d: %d\n", i, f->vdpSpritesInf[i]->size);
			}
			VBlankIntrWait();
		}
	}*/

	ENTITY_ONSPAWN(e);
	if(e->alwaysActive || entity_on_screen(e)) {
		LIST_PUSH(entityList, e);
	} else {
		LIST_PUSH(inactiveList, e);
		if(e->tiloc != NOTILOC) {
			TILOC_FREE(e->tiloc, e->framesize);
			e->tiloc = NOTILOC;
		}
	}
	return e;
}

void entities_replace(uint16_t event, uint16_t type, uint8_t direction, uint16_t flags) {
    const static int flags_to_keep = (NPC_INTERACTIVE | NPC_EVENTONDEATH
                                      | NPC_DISABLEONFLAG | NPC_ENABLEONFLAG | NPC_OPTION2);
	Entity *e = entityList;
	while(e) {
		if(e->event == event) {
			// Need to re-create the structure, the replaced entity may have a different
			// number of sprites
			int32_t x = e->x;
			int32_t y = e->y;
			flags |= e->flags & flags_to_keep;
			uint16_t id = e->id;
            e = entity_delete(e);
			Entity *new = entity_create_ext(x, y, type, flags, id, event);
            new->flags |= flags;
			new->dir = direction;
		} else e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) {
            int32_t x = e->x;
            int32_t y = e->y;
            flags |= e->flags & flags_to_keep;
            uint16_t id = e->id;
            e = entity_delete_inactive(e); // So Balrog doesn't delete every entity in the room
            Entity *new = entity_create_ext(x, y, type, flags, id, event);
            new->flags |= flags;
            new->dir = direction;
		} else e = e->next;
	}
}

void entities_set_state(uint16_t event, uint16_t state, uint8_t direction) {
	Entity *e = entityList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->state = state;
		}
		e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->state = state;
		}
		e = e->next;
	}
}

void entities_move(uint16_t event, uint16_t x, uint16_t y, uint8_t direction) {
	Entity *e = entityList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->x = block_to_sub(x) + pixel_to_sub(8);
			e->y = block_to_sub(y) + pixel_to_sub(8);
			e->grounded = FALSE;
			break;
		}
		e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->x = block_to_sub(x) + pixel_to_sub(8);
			e->y = block_to_sub(y) + pixel_to_sub(8);
			e->grounded = FALSE;
			break;
		}
		e = e->next;
	}
}

uint8_t entity_exists(uint16_t type) {
	Entity *e = entityList;
	while(e) {
		if(e->type == type) return TRUE;
		e = e->next;
	}
	return FALSE;
}

void entities_draw() {
    Entity *e = entityList;
    
    // Start OAM allocation after player (index 0)
    // We rely on vdp.c's internal counter, but we need to ensure it skips 0 if player uses it manually.
    // Actually, vdp_sprites_clear() resets to 0. 
    // If player_draw() is called first and uses index 0 manually, we need to make sure vdp_sprites_add
    // doesn't overwrite it. 
    // Ideally, player should also use vdp_sprites_add or increment the counter.
    // For now, let's assume player uses 0, and we should start allocating from 1.
    // Hack: We can just burn index 0 in vdp_sprites_clear or assume player_draw increments it?
    // manual_oam_set doesn't increment the counter automatically.
    // Let's manually reserve index 0 in vdp_sprites_clear or just start loop at 1?
    // Better: manual_oam_set takes an ID. vdp_sprites_add uses a counter.
    // We should sync them.
    
    // For this specific request, let's calculate positions and draw.
    
    // We need a variable to track the next available OAM slot for entities.
    // Since player uses 0, we start at 1.
    uint8_t entity_oam_id = 1;

    while(e) {
        if(!e->hidden && entity_on_screen(e)) {
            const AnimationFrame *f = get_animation_frame(e->type);
            
            // Calculate screen position relative to camera
            // Camera position is in subpixels (CSF)
            int32_t cam_x = camera.x >> CSF;
            int32_t cam_y = camera.y >> CSF;
            int32_t ent_x = e->x >> CSF;
            int32_t ent_y = e->y >> CSF;
            
            // Offset for screen center (assuming 256x224)
            int16_t screen_x = (int16_t)(ent_x - cam_x + (256/2));
            int16_t screen_y = (int16_t)(ent_y - cam_y + (224/2));
            
            // Adjust for display box / visual center if needed
            // e->display_box is often used for culling, but here we center.
            // Usually Cave Story sprites are drawn relative to center-bottom or center.
            // Let's align center: subtract 8 or 16 depending on size?
            // Using the logic from the commented out block in entities_update:
            // int16_t bx = (e->x>>CSF) - camera.x_shifted - e->display_box.left + e->xoff;
            // camera.x_shifted is roughly camera.x >> CSF - SCREEN_HALF_W.
            
            int16_t bx = screen_x - 8 + e->xoff; // -8 to center 16px sprite
            int16_t by = screen_y - 8;           // -8 to center 16px sprite
            
            // Handle direction offset
            if(e->dir) {
                // If flipped, sometimes position needs adjustment?
                // bx = screen_x + e->display_box.left + e->xoff ...
                // Let's stick to simple centering for now.
            }

            // Iterate sprites in the metasprite
            uint16_t tile_offset = 0;
            for(uint8_t i = 0; i < e->sprite_count; i++) {
                if (entity_oam_id >= 128) break;

                // Basic 16x16 sprite assumption
                uint16_t tile = e->vramindex + tile_offset;
                if(e->sheet != NOSHEET) {
                     tile += frameOffset[e->sheet][e->frame];
                }
                
                // Add sub-sprite offset from AnimationFrame if available
                // f->vdpSpritesInf[i]->x / y
                int16_t final_x = bx;
                int16_t final_y = by;
                
                if (f && i < f->numSprite) {
                    // Adjust position based on frame definition
                    if (e->dir) {
                        // Flipped X layout logic... simplified here:
                        final_x -= (f->vdpSpritesInf[i]->x); // Approximate flip
                    } else {
                        final_x += f->vdpSpritesInf[i]->x;
                    }
                    final_y += f->vdpSpritesInf[i]->y;
                    
                    // Add tile offset
                    tile_offset += f->vdpSpritesInf[i]->numTile;
                }

                // Call Manual OAM Set
                // Priority 2 (below player), Palette from NPC info
                // Size 1 (16x16)
                manual_oam_set(entity_oam_id++, 
                               final_x, 
                               final_y, 
                               2,               // Priority
                               e->dir,          // HFlip (1=Right usually)
                               0,               // VFlip
                               tile, 
                               npc_info[e->type].palette, 
                               1);              // Size (Large)
            }
        }
        e = e->next;
    }
}
void generic_npc_states(Entity *e) {
	switch(e->state) {
		case 0:		// stand
		{
			e->frame = 0;
			e->x_speed = 0;
			e->y_speed = 0;
			if(e->type != OBJ_KAZUMA) {
				RANDBLINK(e, 3, 200);
			}
		}
		break;
		case 3:		// walking
		case 4:
		{
			static const uint8_t f[] = { 1, 0, 2, 0 };
			if(++e->animtime >= 32) e->animtime = 0;
			e->frame = f[e->animtime >> 3];
			MOVE_X(SPEED_10(0x200));
		}
		break;
		case 5:		// face away
		{
			e->frame = e->type == OBJ_KAZUMA ? 3 : 4;
			e->x_speed = 0;
		}
		break;
		case 8:		// walk (alternate state used by OBJ_NPC_JACK)
		{
			if (e->type == OBJ_JACK) {
				e->state = 4;
				e->frame = 1;
			}
		}
		break;
	}
}
