/*
* ptcop2xm v2.6 (9 Octave Support)
* 
* Changelog:
* - UPDATED: Increased octave range support from 7 to 9 (Notes up to 120).
* - ADDED: New resampling group (1/16th size) for highest octave optimization.
* - FIXED: Pass 1 Analysis now registers instruments even if they rely on default keys (Fixes blank notes).
* - FIXED: Envelope parsing now checks PXTN_VOICEFLAG_ENVELOPE.
* - FIXED: Envelope X-values are scaled to prevent "infinite attack" silence in XM.
* - Doubled grid resolution (Ticks per row: beat_clock / 16).
* - Doubled BPM to compensate for increased row count.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include "pxtnService.h"
#include "pxtnError.h"

// --- Constants ---
#define MAX_XM_CHANNELS 8
#define PXTN_VOICEFLAG_WAVELOOP 0x00000001
#define PXTN_VOICEFLAG_ENVELOPE 0x00000002 

// --- Resampling Tables ---
// Expanded to 5 groups to cover 9+ octaves
typedef struct {
    short divisor;   
    short shift;     
} OCTWAVE;

OCTWAVE oct_wave[5] = {
    { 1,  0 }, // Group 0: Oct 0-1 (Full Size)
    { 2, 12 }, // Group 1: Oct 2-3 (Half Size)
    { 4, 24 }, // Group 2: Oct 4-5 (Quarter Size)
    { 8, 36 }, // Group 3: Oct 6-7 (Eighth Size)
    { 16, 48 } // Group 4: Oct 8-9 (Sixteenth Size)
};

// ====================================================================================
// XM STRUCTURES
// ====================================================================================
#pragma pack(push, 1)

struct XMHeader {
    uint8_t id[17];
    uint8_t moduleName[20];
    uint8_t eof;
    uint8_t trackerName[20];
    uint16_t version;
    uint32_t headerSize;
    uint16_t songLength;
    uint16_t restartPosition;
    uint16_t channels;
    uint16_t patterns;
    uint16_t instruments;
    uint16_t flags;
    uint16_t tempo;
    uint16_t bpm;
    uint8_t patternOrder[256];
};

struct XMInstrumentHeader {
    uint32_t size;
    uint8_t name[22];
    uint8_t type;
    uint16_t numSamples;
    uint32_t sampleHeaderSize;
    uint8_t keymap[96];
    uint8_t volEnv[48];
    uint8_t panEnv[48];
    uint8_t volPoints;
    uint8_t panPoints;
    uint8_t volSustain;
    uint8_t volLoopStart;
    uint8_t volLoopEnd;
    uint8_t panSustain;
    uint8_t panLoopStart;
    uint8_t panLoopEnd;
    uint8_t volType;
    uint8_t panType;
    uint8_t vibType;
    uint8_t vibSweep;
    uint8_t vibDepth;
    uint8_t vibRate;
    uint16_t volFade;
    uint8_t reserved[22];
};

struct XMSampleHeader {
    uint32_t length;
    uint32_t loopStart;
    uint32_t loopLength;
    uint8_t volume;
    int8_t finetune;
    uint8_t type;
    uint8_t panning;
    int8_t relNote;
    uint8_t reserved;
    uint8_t name[22];
};

#pragma pack(pop)

// ====================================================================================
// INTERNAL DATA STRUCTURES
// ====================================================================================

struct ParsedSample {
    std::string name;
    int pxtn_id;
    int oct_group; 
    std::vector<int16_t> data_16; 
    
    int32_t basic_key;
    uint32_t flags;
    
    // Envelope
    bool env_enabled;
    std::vector<uint16_t> env_points; 
    int env_sustain;
};

struct GridCell {
    uint8_t note = 0;       // 1-120+, 97=Off (XM standard is 1-96, but extended is common)
    uint8_t instrument = 0; 
    uint8_t volume = 0;     // 0x10 - 0x50 (XM format)
    uint8_t effect = 0;
    uint8_t param = 0;
};

struct NoteEvent {
    int unit_id;
    int start_row;
    int end_row;
    int priority; 
};

struct VoiceStats {
    int32_t min_key;
    int32_t max_key;
    bool used;
    bool group_used[5]; // Expanded to 5 groups
    bool is_sample; 
    
    VoiceStats() {
        min_key = 2147483647;
        max_key = -2147483647;
        used = false;
        is_sample = false;
        for(int i=0; i<5; i++) group_used[i] = false;
    }
};

// ====================================================================================
// HELPERS
// ====================================================================================
// --- Pxtone I/O Callbacks ---
static bool _pxtn_r(void* user, void* p_dst, int size, int num) {
    return fread(p_dst, size, num, (FILE*)user) == (size_t)num;
}
static bool _pxtn_w(void* user, const void* p_src, int size, int num) {
    return fwrite(p_src, size, num, (FILE*)user) == (size_t)num;
}
static bool _pxtn_s(void* user, int mode, int size) {
    return fseek((FILE*)user, size, mode) == 0;
}
static bool _pxtn_p(void* user, int32_t* p_pos) {
    long i = ftell((FILE*)user);
    if (i < 0) return false;
    *p_pos = (int32_t)i;
    return true;
}

// Updated to support higher note range
uint8_t pxtn_key_to_xm(int32_t key) {
    double semitones = key / 256.0;
    int note = (int)(semitones) - 47; 
    if (note < 1) note = 1;
    // Expanded cap from 96 (8 octaves) to 120 (10 octaves)
    if (note > 120) note = 120; 
    return (uint8_t)note;
}

uint8_t calc_xm_volume(int vol, int vel) {
    int combined = vol * vel; 
    int xm_vol = (combined * 64) / (128 * 128); 
    if (xm_vol > 64) xm_vol = 64;
    if (xm_vol < 0) xm_vol = 0;
    return (uint8_t)(xm_vol + 0x10);
}

void normalize_audio(std::vector<int16_t>& data) {
    if (data.empty()) return;
    int max_val = 0;
    for (int16_t s : data) if (abs(s) > max_val) max_val = abs(s);
    if (max_val == 0 || max_val >= 32000) return; 
    double factor = 32000.0 / (double)max_val;
    for (size_t i = 0; i < data.size(); i++) data[i] = (int16_t)(data[i] * factor);
}

bool compare_notes(const NoteEvent &a, const NoteEvent &b) {
    if (a.start_row != b.start_row) return a.start_row < b.start_row;
    if (a.priority != b.priority) return a.priority > b.priority;
    return a.unit_id < b.unit_id;
}

class PxtoneToXM {
    pxtnService* pxtn;
    std::vector<uint8_t> file_mem;

public:

PxtoneToXM() { 
    pxtn = new pxtnService(_pxtn_r, _pxtn_w, _pxtn_s, _pxtn_p); 
}
    ~PxtoneToXM() { if (pxtn) delete pxtn; }

bool load_and_init(const char* filename) {
    printf("[DEBUG] Attempting to open: %s\n", filename);

    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("[DEBUG] ERROR: Could not open file.\n");
        return false;
    }

    pxtnERR pxtn_res = pxtn->init();
    if (pxtn_res != pxtnOK) {
        printf("[DEBUG] ERROR: pxtnService init failed.\n");
        fclose(f);
        return false;
    }

    pxtn->set_destination_quality(2, 44100);

    pxtn_res = pxtn->read(f);
    fclose(f); 

    if (pxtn_res != pxtnOK) {
        printf("[DEBUG] ERROR: pxtnService read failed. Code: %d\n", pxtn_res);
        return false;
    }

    pxtn_res = pxtn->tones_ready();
    if (pxtn_res != pxtnOK) {
        printf("[DEBUG] ERROR: pxtnService tones_ready failed.\n");
        return false;
    }

    return true;
}

    void export_xm(const char* out_filename) {
        int woice_count = pxtn->Woice_Num();
        int unit_count = pxtn->Unit_Num();
        const pxtnEvelist* evels = pxtn->evels;

        // Timing Calculation
        int32_t beat_num, beat_clock, meas_num;
        float beat_tempo;
        pxtn->master->Get(&beat_num, &beat_tempo, &beat_clock, &meas_num);
        int rows_per_beat = 8; // High resolution
        int ticks_per_row = beat_clock / rows_per_beat; 
        if (ticks_per_row < 1) ticks_per_row = 1;
        
        int32_t max_clock = pxtn->evels->get_Max_Clock();
        int total_rows = (max_clock / ticks_per_row) + 64; 

        // =========================================================================
        // PASS 1: ANALYZE INSTRUMENT USAGE (FIXED)
        // =========================================================================
        std::vector<VoiceStats> v_stats(woice_count);
        std::vector<int> unit_current_woice(unit_count, 0);
        std::vector<int> unit_current_key(unit_count, 0x6000); 

        for (const EVERECORD* e = evels->get_Records(); e != NULL; e = e->next) {
            if (e->unit_no >= unit_count) continue;
            
            if (e->kind == EVENTKIND_VOICENO) {
                unit_current_woice[e->unit_no] = e->value;
            }
            else if (e->kind == EVENTKIND_KEY) {
                unit_current_key[e->unit_no] = e->value;
                int w = unit_current_woice[e->unit_no];
                if (w < woice_count) {
                    v_stats[w].used = true;
                    if (e->value < v_stats[w].min_key) v_stats[w].min_key = e->value;
                    if (e->value > v_stats[w].max_key) v_stats[w].max_key = e->value;
                    
                    uint8_t note = pxtn_key_to_xm(e->value);
                    int oct = (note - 1) / 12;
                    if (oct < 0) oct = 0; 
                    if (oct > 9) oct = 9; // Expanded to 9
                    int group = oct / 2;
                    if (group > 4) group = 4;
                    v_stats[w].group_used[group] = true;
                }
            }
            else if (e->kind == EVENTKIND_ON) {
                int w = unit_current_woice[e->unit_no];
                int k = unit_current_key[e->unit_no];
                if (w < woice_count) {
                    v_stats[w].used = true;
                    if (k < v_stats[w].min_key) v_stats[w].min_key = k;
                    if (k > v_stats[w].max_key) v_stats[w].max_key = k;

                    uint8_t note = pxtn_key_to_xm(k);
                    int oct = (note - 1) / 12;
                    if (oct < 0) oct = 0; 
                    if (oct > 9) oct = 9; // Expanded to 9
                    int group = oct / 2;
                    if (group > 4) group = 4;
                    v_stats[w].group_used[group] = true;
                }
            }
        }

        // Determine if voices are samples
        for(int i=0; i<woice_count; i++) {
             const pxtnWoice* w = pxtn->Woice_Get(i);
             if (w) {
                 const pxtnVOICEINSTANCE* inst = w->get_instance(0);
                 if (inst && inst->p_smp_w) v_stats[i].is_sample = true;
             }
        }

        // =========================================================================
        // PASS 2: GENERATE INSTRUMENTS & MAPS
        // =========================================================================
        std::vector<ParsedSample> instruments;
        // Map size expanded to 5
        std::vector<std::vector<int>> xm_inst_map(woice_count, std::vector<int>(5, 0));
        int xm_inst_counter = 1;

        for (int i = 0; i < woice_count; i++) {
            if (!v_stats[i].used) continue;

            const pxtnWoice* woice = pxtn->Woice_Get(i);
            int range = v_stats[i].max_key - v_stats[i].min_key;
            bool single_mode = (range <= 3072); 

            int name_len = 0;
            std::string base_name;
            const char* name_ptr = woice->get_name_buf(&name_len);
            if (name_len > 0) base_name = std::string(name_ptr, name_len);
            else base_name = "Inst " + std::to_string(i);

            const pxtnVOICEUNIT* unit = woice->get_voice(0);
            const pxtnVOICEINSTANCE* inst = woice->get_instance(0);
            
            std::vector<int16_t> original_mono;
            bool has_sample = false;
            
            if (unit && inst && inst->p_smp_w) {
                has_sample = true;
                int32_t len = inst->smp_body_w; 
                original_mono.resize(len);
                int16_t* raw = (int16_t*)inst->p_smp_w;
                for(int k=0; k<len; k++) original_mono[k] = (raw[k*2] + raw[k*2+1]) / 2;
            }

            if (single_mode) {
                ParsedSample ps;
                ps.pxtn_id = i;
                ps.oct_group = 0; 
                ps.name = base_name;
                ps.env_enabled = false;
                
                if (has_sample) {
                    ps.basic_key = unit->basic_key;
                    ps.flags = unit->voice_flags;
                    ps.data_16 = original_mono;
                    parse_envelope(unit, ps);
                }
                instruments.push_back(ps);
                // Map all groups to this single instrument
                for(int g=0; g<5; g++) xm_inst_map[i][g] = xm_inst_counter;
                xm_inst_counter++;
            } else {
                // Loop extended to 5
                for(int g = 0; g < 5; g++) {
                    if (!v_stats[i].group_used[g]) continue; 

                    ParsedSample ps;
                    ps.pxtn_id = i;
                    ps.oct_group = g;
                    ps.name = base_name;
                    ps.env_enabled = false;

                    if (has_sample) {
                        ps.basic_key = unit->basic_key;
                        ps.flags = unit->voice_flags;
                        parse_envelope(unit, ps);

                        int divisor = oct_wave[g].divisor;
                        if (divisor == 1) {
                            ps.data_16 = original_mono;
                        } else {
                            int src_len = original_mono.size();
                            int dst_len = src_len / divisor;
                            if (dst_len < 2) dst_len = 2;
                            ps.data_16.resize(dst_len);
                            double step = (double)src_len / (double)dst_len;
                            double pos = 0.0;
                            for(int k=0; k<dst_len; k++) {
                                int idx = (int)pos;
                                if(idx >= src_len) idx = src_len - 1;
                                ps.data_16[k] = original_mono[idx];
                                pos += step;
                            }
                        }
                    }
                    instruments.push_back(ps);
                    xm_inst_map[i][g] = xm_inst_counter;
                    xm_inst_counter++;
                }
            }
        }

        // =========================================================================
        // PASS 3: CHANNEL ALLOCATION
        // =========================================================================
        std::vector<NoteEvent> events;
        std::fill(unit_current_woice.begin(), unit_current_woice.end(), 0);

        for (const EVERECORD* e = evels->get_Records(); e != NULL; e = e->next) {
            if (e->unit_no >= unit_count) continue;
            
            if (e->kind == EVENTKIND_VOICENO) {
                unit_current_woice[e->unit_no] = e->value;
            }
            else if (e->kind == EVENTKIND_ON) {
                int start_r = e->clock / ticks_per_row;
                int dur_r = e->value / ticks_per_row;
                if (dur_r < 1) dur_r = 1;
                int end_r = start_r + dur_r - 1;

                NoteEvent ne;
                ne.unit_id = e->unit_no;
                ne.start_row = start_r;
                ne.end_row = end_r;
                int w = unit_current_woice[e->unit_no];
                ne.priority = (w < woice_count && v_stats[w].is_sample) ? 1 : 0;
                events.push_back(ne);
            }
        }
        std::sort(events.begin(), events.end(), compare_notes);

        std::vector<std::vector<int>> alloc_layout(MAX_XM_CHANNELS, std::vector<int>(total_rows, -1));
        std::vector<std::vector<bool>> is_attack(MAX_XM_CHANNELS, std::vector<bool>(total_rows, false));

        for (const auto& evt : events) {
            int best_ch = -1;
            for (int ch = 0; ch < MAX_XM_CHANNELS; ch++) {
                bool collision = false;
                for (int r = evt.start_row; r <= evt.end_row; r++) {
                    if (r < total_rows && alloc_layout[ch][r] != -1) { collision = true; break; }
                }
                if (!collision) { best_ch = ch; break; }
            }
            if (best_ch == -1) {
                for (int ch = 0; ch < MAX_XM_CHANNELS; ch++) {
                    if (evt.start_row < total_rows && !alloc_layout[ch][evt.start_row]) { best_ch = ch; break; }
                }
            }
            if (best_ch != -1) {
                for (int r = evt.start_row; r <= evt.end_row; r++) {
                    if (r >= total_rows) break;
                    if (r > evt.start_row && is_attack[best_ch][r]) break;
                    alloc_layout[best_ch][r] = evt.unit_id;
                    is_attack[best_ch][r] = (r == evt.start_row);
                }
            }
        }

        // =========================================================================
        // PASS 4: FILL GRID
        // =========================================================================
        int num_patterns = (total_rows / 64) + 1;
        std::vector<std::vector<std::vector<GridCell>>> patterns(
            num_patterns, 
            std::vector<std::vector<GridCell>>(64, std::vector<GridCell>(MAX_XM_CHANNELS))
        );

        std::vector<int> u_key(unit_count, 0x6000);
        std::vector<int> u_woice(unit_count, 0);
        std::vector<int> u_vel(unit_count, 128);
        std::vector<int> u_vol(unit_count, 128);
        
        const EVERECORD* e = evels->get_Records();
        
        for (int r_abs = 0; r_abs < total_rows; r_abs++) {
            int p_idx = r_abs / 64;
            int r_idx = r_abs % 64;
            if (p_idx >= num_patterns) break;

            int row_start_clock = r_abs * ticks_per_row;
            int row_end_clock = (r_abs + 1) * ticks_per_row;

            while (e != NULL && e->clock < row_end_clock) {
                int u = e->unit_no;
                if (u < unit_count) {
                    bool vol_changed = false; 

                    switch(e->kind) {
                        case EVENTKIND_VOICENO: u_woice[u] = e->value; break;
                        case EVENTKIND_KEY: u_key[u] = e->value; break;
                        case EVENTKIND_VELOCITY: 
                            u_vel[u] = e->value; 
                            vol_changed = true; 
                            break;
                        case EVENTKIND_VOLUME: 
                            u_vol[u] = e->value; 
                            vol_changed = true; 
                            break;
                        case EVENTKIND_PAN_VOLUME: break; 
                    }
                    
                    for(int ch=0; ch<MAX_XM_CHANNELS; ch++) {
                        if (alloc_layout[ch][r_abs] == u) {
                             GridCell& cell = patterns[p_idx][r_idx][ch];
                            if (vol_changed) {
                                cell.volume = calc_xm_volume(u_vol[u], u_vel[u]);
                            }
                            if (e->kind == EVENTKIND_PAN_VOLUME) {
                                cell.effect = 0x08;
                                cell.param = (uint8_t)(e->value * 2);
                                if (cell.param > 255) cell.param = 255;
                            }
                        }
                    }
                }
                e = e->next;
            }

            // Fallback logic for Note placement
            for(int ch=0; ch<MAX_XM_CHANNELS; ch++) {
                if (is_attack[ch][r_abs]) {
                    GridCell& cell = patterns[p_idx][r_idx][ch];
                    if (cell.note == 0) { 
                        int u = alloc_layout[ch][r_abs];
                        if (u != -1) {
                            uint8_t n = pxtn_key_to_xm(u_key[u]);
                            int oct = (n - 1) / 12; 
                            if (oct < 0) oct = 0; 
                            if (oct > 9) oct = 9; // Cap expanded
                            int grp = oct / 2;
                            if (grp > 4) grp = 4;

                            int w = u_woice[u];
                            if (w < woice_count) {
                                int xm_id = xm_inst_map[w][grp];
                                int used_grp = grp;
                                
                                // Neighbor search expanded to 5
                                if(xm_id == 0) { 
                                    int best_dist = 100;
                                    int best_grp = -1;
                                    for(int k=0; k<5; k++) {
                                        if (xm_inst_map[w][k] != 0) {
                                            int dist = abs(k - grp);
                                            if (dist < best_dist) { best_dist = dist; best_grp = k; }
                                        }
                                    }
                                    if (best_grp != -1) { xm_id = xm_inst_map[w][best_grp]; used_grp = best_grp; }
                                }

                                if (xm_id != 0) {
                                    cell.instrument = (uint8_t)xm_id;
                                    int correction = (used_grp - grp) * 12;
                                    int final_note = n + correction;
                                    if (final_note < 1) final_note = 1; 
                                    // XM Note limit is 96, but some trackers support more. 
                                    // We cap at 120 to support the range requested.
                                    if (final_note > 120) final_note = 120; 
                                    cell.note = (uint8_t)final_note;
                                    cell.volume = calc_xm_volume(u_vol[u], u_vel[u]);
                                }
                            }
                        }
                    }
                }
                
                int curr_u = alloc_layout[ch][r_abs];
                int prev_u = (r_abs > 0) ? alloc_layout[ch][r_abs-1] : -1;
                if (prev_u != -1 && curr_u == -1) {
                     GridCell& cell = patterns[p_idx][r_idx][ch];
                     if (cell.note == 0) cell.note = 97; 
                }
            }
        }

        write_xm_file(out_filename, num_patterns, MAX_XM_CHANNELS, instruments, patterns, beat_tempo/2);
    }

private:
    void parse_envelope(const pxtnVOICEUNIT* unit, ParsedSample& ps) {
        bool loops = (unit->voice_flags & PXTN_VOICEFLAG_WAVELOOP);
        bool has_env = (unit->voice_flags & PXTN_VOICEFLAG_ENVELOPE);
        
        int time_scale_div = 4; 

        if ((unit->envelope.head_num > 0 || unit->envelope.body_num > 0) || has_env) {
            ps.env_enabled = true;
            int cur_x = 0;
            int count = unit->envelope.head_num + unit->envelope.body_num + unit->envelope.tail_num;
            if (count > 12) count = 12; 

            if (count == 0 && has_env) {
                 ps.env_points.push_back(0); ps.env_points.push_back(64);
                 ps.env_points.push_back(10); ps.env_points.push_back(0);
                 ps.env_sustain = 0;
            } else {
                for (int k = 0; k < count; k++) {
                    cur_x += unit->envelope.points[k].x;
                    int x_xm = cur_x / time_scale_div; 
                    int y = unit->envelope.points[k].y / 2;
                    if (y > 64) y = 64;
                    
                    ps.env_points.push_back((uint16_t)x_xm);
                    ps.env_points.push_back((uint16_t)y);
                }
                
                if (unit->envelope.body_num > 0) {
                     ps.env_sustain = unit->envelope.head_num + unit->envelope.body_num - 1;
                } else {
                     ps.env_sustain = (ps.env_points.size()/2) - 1;
                }
                if(ps.env_sustain < 0) ps.env_sustain = 0;
            }
        } 
        else if (loops) {
            ps.env_enabled = true;
            ps.env_points.push_back(0); ps.env_points.push_back(64); 
            ps.env_points.push_back(1); ps.env_points.push_back(64); 
            ps.env_points.push_back(2); ps.env_points.push_back(0);  
            ps.env_sustain = 1; 
        }
    }

    void write_xm_file(const char* fname, int num_patterns, int num_channels, 
                       const std::vector<ParsedSample>& instruments, 
                       const std::vector<std::vector<std::vector<GridCell>>>& patterns,
                       float tempo) 
    {
        FILE* f = fopen(fname, "wb");
        if (!f) return;

        XMHeader xmh = {0};
        memcpy(xmh.id, "Extended Module: ", 17);
        memcpy(xmh.moduleName, "Pxtone Export", 13);
        xmh.eof = 0x1A;
        memcpy(xmh.trackerName, "PxtoneToXM 2.6", 14);
        xmh.version = 0x0104;
        xmh.headerSize = 0x114;
        xmh.songLength = num_patterns > 256 ? 256 : num_patterns;
        xmh.restartPosition = 0;
        xmh.channels = num_channels;
        xmh.patterns = num_patterns;
        xmh.instruments = instruments.size(); 
        xmh.flags = 1; 
        xmh.tempo = 3; 
        xmh.bpm = (uint16_t)(tempo * 2); 
        
        for (int i = 0; i < xmh.songLength; i++) xmh.patternOrder[i] = i;

        fwrite(&xmh, sizeof(xmh), 1, f);

        for (int p = 0; p < num_patterns; p++) {
            uint32_t len=9; uint8_t typ=0; uint16_t rows=64; uint16_t size=0;
            std::vector<uint8_t> dat;
            for (int r = 0; r < 64; r++) {
                for (int c = 0; c < num_channels; c++) {
                    const GridCell& gc = patterns[p][r][c];
                    uint8_t mask = 0;
                    if (gc.note) mask |= 1;
                    if (gc.instrument) mask |= 2;
                    if (gc.volume) mask |= 4;
                    if (gc.effect) mask |= 24;
                    if (mask) {
                        dat.push_back(0x80 | mask);
                        if (mask & 1) dat.push_back(gc.note);
                        if (mask & 2) dat.push_back(gc.instrument);
                        if (mask & 4) dat.push_back(gc.volume);
                        if (mask & 24) { dat.push_back(gc.effect); dat.push_back(gc.param); }
                    } else dat.push_back(0x80);
                }
            }
            size = (uint16_t)dat.size();
            fwrite(&len, 4, 1, f); fwrite(&typ, 1, 1, f); fwrite(&rows, 2, 1, f); fwrite(&size, 2, 1, f);
            fwrite(dat.data(), 1, size, f);
        }

        for (const auto& inst : instruments) {
            XMInstrumentHeader xih = {0};
            bool has_sample = !inst.data_16.empty();
            xih.size = has_sample ? 263 : 29;
            char name_buf[23];
            int s_oct = inst.oct_group * 2;
            snprintf(name_buf, 22, "%.15s-O%d", inst.name.c_str(), s_oct);
            memcpy(xih.name, name_buf, 22);
            xih.numSamples = has_sample ? 1 : 0;
            if (has_sample) {
                xih.sampleHeaderSize = 40;
                if (inst.env_enabled) {
                    xih.volType = 1; xih.volPoints = inst.env_points.size() / 2;
                    xih.volSustain = inst.env_sustain; xih.volType |= 2; 
                    for (size_t k = 0; k < inst.env_points.size(); k++) {
                        uint16_t val = inst.env_points[k];
                        xih.volEnv[k*2] = val & 0xFF; xih.volEnv[k*2+1] = (val >> 8) & 0xFF;
                    }
                }
            }
            fwrite(&xih, xih.size, 1, f);
            if (has_sample) {
                XMSampleHeader xsh = {0};
                xsh.length = inst.data_16.size() * 2; xsh.type = 16; 
                if (inst.flags & PXTN_VOICEFLAG_WAVELOOP) { xsh.type |= 1; xsh.loopLength = xsh.length; }
                xsh.volume = 64; xsh.panning = 128;
                double rate_offset = 12.0 * log2(44100.0 / 8363.0) - 12.0; 
                double pxtn_offset = (24576.0 - (double)inst.basic_key) / 256.0;
                double resize_shift = (double)oct_wave[inst.oct_group].shift;
                double total_semitones = rate_offset + pxtn_offset - 15 - resize_shift;
                xsh.relNote = (int8_t)round(total_semitones);
                double fine_val = (total_semitones - xsh.relNote) * 128.0;
                if (fine_val > 127) fine_val = 127; if (fine_val < -128) fine_val = -128;
                xsh.finetune = (int8_t)fine_val;
                strncpy((char*)xsh.name, name_buf, 22);
                fwrite(&xsh, 40, 1, f);
                std::vector<int16_t> final_data = inst.data_16;
                int16_t old = 0;
                for (size_t k = 0; k < final_data.size(); k++) {
                    int16_t val = final_data[k]; final_data[k] = val - old; old = val;
                }
                fwrite(final_data.data(), 1, final_data.size() * 2, f);
            }
        }
        fclose(f);
        printf("[SUCCESS] XM exported to %s\n", fname);
    }
};

int main(int argc, char** argv) {
    if (argc < 3) { printf("Usage: ptcop2xm <in.ptcop> <out.xm>\n"); return 1; }
    PxtoneToXM converter;
    if (converter.load_and_init(argv[1])) {
        converter.export_xm(argv[2]);
    } else {
        printf("[ERROR] Failed to load pxtone file.\n");
        return 1;
    }
    return 0;
}