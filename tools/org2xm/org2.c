/*
* Org2XM v2.2 (Nearest-Neighbor Resampling + Sustain Fix)
* Converts Org songs from Cave Story to XM modules.
* Public Domain.
* 
* todo resample octaves individually (at 256 len ?)
* Updates:
* - v2.2: Resampling now uses Nearest-Neighbor (Decimation) on PCM data. No filtering.
* - v2.1: Fixed Delta-decoding logic before resampling.
* - v2.0: Fixed lingering notes (Explicit Note Offs).
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PACKED __attribute__((packed))

#define read(to, bytes) fread(to, 1, bytes, f)
#define write(from, bytes) fwrite(from, 1, bytes, g)

#define VOL 51  // default volume
#define MAX_CHANNELS 8 
#define MAX_BARS 2048   // Safe limit

//////////////////////////////////////////////////////////////////////// Input
#pragma pack(push,1)
struct OrgHeader
{
    uint8_t magic[6];
    uint16_t msPerBeat;
    uint8_t measuresPerBar;
    uint8_t beatsPerMeasure;
    uint32_t loopStart;
    uint32_t loopEnd;
} PACKED header;
#pragma pack(pop)

struct Note
{
    uint32_t start;
    uint8_t len;
    uint8_t key;
    uint8_t vol;
    uint8_t pan;
} *note[16];

///////////////////////////////////////////////////// Immediate representation
#pragma pack(push,1)
struct Instrument
{
    uint16_t freqShift;
    uint8_t sample;
    uint8_t noLoop;
    uint16_t notes;

    uint8_t drum;
    uint8_t instrument;
    int8_t finetune;
    
    int8_t _pad1;
    uint8_t _pad2;
    uint8_t _pad3;
} PACKED t[16];
#pragma pack(pop)

struct ChannelState {
    int8_t lastPan;
    uint8_t lastVol;
    uint8_t played;
    int lastInputTrack; 
} chState[16];

struct Track
{
    float freq;
    uint8_t vol;
    int8_t pan;
} *n[16];

typedef struct {
    int track_idx;
    int start_row;
    int end_row;
    int duration;
    int is_drum;
} NoteEvent;

uint8_t *pat[MAX_BARS]; 
int patLen[MAX_BARS]; 

// Maps
int pattern_map[MAX_BARS];      // Bar Index -> XM Pattern ID
int source_bars[MAX_BARS];      // XM Pattern ID -> Original Bar Index

int instruments;
int tracks;
int out_channels = 0;
int *track_layout; 
uint8_t *layout_is_attack; 

int barLen;
int rows, bars;
int loop;

int compatibility;
int verorg;

/////////////////////////////////////////////////////////////////////// Output
#pragma pack(push,1)
struct XMHeader
{
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
} PACKED xmh = {
    "Extended Module: ", "", 0x1A, "Org2XM v2.2 NoFilter", 0x104, 0x114
};

struct XMInstrument
{
    uint32_t size;
    uint8_t instrumentName[22];
    uint8_t zero;
    uint16_t samples;
    uint32_t sampleHeaderSize;
    uint8_t misc[230];
    uint32_t sampleLength;
    uint32_t loopStart;
    uint32_t loopLength;
    uint8_t volume;
    int8_t finetune;
    uint8_t type;
    uint8_t panning;
    uint8_t relativeKey;
    uint8_t reserved;
    uint8_t sampleName[22];
} PACKED smp = {
    0x107, "Melody-00", 0, 1, 0x28, {}, 256, 0, 256, VOL, 0, 1, 128, 48, 0, ""
};
#pragma pack(pop)

/////////////////////////////////////////////////// Drums and drum accessories
struct SoundBank
{
    uint8_t magic[6];
    uint8_t verbank;
    uint8_t verorg;
    uint8_t snumMelo;
    uint8_t snumDrum;
    uint16_t lenMelo;
    uint32_t *tblLenDrum;
    char (*tblNameDrum)[22];
    int8_t *melody;
    int8_t *drums;
    uint32_t *tblOffDrum;
    uint32_t lenAllDrm;
} sbank;

int SBKload(char *path)
{
    FILE *f;
    uint32_t i, j;
    if (!(f = fopen(path, "rb"))) return 10;
    fseek(f , 0 , SEEK_END); rewind(f);
    read(&sbank.magic, 6);
    if (memcmp(sbank.magic, "ORGBNK", 6)) return 11;
    read(&sbank.verbank, 1); read(&sbank.verorg, 1);
    read(&sbank.snumMelo, 1); read(&sbank.snumDrum, 1);

    uint16_t tmp = 0, x = 0;
    read(&x, 1); tmp = (tmp << 8) + x; read(&x, 1); tmp = (tmp << 8) + x;
    sbank.lenMelo = tmp;

    sbank.tblLenDrum = malloc(sbank.snumDrum * sizeof(uint32_t));
    sbank.tblOffDrum = malloc(sbank.snumDrum * sizeof(uint32_t));
    sbank.lenAllDrm = 0;
    uint32_t off = 0;
    for(i = 0; i < sbank.snumDrum; i++) {
        read(&x, 1); tmp = (tmp << 8) + x; read(&x, 1); tmp = (tmp << 8) + x;
        read(&x, 1); tmp = (tmp << 8) + x; read(&x, 1); tmp = (tmp << 8) + x;
        sbank.tblLenDrum[i] = tmp; sbank.lenAllDrm += tmp;
        sbank.tblOffDrum[i] = off; off += tmp;
    }

    #define MAXSTR 23
    sbank.tblNameDrum = malloc(sbank.snumDrum * sizeof(char[MAXSTR]));
    for(i = 0; i < sbank.snumDrum; i++)
        for(j = 0; j < MAXSTR; j++) {
            char c = (char)fgetc(f); sbank.tblNameDrum[i][j] = j == MAXSTR-1 ? '\0' : c;
            if (c == '\0') break;
        }

    sbank.melody = malloc(sbank.snumMelo * sbank.lenMelo * 1);
    read(sbank.melody, sbank.snumMelo * sbank.lenMelo * 1);
    sbank.drums = malloc(sbank.lenAllDrm * 1);
    read(sbank.drums, sbank.lenAllDrm * 1);

    int8_t *buf;
    // CONVERT TO DELTA (XM FORMAT)
    for (i = 0 ; i < sbank.snumMelo; i++) {
        buf = &sbank.melody[i * sbank.lenMelo];
        for (j = sbank.lenMelo - 1; j > 0; --j) buf[j] -= buf[j - 1];
    }
    for (i = 0 ; i < sbank.snumDrum; i++) {
        buf = &sbank.drums[sbank.tblOffDrum[i]];
        for (j = sbank.tblLenDrum[i] - 1; j > 0; --j) buf[j] -= buf[j - 1];
        buf[0] ^= 0x80;
    }
    fclose(f); return 0;
}

int compare_notes(const void *a, const void *b) {
    NoteEvent *na = (NoteEvent *)a;
    NoteEvent *nb = (NoteEvent *)b;
    if (na->start_row != nb->start_row) return na->start_row - nb->start_row;
    if (na->is_drum != nb->is_drum) return nb->is_drum - na->is_drum;
    return na->track_idx - nb->track_idx;
}

////////////////////////////////////////////////////////// XM pattern encoding
int key, finetune;
uint8_t wKey, wInst, wVol, wFine, wPan, wPanVol, wSkip;

void encode(int i, int ch, int j)
{
    void resetPanVol(void) {
        wPan = (n[i][j].pan != (wInst ? 0 : chState[ch].lastPan));
        wVol = (n[i][j].vol != (wInst ? VOL : chState[ch].lastVol));
    }

    if (!j) {
        chState[ch].lastPan = chState[ch].lastVol = 0;
        chState[ch].lastInputTrack = -1;
    }
    
    if (j%barLen == 0) chState[ch].played = 0;

    wKey = wInst = wVol = wFine = wPan = wPanVol = 0;
    wSkip = (j==header.loopEnd-1 && ch==out_channels-1);

    if (j == header.loopStart && !t[i].noLoop) chState[ch].lastVol = -1;

    if (n[i][j].freq) {
        wKey = 1;
        finetune = (log2(n[i][j].freq/8363)*12 - t[i].finetune/128. + t[i].drum*36)*8 + .5;
        key = (finetune+4) / 8;
        finetune -= key*8;

        wInst = !chState[ch].played; 
        if (chState[ch].lastInputTrack != i) wInst = 1;
        if (compatibility) wInst = 1;
        
        wFine = !!finetune;
        resetPanVol();

        if (wPan && !n[i][j].pan) { wInst = 1; resetPanVol(); }
        if (wVol && n[i][j].vol==VOL && (n[i][j].pan!=chState[ch].lastPan || !n[i][j].pan)) { wInst = 1; resetPanVol(); }
        if (wPan && (wFine||wSkip) && !wVol) { wPan = 0; wPanVol = 1; }
    } else {
        if (n[i][j].vol != chState[ch].lastVol) wVol = 1;
        if (n[i][j].vol && n[i][j].pan != chState[ch].lastPan) wPan = 1;

        if (wVol && !n[i][j].vol) {
            wVol = wPan = wFine = 0;
            // Force Key Off for melody instruments to prevent infinite sustain
            if (!t[i].drum || !t[i].noLoop) { wKey = 1; key = 0x60; }
            chState[ch].lastVol = 0;
        }
    }

    if (wInst) {
        chState[ch].lastPan = 0; chState[ch].lastVol = VOL; 
        chState[ch].played = 1; chState[ch].lastInputTrack = i;
    }
    if (wVol) { wPanVol = 0; chState[ch].lastVol = n[i][j].vol; }
    if (wPanVol || wPan) chState[ch].lastPan = n[i][j].pan;
    if (wSkip) wPan = wFine = 0;
    if (wPan) wFine = 0;
}


///////////////////////////////////////////////////////////////////////// Main
int main(int argc, char** argv)
{
    int i, j, k;
    FILE *f, *g;

    if (argc < 2) return 1;
    if (argc > 3) compatibility = 1;

    int ret = SBKload(argc < 3 ? "ORG210EN.DAT" : argv[2]);
    if (ret) return ret;

    // --- RESAMPLING 256 -> 128 (Nearest Neighbor) ---
    // This reduces the sample size by half, allowing the SNES to play notes
    // one octave higher than before without hitting the 32kHz hardware limit.
    if (sbank.lenMelo == 256) {
        printf("Info: Resampling melody samples to 128 bytes (Nearest-Neighbor)...\n");
        for (i = 0; i < sbank.snumMelo; i++) {
            int8_t *buf = &sbank.melody[i * 256];
            
            // 1. Reconstruct PCM (Undo Delta from SBKload)
            // SBKload: buf[j] -= buf[j-1]. Inverse: buf[j] += buf[j-1]
            for (j = 1; j < 256; j++) buf[j] += buf[j - 1];

            // 2. Resample (Nearest Neighbor)
            // Just take every 2nd sample.
            // Since we write to i*128 and read from i*256, we never overwrite unread data.
            int8_t *dst = &sbank.melody[i * 128]; 
            for(j=0; j<128; j++) {
                dst[j] = buf[j*2]; 
            }

            // 3. Re-apply Delta Encoding (For XM)
            for (j = 127; j > 0; --j) dst[j] -= dst[j - 1];
        }
        sbank.lenMelo = 128;
    }

    if (!(f = fopen(argv[1], "rb"))) return 2;
    read(&header, sizeof(struct OrgHeader));

    if (memcmp(header.magic, "Org-02", 6)) verorg = 2;
    else if (memcmp(header.magic, "Org-03", 6)) { verorg = 3; if(sbank.verorg == 0xFF) return 6; } 
    else return 3;
    if(sbank.verorg < verorg) return 6;

    for (i=0; i<16; ++i) read(&t[i], 6);

    for (i=0; i<16; ++i) {
        note[i] = malloc(t[i].notes * sizeof(struct Note));
        if (i>=8) { t[i].sample += 100; t[i].noLoop = 1; t[i].drum = 1; } 
        else t[i].drum = 0;

        for (j=0; j<t[i].notes; ++j) read(&note[i][j].start, 4);
        for (j=0; j<t[i].notes; ++j) read(&note[i][j].key, 1);
        for (j=0; j<t[i].notes; ++j) read(&note[i][j].len, 1);
        for (j=0; j<t[i].notes; ++j) read(&note[i][j].vol, 1);
        for (j=0; j<t[i].notes; ++j) read(&note[i][j].pan, 1);

        for (j=0; j<t[i].notes; ++j)
        if (rows < note[i][j].start + note[i][j].len + 1)
        rows = note[i][j].start + note[i][j].len + 1;
    }

    barLen = header.measuresPerBar*header.beatsPerMeasure;
    if (header.loopStart < rows) { loop = 1; bars = header.loopEnd / barLen; } 
    else { loop = 0; bars = (rows+barLen-1) / barLen; }
    rows = bars*barLen;

    if (bars > MAX_BARS) {
        printf("Warning: Song too long (%d bars), truncating to %d bars.\n", bars, MAX_BARS);
        bars = MAX_BARS;
        rows = bars * barLen;
    }
    fclose(f);

    for (i=tracks=0; i<16; ++i) {
        if (!t[i].notes) continue;
        n[tracks] = malloc(rows * sizeof(struct Track));
        memset(n[tracks], 0, rows * sizeof(struct Track));

        for (j=0; j<t[i].notes; ++j) {
            k = note[i][j].start; if (k >= rows) continue;
            int vol = note[i][j].vol;
            int pan = note[i][j].pan;
            vol = (vol==0xff ? VOL : (vol/255.)*56.5+8.499);
            pan = (pan==0xff ? 0 : (pan-6)*127/6);

            if (note[i][j].key != 0xff) {
                n[tracks][k].freq = t[i].freqShift - 1000;
                if (t[i].drum) n[tracks][k].freq += 800 * note[i][j].key + 100;
                else n[tracks][k].freq += 8363 * pow(2, note[i][j].key/12.);
                if (t[i].noLoop) note[i][j].len = 16;
                do {
                    n[tracks][k].vol = vol;
                    n[tracks][k].pan = pan;
                } while (--note[i][j].len && ++k<rows && !(n[tracks][k].freq));
            } else {
                for ( ; n[tracks][k].vol && !(n[tracks][k].freq) && k<rows; ++k) {
                    if (note[i][j].vol != 0xff) n[tracks][k].vol = vol;
                    if (note[i][j].pan != 0xff) n[tracks][k].pan = pan;
                }
            }
            t[tracks] = t[i];
        }
        ++tracks;
        free(note[i]);
    }

    unsigned bestTempo=0, bestBPM=0, bestE=-1;
    for (i=1; i<32; ++i) {
        unsigned bpm = 2500*i/header.msPerBeat;
        if (bpm>31 && bpm<256) {
            int e = abs(2500000*i/bpm - header.msPerBeat*1000);
            if (bestE>e || (bestE==e && abs(bestBPM-125)>abs(bpm-125))) {
                bestTempo = i; bestBPM = bpm; bestE = e;
            }
        }
    }

    // --- ALLOCATION ---
    printf("Diagnostic: Gathering notes from %d tracks...\n", tracks);
    
    int est_notes = 0;
    for(i=0; i<tracks; i++) for(j=0; j<rows; j++) if(n[i][j].freq > 0) est_notes++;
    NoteEvent *evList = malloc(est_notes * sizeof(NoteEvent));
    int evCount = 0;

    for(i=0; i<tracks; i++) {
        int r = 0;
        while(r < rows) {
            if(n[i][r].freq > 0) {
                int start = r;
                int end = r;
                while (end < rows - 1) {
                    end++;
                    if (n[i][end].freq > 0) { end--; break; } 
                    if (n[i][end].vol == 0) { break; } 
                }
                evList[evCount].track_idx = i;
                evList[evCount].start_row = start;
                evList[evCount].end_row = end;
                evList[evCount].duration = end - start + 1;
                evList[evCount].is_drum = t[i].drum;
                evCount++;
                r = start + 1;
            } else {
                r++;
            }
        }
    }
    qsort(evList, evCount, sizeof(NoteEvent), compare_notes);

    track_layout = malloc(MAX_CHANNELS * rows * sizeof(int));
    layout_is_attack = malloc(MAX_CHANNELS * rows * sizeof(uint8_t));
    memset(track_layout, -1, MAX_CHANNELS * rows * sizeof(int));
    memset(layout_is_attack, 0, MAX_CHANNELS * rows * sizeof(uint8_t));
    
    out_channels = MAX_CHANNELS;

    int placed = 0, dropped = 0, stolen = 0;
    
    for(i=0; i<evCount; i++) {
        NoteEvent *e = &evList[i];
        int best_ch = -1;
        
        // 1. Perfect Fit
        for(int ch=0; ch<MAX_CHANNELS; ch++) {
            int collision = 0;
            for(int k=e->start_row; k <= e->end_row; k++) {
                if(track_layout[ch*rows + k] != -1) { collision = 1; break; }
            }
            if(!collision) { best_ch = ch; break; }
        }

        // 2. Sustain Stealing
        if (best_ch == -1) {
            for(int ch=0; ch<MAX_CHANNELS; ch++) {
                if (layout_is_attack[ch*rows + e->start_row]) continue;
                int safe_to_steal = 1;
                for (int k = e->start_row + 1; k <= e->end_row; k++) {
                    if (layout_is_attack[ch*rows + k]) { safe_to_steal = 0; break; }
                }
                if (safe_to_steal) {
                    best_ch = ch; stolen++; break;
                }
            }
        }

        // 3. Truncation Fitting
        if (best_ch == -1) {
            int max_len = 0;
            int best_trunc_ch = -1;
            
            for (int ch=0; ch<MAX_CHANNELS; ch++) {
                if (layout_is_attack[ch*rows + e->start_row]) continue;
                int valid_len = 0;
                for (int k = e->start_row; k <= e->end_row; k++) {
                    if (layout_is_attack[ch*rows + k]) break;
                    valid_len++;
                }
                if (valid_len > max_len) {
                    max_len = valid_len;
                    best_trunc_ch = ch;
                }
            }
            
            if (max_len > 0) {
                best_ch = best_trunc_ch;
                e->end_row = e->start_row + max_len - 1;
                stolen++;
            }
        }

        if(best_ch != -1) {
            placed++;
            for(int k=e->start_row; k <= e->end_row; k++) {
                track_layout[best_ch*rows + k] = e->track_idx;
                layout_is_attack[best_ch*rows + k] = (k == e->start_row) ? 1 : 0;
            }
        } else {
            dropped++;
        }
    }

    printf("Diagnostic: Allocation Complete\n");
    printf("  Placed:      %d (Stolen/Cut: %d)\n", placed, stolen);
    printf("  Dropped:     %d\n", dropped);

    free(evList);
    free(layout_is_attack);

    // --- FINETUNE ---
    for (i=0; i<tracks; ++i) {
        double bestE=1e30; uint8_t bestFinetune;
        for (k=-64; k<64; k++) {
            double e = 0; t[i].finetune = k;
            for (j=0; j<rows; j++) {
                encode(i, 0, j); 
                if (wKey && key!=0x60) {
                    if (!wFine) finetune = 0;
                    float logfreq = log2(8363) + (key + finetune/8. + t[i].finetune/128. - t[i].drum*36)/12.;
                    float d = log2(n[i][j].freq) - logfreq;
                    e += d*d + (finetune ? 1e-8 : 0);
                }
                if (e > bestE) goto nextk;
            }
            bestFinetune = k; bestE = e; nextk: continue;
        }
        t[i].finetune = bestFinetune;
    }

    for (i=0; i<tracks; ++i) t[i].instrument = i;
    for (i=0; i<tracks; ++i) if (t[i].instrument == i)
    for (j=i+1; j<tracks; ++j)
    if (t[j].sample == t[i].sample && t[j].finetune == t[i].finetune && t[j].noLoop == t[i].noLoop)
    t[j].instrument = i;

    for (instruments=0, i=0; i<tracks; ++i) {
        if (t[i].instrument == i) t[i].instrument = ++instruments;
        else t[i].instrument = t[t[i].instrument].instrument;
    }

    // --- PATTERNS ---
    memset(chState, 0, sizeof(chState));
    for(i=0;i<16;i++) chState[i].lastInputTrack = -1;

    for (k=0; k<bars; ++k) {
        int len;
        uint8_t *buf = pat[k] = malloc(5*barLen*out_channels+9);
        memset(buf, 0, 5*barLen*out_channels+9);
        *(uint32_t*)&buf[0] = len = 9;
        *(uint16_t*)&buf[5] = barLen;

        for (j=k*barLen; j<(k+1)*barLen; ++j) for (i=0; i<out_channels; ++i) {
            int input_trk = track_layout[i * rows + j];
            
            if (input_trk == -1) {
                // EXPLICIT CUT LOGIC: If we were playing something, kill it.
                if (chState[i].lastInputTrack != -1 && chState[i].lastVol > 0) {
                     int prev = chState[i].lastInputTrack;
                     if (!t[prev].drum || !t[prev].noLoop) {
                         wKey = 1; key = 0x60; // Key Off (Note 97)
                     } else {
                         wKey = 0;
                     }
                     // Reset state
                     chState[i].lastInputTrack = -1; 
                     chState[i].lastVol = 0;
                     chState[i].played = 0; 
                } else {
                    wKey = 0;
                }
                
                wInst = wVol = wFine = wPan = wPanVol = 0;
                wSkip = (j==header.loopEnd-1 && i==out_channels-1);
            } else {
                 encode(input_trk, i, j);
            }

            uint8_t p = 0x80 | wKey | wInst*2 | (wVol||wPanVol)*4 | (wPan||wSkip||wFine)*24;
            if (p != 0x9F) buf[len++] = p;
            if (wKey) buf[len++] = key+1;
            if (wInst) buf[len++] = t[input_trk].instrument;
            if (wVol) buf[len++] = 0x10 + n[input_trk][j].vol;
            else if (wPanVol) buf[len++] = 0xC0 + (n[input_trk][j].pan>0x77 ? 0xF : n[input_trk][j].pan+0x88>>4);

            if (wSkip) { buf[len++] = 0xB; buf[len++] = header.loopStart / barLen; }
            else if (wPan) { buf[len++] = 8; buf[len++] = n[input_trk][j].pan + 0x80; }
            else if (wFine) { buf[len++] = 0xE; buf[len++] = 0x58 + finetune; }
        }
        *(uint16_t*)&buf[7] = len-9;
        patLen[k] = len;
    }

    // Deduplication
    int unique_patterns = 0;
    
    for(i=0; i<MAX_BARS; i++) pattern_map[i] = -1;

    for(i=0; i<bars; i++) {
        int found = -1;
        for(int p=0; p<unique_patterns; p++) {
             int src = source_bars[p];
             if (patLen[i] == patLen[src] && !memcmp(pat[i], pat[src], patLen[i])) {
                 found = p;
                 break;
             }
        }
        
        if(found != -1) {
            pattern_map[i] = found;
        } else {
            pattern_map[i] = unique_patterns;
            source_bars[unique_patterns] = i;
            unique_patterns++;
        }
    }

    argv[1][strlen(argv[1])-3] = 'x'; argv[1][strlen(argv[1])-2] = 'm'; argv[1][strlen(argv[1])-1] = 0;
    if (!(g = fopen(argv[1], "wb"))) return 2;
    for (i=strlen(argv[1]); i>0 && argv[1][i-1]!='\\' && argv[1][i-1]!='/'; --i) ;
    argv[1][strlen(argv[1])-3] = 0;
    memcpy(xmh.moduleName, &argv[1][i], strlen(&argv[1][i])>20 ? 20 : strlen(&argv[1][i]));
    xmh.songLength = bars; xmh.restartPosition = header.loopStart / barLen;
    xmh.channels = out_channels; xmh.patterns = unique_patterns; xmh.instruments = instruments;
    xmh.flags = 1; xmh.tempo = bestTempo; xmh.bpm = bestBPM;
    
    memset(xmh.patternOrder, 0, 256);
    int orders_to_write = (bars > 256) ? 256 : bars;
    for(int o=0; o<orders_to_write; o++) {
        xmh.patternOrder[o] = pattern_map[o];
    }
    
    write(&xmh, sizeof(struct XMHeader));

    for (i=0; i<unique_patterns; ++i) {
        int src = source_bars[i];
        write(pat[src], patLen[src]);
    }

    for (k=1, i=0; i<tracks; ++i) if (t[i].instrument == k) {
        int8_t *sbuf;
        sprintf(smp.sampleName, "samples/%03d.wav", t[i].sample);
        smp.loopStart = 0; smp.finetune = t[i].finetune;
        memset(smp.instrumentName, 0, 22);
        if (t[i].drum) {
            uint8_t dsmp = t[i].sample - 100;
            sbuf = &sbank.drums[sbank.tblOffDrum[dsmp]];
            smp.type = 0; smp.loopLength = 0; smp.sampleLength = sbank.tblLenDrum[dsmp];
            strcpy(smp.instrumentName, sbank.tblNameDrum[dsmp]); smp.relativeKey = 12;
        } else {
            sbuf = &sbank.melody[t[i].sample * sbank.lenMelo];
            smp.type = 1; smp.loopLength = sbank.lenMelo; smp.sampleLength = sbank.lenMelo;
            sprintf(smp.instrumentName, t[i].freqShift==1000 ? "Melody%02d" : "Melody%02d %+d Hz", t[i].sample, t[i].freqShift-1000);
            
            // --- FIX FOR 128 BYTE SAMPLES ---
            // If lenMelo is 128 (1 octave higher native pitch), we set relativeKey to 36 (C-3)
            // instead of 48 (C-4) to compensate.
            if (sbank.lenMelo == 128) smp.relativeKey = 36;
            else smp.relativeKey = 48; // Standard 256 bytes
        }
        write(&smp, sizeof(struct XMInstrument)); write(sbuf, smp.sampleLength); ++k;
    }

    fclose(g);
    for (i=0; i<tracks; ++i) free(n[i]);
    for (k=0; k<bars; ++k) free(pat[k]);
    free(track_layout); 

    return 0;
}