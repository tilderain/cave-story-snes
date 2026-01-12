/*#include "snesmod.h"
#include "audio_data.h"
//#include "res/soundbank.h"
extern unsigned char soundbank_Access;
extern unsigned char soundbank_Balcony;
extern unsigned char soundbank_BalrogTheme;
extern unsigned char soundbank_BreakDown;
extern unsigned char soundbank_CAVE;
extern unsigned char soundbank_CaveStory;
extern unsigned char soundbank_Charge;
extern unsigned char soundbank_Gameover;
extern unsigned char soundbank_Gestation;
extern unsigned char soundbank_GetHeartTank;
extern unsigned char soundbank_GotItem;
extern unsigned char soundbank_Gravity;
extern unsigned char soundbank_HeroEnd;
extern unsigned char soundbank_LastBattle;
extern unsigned char soundbank_LastCave;
extern unsigned char soundbank_Moonsong;
extern unsigned char soundbank_OntoGrasstown;
extern unsigned char soundbank_Oppression;
extern unsigned char soundbank_Pulse;
extern unsigned char soundbank_Quiet;
extern unsigned char soundbank_Run;
extern unsigned char soundbank_RunningHell;
extern unsigned char soundbank_ScorchingBack;
extern unsigned char soundbank_SealChamber;
extern unsigned char soundbank_TheWayBackHome;
extern unsigned char soundbank_Tyrant;
extern unsigned char soundbank_Victory;
extern unsigned char soundbank_VIVI;
extern unsigned char soundbank_WANPAK2;
extern unsigned char soundbank_whatislovee;
extern unsigned char soundbank_whatislove;
extern unsigned char soundbank_Zombie;
//extern unsigned char SOUNDBANK__; // The symbol from ASM
void play_music_track(MusicId id) {
    switch (id) {
        case MOD_ACCESS:          spcSetBank(&soundbank_Access); break;
        case MOD_BALCONY:        spcSetBank(&soundbank_Balcony); break;
        case MOD_BALROGTHEME:    spcSetBank(&soundbank_BalrogTheme); break;
        case MOD_BREAKDOWN:      spcSetBank(&soundbank_BreakDown); break;
        case MOD_CAVE:           spcSetBank(&soundbank_CAVE); break;
        case MOD_CAVESTORY:      spcSetBank(&soundbank_CaveStory); break;
        case MOD_CHARGE:         spcSetBank(&soundbank_Charge); break;
        case MOD_GAMEOVER:       spcSetBank(&soundbank_Gameover); break;
        case MOD_GESTATION:      spcSetBank(&soundbank_Gestation); break;
        case MOD_GETHEARTTANK:   spcSetBank(&soundbank_GetHeartTank); break;
        case MOD_GOTITEM:        spcSetBank(&soundbank_GotItem); break;
        case MOD_GRAVITY:        spcSetBank(&soundbank_Gravity); break;
        case MOD_HEROEND:        spcSetBank(&soundbank_HeroEnd); break;
        case MOD_LASTBATTLE:     spcSetBank(&soundbank_LastBattle); break;
        case MOD_LASTCAVE:       spcSetBank(&soundbank_LastCave); break;
        case MOD_MOONSONG:       spcSetBank(&soundbank_Moonsong); break;
        case MOD_ONTOGRASSTOWN:  spcSetBank(&soundbank_OntoGrasstown); break;
        case MOD_OPPRESSION:     spcSetBank(&soundbank_Oppression); break;
        case MOD_PULSE:          spcSetBank(&soundbank_Pulse); break;
        case MOD_QUIET:          spcSetBank(&soundbank_Quiet); break;
        case MOD_RUN:            spcSetBank(&soundbank_Run); break;
        case MOD_RUNNINGHELL:    spcSetBank(&soundbank_RunningHell); break;
        case MOD_SCORCHINGBACK:  spcSetBank(&soundbank_ScorchingBack); break;
        case MOD_SEALCHAMBER:    spcSetBank(&soundbank_SealChamber); break;
        case MOD_THEWAYBACKHOME: spcSetBank(&soundbank_TheWayBackHome); break;
        case MOD_TYRANT:         spcSetBank(&soundbank_Tyrant); break;
        case MOD_VICTORY:        spcSetBank(&soundbank_Victory); break;
        case MOD_VIVI:           spcSetBank(&soundbank_VIVI); break;
        case MOD_WANPAK2:        spcSetBank(&soundbank_WANPAK2); break;
        //case MOD_WHATISLOVEE:    spcSetBank(&soundbank_whatislovee); break;
        case MOD_WHATISLOVE:     spcSetBank(&soundbank_whatislove); break;
        case MOD_ZOMBIE:         spcSetBank(&soundbank_Zombie); break;
        default: return;
    }


    // Since each soundbank contains exactly one module, 
    // the index within the bank is always 0.
    spcLoad(0);
	    spcPlay(0);
    spcSetModuleVolume(100);
}*/
