// --- START OF CONVERTER SCRIPT ---

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

// Include the Pxtone headers (definitions must be available in all.txt or these files)
#include "pxtnService.h"
#include "pxtnError.h"

// ====================================================================================
// XM STRUCTURES (FastTracker 2 Format)
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
    std::vector<uint8_t> data;
    bool is_16bit;
    int channels;
    int sample_rate;
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

// ====================================================================================
// CONVERTER LOGIC
// ====================================================================================

uint8_t pxtn_key_to_xm(int32_t key) {
    double semitones = key / 256.0;
    // C4 (Middle C) in XM is Note 49. Pxtone 0x6000 (24576) is Middle C.
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
        // Prevent library from attempting OGG decode if dependencies are missing
        const char* target = "mateOGGV";
        const char* replace = "textCOMM";
        if (file_mem.size() < 8) return;
        for (size_t i = 0; i < file_mem.size() - 8; i++) {
            if (memcmp(&file_mem[i], target, 8) == 0) {
                memcpy(&file_mem[i], replace, 8);
                printf("[INFO] Patched OGG chunk at offset %zu.\n", i);
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

        pxtnERR res = pxtn->init();
        if (res != pxtnOK) { printf("Init Error: %d\n", res); return false; }

        pxtn->set_destination_quality(2, 44100);

        pxtnDescriptor desc;
        if (!desc.set_memory_r(file_mem.data(), size)) return false;

        res = pxtn->read(&desc);
        if (res != pxtnOK) { printf("Read Error: %d\n", res); return false; }

        res = pxtn->tones_ready();
        if (res != pxtnOK) { printf("Tones Ready Error: %d\n", res); return false; }

        return true;
    }

    void export_xm(const char* out_filename) {
        // --- 1. Export Instruments ---
        std::vector<ParsedSample> instruments;
        int woice_count = pxtn->Woice_Num();
        
        for (int i = 0; i < woice_count; i++) {
            const pxtnWoice* woice = pxtn->Woice_Get(i);
            ParsedSample ps;
            
            int name_len = 0;
            const char* name_ptr = woice->get_name_buf(&name_len);
            if (name_len > 0) ps.name = std::string(name_ptr, name_len);
            else ps.name = "Inst " + std::to_string(i);
            
            ps.pxtn_id = i;
            ps.env_enabled = false;

            const pxtnVOICEUNIT* unit = woice->get_voice(0);
            const pxtnVOICEINSTANCE* inst = woice->get_instance(0);

            if (unit && inst && inst->p_smp_w) {
                ps.basic_key = unit->basic_key;
                ps.flags = unit->voice_flags;
                ps.channels = 2;
                ps.sample_rate = 44100;
                ps.is_16bit = true;
                
                size_t data_size = inst->smp_body_w * 4; 
                ps.data.resize(data_size);
                memcpy(ps.data.data(), inst->p_smp_w, data_size);

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
        }

        // --- 2. Build Grid ---
        
        int32_t beat_num, beat_clock, meas_num;
        float beat_tempo;
        pxtn->master->Get(&beat_num, &beat_tempo, &beat_clock, &meas_num);
        
        // High Res: 8 rows per beat (32nd notes)
        int rows_per_beat = 8;
        int ticks_per_row = beat_clock / rows_per_beat; 
        if (ticks_per_row < 1) ticks_per_row = 1;

        // Fix Song Length: Check actual events
        int32_t header_end = pxtn->master->get_last_clock();
        int32_t events_end = pxtn->evels->get_Max_Clock();
        int32_t actual_end = (events_end > header_end) ? events_end : header_end;

        int total_rows = (actual_end / ticks_per_row) + 128; // Buffer
        int num_patterns = (total_rows / 64) + 1;
        
        printf("[INFO] Song Length: Header=%d, Events=%d -> Used=%d (%d Patterns)\n", header_end, events_end, actual_end, num_patterns);

        int num_channels = pxtn->Unit_Num();
        
        std::vector<std::vector<std::vector<GridCell>>> patterns(
            num_patterns, 
            std::vector<std::vector<GridCell>>(64, std::vector<GridCell>(num_channels))
        );

        std::vector<int> unit_key(num_channels, 0x6000);
        std::vector<int> unit_woice(num_channels, 0);
        std::vector<int> unit_vel(num_channels, 128);
        std::vector<int> unit_vol(num_channels, 128);

        const pxtnEvelist* evels = pxtn->evels;
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
                    cell.instrument = unit_woice[u] + 1;
                    cell.volume = calc_xm_volume(unit_vol[u], unit_vel[u]);

                    int duration_rows = e->value / ticks_per_row;
                    int off_abs = row_abs + duration_rows;
                    if (off_abs <= row_abs) off_abs = row_abs + 1; // Ensure duration

                    int off_pat = off_abs / 64;
                    int off_row = off_abs % 64;

                    if (off_pat < num_patterns) {
                        GridCell& off_cell = patterns[off_pat][off_row][u];
                        if (off_cell.note == 0) off_cell.note = 97; // Key Off
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

        // --- 3. Write XM File ---

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
        xmh.instruments = instruments.size();
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
                    } else {
                        dat.push_back(0x80);
                    }
                }
            }
            size = (uint16_t)dat.size();
            fwrite(&len, 4, 1, f); fwrite(&typ, 1, 1, f); fwrite(&rows, 2, 1, f); fwrite(&size, 2, 1, f);
            fwrite(dat.data(), 1, size, f);
        }

        for (auto& inst : instruments) {
            XMInstrumentHeader xih = {0};
            bool has_sample = !inst.data.empty();
            xih.size = has_sample ? 263 : 29;
            strncpy((char*)xih.name, inst.name.c_str(), 22);
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
                std::vector<int16_t> mono_data(inst.data.size() / 4);
                int16_t* src = (int16_t*)inst.data.data();
                for(size_t k=0; k<mono_data.size(); k++) mono_data[k] = (src[k*2] + src[k*2+1]) / 2;

                xsh.length = mono_data.size() * 2;
                xsh.type = 16;
                if (inst.flags & 1) { xsh.type |= 1; xsh.loopLength = xsh.length; }
                xsh.volume = 64; xsh.panning = 128;
                
                // --- PITCH CORRECTION ---
                // Base calculation for 44.1k sample on 8.3k engine = +28.78 semitones.
                // Adjusted down by 1 octave (-12 semitones) based on "Too High" feedback.
                // New offset ~16.78 semitones.
                double rate_offset = 12.0 * log2(44100.0 / 8363.0) - 12.0;
                
                double pxtn_offset = (24576.0 - (double)inst.basic_key) / 256.0;
                double total_semitones = rate_offset + pxtn_offset - 15;

                xsh.relNote = (int8_t)round(total_semitones);
                
                // Finetune calculation (-128 to 127)
                double fine_val = (total_semitones - xsh.relNote) * 128.0;
                if (fine_val > 127) fine_val = 127;
                if (fine_val < -128) fine_val = -128;
                xsh.finetune = (int8_t)fine_val;
                
                strncpy((char*)xsh.name, inst.name.c_str(), 22);
                fwrite(&xsh, 40, 1, f);

                int16_t old = 0;
                for (size_t k = 0; k < mono_data.size(); k++) {
                    int16_t val = mono_data[k];
                    mono_data[k] = val - old;
                    old = val;
                }
                fwrite(mono_data.data(), 1, mono_data.size() * 2, f);
            }
        }
        fclose(f);
        printf("[SUCCESS] XM exported to %s\n", out_filename);
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