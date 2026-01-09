/*
* ptcop2xm (Dynamic Smart Resampling)
* 
* Logic:
* - Scans song to find active note ranges for every voice.
* - "Single Mode": If usage range <= 1 Octave, generate 1 High-Quality Instrument (No Resample).
* - "Multi Mode": If usage range > 1 Octave, generate split instruments for active groups only.
* - Groups: 0 (Oct 0-1), 1 (Oct 2-3), 2 (Oct 4-5), 3 (Oct 6-7).
* - Calculates correct mapping so pattern data points to the generated XM IDs.
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

// --- Resampling Logic Structures ---
typedef struct
{
    short divisor;   // Downsampling factor
    short shift;     // Pitch shift correction
} OCTWAVE;

// Table: 4 Entries (Covering 2 Octaves each)
OCTWAVE oct_wave[4] =
{
    { 1,  0 }, // Group 0: Oct 0-1 (Full Size)
    { 2, 12 }, // Group 1: Oct 2-3 (Half Size)
    { 4, 24 }, // Group 2: Oct 4-5 (Quarter Size)
    { 8, 36 }  // Group 3: Oct 6-7 (Eighth Size)
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
// INTERMEDIATE DATA STRUCTURES
// ====================================================================================

struct ParsedSample {
    std::string name;
    int pxtn_id;
    int oct_group; // 0-3
    std::vector<int16_t> data_16; 
    
    int32_t basic_key;
    uint32_t flags;
    
    // Envelope
    bool env_enabled;
    std::vector<uint16_t> env_points; 
    int env_sustain;
};

struct GridCell {
    uint8_t note = 0;       // 1-96, 97=Off
    uint8_t instrument = 0; // 1-128
    uint8_t volume = 0;     // 0x10 - 0x50 (XM format)
    uint8_t effect = 0;
    uint8_t param = 0;
};

// Analysis Struct
struct VoiceStats {
    int32_t min_key;
    int32_t max_key;
    bool used;
    bool group_used[4]; // Tracks usage of specific octave groups
    
    VoiceStats() {
        min_key = 2147483647;
        max_key = -2147483647;
        used = false;
        for(int i=0; i<4; i++) group_used[i] = false;
    }
};

// ====================================================================================
// CONVERTER LOGIC
// ====================================================================================

uint8_t pxtn_key_to_xm(int32_t key) {
    double semitones = key / 256.0;
    int note = (int)(semitones) - 47; 
    if (note < 1) note = 1;
    if (note > 96) note = 96;
    return (uint8_t)note;
}

uint8_t calc_xm_volume(int vol, int vel) {
    int combined = vol * vel;
    int xm_vol = combined / 256; 
    if (xm_vol > 64) xm_vol = 64;
    if (xm_vol < 0) xm_vol = 0;
    return (uint8_t)(xm_vol + 0x10);
}

class PxtoneToXM {
    pxtnService* pxtn;
    std::vector<uint8_t> file_mem;

public:
    PxtoneToXM() {
        pxtn = new pxtnService();
    }

    ~PxtoneToXM() {
        if (pxtn) delete pxtn;
    }

    void patch_memory() {
        const char* target = "mateOGGV";
        const char* replace = "textCOMM";
        if (file_mem.size() < 8) return;
        for (size_t i = 0; i < file_mem.size() - 8; i++) {
            if (memcmp(&file_mem[i], target, 8) == 0) {
                memcpy(&file_mem[i], replace, 8);
            }
        }
    }

    bool load_and_init(const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (!f) return false;
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        file_mem.resize(size);
        fread(file_mem.data(), 1, size, f);
        fclose(f);
        patch_memory();

        if (pxtn->init() != pxtnOK) return false;
        pxtn->set_destination_quality(2, 44100);
        pxtnDescriptor desc;
        if (!desc.set_memory_r(file_mem.data(), size)) return false;
        if (pxtn->read(&desc) != pxtnOK) return false;
        if (pxtn->tones_ready() != pxtnOK) return false;
        return true;
    }

    void export_xm(const char* out_filename) {
        int woice_count = pxtn->Woice_Num();
        int num_channels = pxtn->Unit_Num();
        const pxtnEvelist* evels = pxtn->evels;

        // =========================================================================
        // PASS 1: ANALYZE USAGE
        // =========================================================================
        std::vector<VoiceStats> v_stats(woice_count);
        std::vector<int> unit_current_woice(num_channels, 0);

        // Pre-scan events
        for (const EVERECORD* e = evels->get_Records(); e != NULL; e = e->next) {
            if (e->unit_no >= num_channels) continue;
            
            if (e->kind == EVENTKIND_VOICENO) {
                unit_current_woice[e->unit_no] = e->value;
            }
            else if (e->kind == EVENTKIND_KEY) {
                int w = unit_current_woice[e->unit_no];
                if (w < woice_count) {
                    v_stats[w].used = true;
                    if (e->value < v_stats[w].min_key) v_stats[w].min_key = e->value;
                    if (e->value > v_stats[w].max_key) v_stats[w].max_key = e->value;
                    
                    // Determine Group (1-96 in XM)
                    uint8_t note = pxtn_key_to_xm(e->value);
                    int oct = (note - 1) / 12;
                    if (oct < 0) oct = 0;
                    if (oct > 7) oct = 7;
                    int group = oct / 2;
                    v_stats[w].group_used[group] = true;
                }
            }
        }

        // =========================================================================
        // PASS 2: GENERATE INSTRUMENTS
        // =========================================================================
        std::vector<ParsedSample> instruments;
        
        // Map [PxtoneID][OctGroup] -> XM Instrument Index (1-based)
        // Note: For Single Mode, all groups map to the same index.
        std::vector<std::vector<int>> xm_inst_map(woice_count, std::vector<int>(4, 0));
        
        int xm_inst_counter = 1;

        for (int i = 0; i < woice_count; i++) {
            if (!v_stats[i].used) continue; // Skip unused voices entirely

            const pxtnWoice* woice = pxtn->Woice_Get(i);
            
            // Logic: Single Mode vs Multi Mode
            int range = v_stats[i].max_key - v_stats[i].min_key;
            bool single_mode = (range <= 3072); // <= 1 Octave (12 semitones * 256)

            int name_len = 0;
            std::string base_name;
            const char* name_ptr = woice->get_name_buf(&name_len);
            if (name_len > 0) base_name = std::string(name_ptr, name_len);
            else base_name = "Inst " + std::to_string(i);

            // Extract Raw PCM
            const pxtnVOICEUNIT* unit = woice->get_voice(0);
            const pxtnVOICEINSTANCE* inst = woice->get_instance(0);
            
            std::vector<int16_t> original_mono;
            bool has_sample = false;
            
            if (unit && inst && inst->p_smp_w) {
                has_sample = true;
                int32_t len = inst->smp_body_w; 
                original_mono.resize(len);
                int16_t* raw = (int16_t*)inst->p_smp_w;
                for(int k=0; k<len; k++) {
                     original_mono[k] = (raw[k*2] + raw[k*2+1]) / 2;
                }
            }

            if (single_mode) {
                // --- SINGLE MODE: 1 Instrument, Full Quality ---
                ParsedSample ps;
                ps.pxtn_id = i;
                ps.oct_group = 0; // Nominal group 0, but acts as universal
                ps.name = base_name; // No suffix needed, or maybe "-HQ"
                ps.env_enabled = false;
                
                // Copy Envelope Logic
                if (has_sample) {
                    ps.basic_key = unit->basic_key;
                    ps.flags = unit->voice_flags;
                    ps.data_16 = original_mono; // Divisor 1

                    // Envelope parsing...
                    if (unit->envelope.head_num > 0 || unit->envelope.body_num > 0) {
                        ps.env_enabled = true;
                        int cur_x = 0;
                        int count = unit->envelope.head_num + unit->envelope.body_num + unit->envelope.tail_num;
                        if (count > 12) count = 12; 
                        for (int k = 0; k < count; k++) {
                            cur_x += unit->envelope.points[k].x;
                            int y = unit->envelope.points[k].y / 2;
                            if (y > 64) y = 64;
                            ps.env_points.push_back((uint16_t)cur_x);
                            ps.env_points.push_back((uint16_t)y);
                        }
                        if (unit->envelope.tail_num > 0)
                            ps.env_sustain = unit->envelope.head_num + unit->envelope.body_num - 1;
                        else
                            ps.env_sustain = (ps.env_points.size()/2) - 1;
                        if(ps.env_sustain < 0) ps.env_sustain = 0;
                    }
                }
                
                instruments.push_back(ps);
                
                // Map ALL groups to this single instrument
                for(int g=0; g<4; g++) xm_inst_map[i][g] = xm_inst_counter;
                xm_inst_counter++;

                printf("[INFO] Voice %d: Single Mode (Range %d). Mapped to ID %d.\n", i, range, xm_inst_counter-1);

            } else {
                // --- MULTI MODE: Dynamic Split ---
                // Only generate for used groups
                for(int g = 0; g < 4; g++) {
                    if (!v_stats[i].group_used[g]) continue; // Skip unused groups

                    ParsedSample ps;
                    ps.pxtn_id = i;
                    ps.oct_group = g;
                    ps.name = base_name;
                    ps.env_enabled = false;

                    if (has_sample) {
                        ps.basic_key = unit->basic_key;
                        ps.flags = unit->voice_flags;
                        
                        // Envelope parsing...
                        if (unit->envelope.head_num > 0 || unit->envelope.body_num > 0) {
                            ps.env_enabled = true;
                            int cur_x = 0;
                            int count = unit->envelope.head_num + unit->envelope.body_num + unit->envelope.tail_num;
                            if (count > 12) count = 12; 
                            for (int k = 0; k < count; k++) {
                                cur_x += unit->envelope.points[k].x;
                                int y = unit->envelope.points[k].y / 2;
                                if (y > 64) y = 64;
                                ps.env_points.push_back((uint16_t)cur_x);
                                ps.env_points.push_back((uint16_t)y);
                            }
                            if (unit->envelope.tail_num > 0)
                                ps.env_sustain = unit->envelope.head_num + unit->envelope.body_num - 1;
                            else
                                ps.env_sustain = (ps.env_points.size()/2) - 1;
                            if(ps.env_sustain < 0) ps.env_sustain = 0;
                        }

                        // Resampling
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
                    
                    // Map specific group to this instrument
                    xm_inst_map[i][g] = xm_inst_counter;
                    xm_inst_counter++;
                }
                printf("[INFO] Voice %d: Multi Mode (Range %d). Generated active groups.\n", i, range);
            }
        }

        // =========================================================================
        // PASS 3: BUILD GRID (Pattern Data)
        // =========================================================================
        
        int32_t beat_num, beat_clock, meas_num;
        float beat_tempo;
        pxtn->master->Get(&beat_num, &beat_tempo, &beat_clock, &meas_num);
        int rows_per_beat = 8;
        int ticks_per_row = beat_clock / rows_per_beat; 
        if (ticks_per_row < 1) ticks_per_row = 1;
        int32_t actual_end = pxtn->evels->get_Max_Clock();
        if (actual_end < pxtn->master->get_last_clock()) actual_end = pxtn->master->get_last_clock();

        int num_patterns = ((actual_end / ticks_per_row) / 64) + 1;
        
        std::vector<std::vector<std::vector<GridCell>>> patterns(
            num_patterns, 
            std::vector<std::vector<GridCell>>(64, std::vector<GridCell>(num_channels))
        );

        std::vector<int> unit_key(num_channels, 0x6000);
        std::vector<int> unit_woice(num_channels, 0);
        std::vector<int> unit_vel(num_channels, 128);
        std::vector<int> unit_vol(num_channels, 128);

        for (const EVERECORD* e = evels->get_Records(); e != NULL; e = e->next) {
            int u = e->unit_no;
            if (u >= num_channels) continue;

            int row_abs = e->clock / ticks_per_row;
            int pat_idx = row_abs / 64;
            int row_idx = row_abs % 64;

            if (pat_idx >= num_patterns) continue;
            GridCell& cell = patterns[pat_idx][row_idx][u];

            switch (e->kind) {
                case EVENTKIND_VOICENO: unit_woice[u] = e->value; break;
                case EVENTKIND_KEY: unit_key[u] = e->value; break;
                case EVENTKIND_VELOCITY: 
                    unit_vel[u] = e->value; 
                    cell.volume = calc_xm_volume(unit_vol[u], unit_vel[u]);
                    break;
                case EVENTKIND_VOLUME: 
                    unit_vol[u] = e->value; 
                    cell.volume = calc_xm_volume(unit_vol[u], unit_vel[u]);
                    break;

                case EVENTKIND_ON:
                {
                    uint8_t note = pxtn_key_to_xm(unit_key[u]);
                    cell.note = note;
                    
                    int oct = (note - 1) / 12;
                    if (oct < 0) oct = 0; if (oct > 7) oct = 7;
                    int group = oct / 2;

                    int pxtn_v = unit_woice[u];
                    if (pxtn_v < woice_count) {
                        // USE THE MAP to find the correct instrument ID
                        int xm_id = xm_inst_map[pxtn_v][group];
                        
                        // Fallback logic if map is 0 (shouldn't happen with correct pre-scan, 
                        // but maybe note is slightly out of bounds of detected range or unused group)
                        if (xm_id == 0) {
                            // Find nearest neighbor in map
                            for(int k=0; k<4; k++) if(xm_inst_map[pxtn_v][k] != 0) xm_id = xm_inst_map[pxtn_v][k];
                        }
                        
                        cell.instrument = (uint8_t)xm_id;
                    }
                    
                    cell.volume = calc_xm_volume(unit_vol[u], unit_vel[u]);

                    int duration = e->value / ticks_per_row;
                    int off_abs = row_abs + duration;
                    if (off_abs <= row_abs) off_abs = row_abs + 1; 
                    int off_pat = off_abs / 64; 
                    int off_row = off_abs % 64;
                    if (off_pat < num_patterns) {
                        GridCell& off_cell = patterns[off_pat][off_row][u];
                        if (off_cell.note == 0) off_cell.note = 97; 
                    }
                    break;
                }
                case EVENTKIND_PAN_VOLUME:
                    cell.effect = 0x08;
                    cell.param = (uint8_t)(e->value * 2); 
                    if (cell.param > 255) cell.param = 255;
                    break;
            }
        }

        // =========================================================================
        // PASS 4: WRITE FILE
        // =========================================================================
        FILE* f = fopen(out_filename, "wb");
        if (!f) return;

        XMHeader xmh = {0};
        memcpy(xmh.id, "Extended Module: ", 17);
        memcpy(xmh.moduleName, "Pxtone Export", 13);
        xmh.eof = 0x1A;
        memcpy(xmh.trackerName, "PxtoneLib", 9);
        xmh.version = 0x0104;
        xmh.headerSize = 0x114;
        xmh.songLength = num_patterns;
        xmh.restartPosition = 0;
        xmh.channels = num_channels;
        xmh.patterns = num_patterns;
        xmh.instruments = instruments.size(); // Actual generated count
        xmh.flags = 1; 
        xmh.tempo = 3; 
        xmh.bpm = (uint16_t)beat_tempo;
        for (int i = 0; i < num_patterns; i++) xmh.patternOrder[i] = i;
        fwrite(&xmh, sizeof(xmh), 1, f);

        for (int p = 0; p < num_patterns; p++) {
            uint32_t len=9; uint8_t typ=0; uint16_t rows=64; uint16_t size=0;
            std::vector<uint8_t> dat;
            for (int r = 0; r < 64; r++) {
                for (int c = 0; c < num_channels; c++) {
                    GridCell& gc = patterns[p][r][c];
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

        for (auto& inst : instruments) {
            XMInstrumentHeader xih = {0};
            bool has_sample = !inst.data_16.empty();
            xih.size = has_sample ? 263 : 29;
            
            // Name generation
            char name_buf[23];
            int s_oct = inst.oct_group * 2;
            
            // Check if Single Mode (how do we know? if data size == original size AND group==0 is ambiguous)
            // But we know group 0 divisor 1 maps to full size.
            // Let's use name logic:
            if (inst.data_16.size() > 0 && inst.oct_group == 0 && inst.data_16.size() >= 100 /*arbitrary small*/ ) {
                // If it was Single Mode, we just used name without suffix logic
                snprintf(name_buf, 22, "%.15s", inst.name.c_str());
            } else {
                snprintf(name_buf, 22, "%.14s-O%d%d", inst.name.c_str(), s_oct, s_oct+1);
            }
            memcpy(xih.name, name_buf, 22);

            xih.numSamples = has_sample ? 1 : 0;
            if (has_sample) {
                xih.sampleHeaderSize = 40;
                if (inst.env_enabled) {
                    xih.volType = 1;
                    xih.volPoints = inst.env_points.size() / 2;
                    xih.volSustain = inst.env_sustain;
                    if (xih.volSustain < xih.volPoints) xih.volType |= 2;
                    for (size_t k = 0; k < inst.env_points.size(); k++) {
                        uint16_t val = inst.env_points[k];
                        xih.volEnv[k*2] = val & 0xFF;
                        xih.volEnv[k*2+1] = (val >> 8) & 0xFF;
                    }
                }
            }
            fwrite(&xih, xih.size, 1, f);

            if (has_sample) {
                XMSampleHeader xsh = {0};
                xsh.length = inst.data_16.size() * 2; 
                xsh.type = 16; 
                if (inst.flags & 1) { xsh.type |= 1; xsh.loopLength = xsh.length; }
                xsh.volume = 64; xsh.panning = 128;
                
                // Pitch Calc
                double rate_offset = 12.0 * log2(44100.0 / 8363.0) - 12.0; 
                double pxtn_offset = (24576.0 - (double)inst.basic_key) / 256.0;
                
                // Divisor Logic Check
                // If this specific instrument used a divisor, apply shift
                int applied_divisor = oct_wave[inst.oct_group].divisor;
                // However, in SINGLE MODE, we forced Divisor 1 regardless of group?
                // Wait, in Single Mode we set `ps.oct_group = 0`. So `oct_wave[0].divisor` is 1. Correct.
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
                    int16_t val = final_data[k];
                    final_data[k] = val - old;
                    old = val;
                }
                fwrite(final_data.data(), 1, final_data.size() * 2, f);
            }
        }
        fclose(f);
        printf("[SUCCESS] XM exported to %s (Dynamic Instruments)\n", out_filename);
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