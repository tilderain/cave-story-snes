#ifndef _SPC700_H
#define _SPC700_H

/* VBCC Type Definitions for SNES */
/* If you already have a types.h, include it instead of these typedefs */
typedef unsigned char u8;
typedef unsigned short u16;

/* Forward declaration of the brrsamples struct. 
   You will need the full definition of this struct elsewhere 
   in your code if you access its members. */
typedef struct brrsamples brrsamples;

/*! \fn  spcBoot(void)
    \brief boots the spc700 with sm-spc. call once at startup
    disable time consuing interrupts during this function
*/
void spcBoot(void);

/*! \fn  spcSetBank(u8 *bank)
    \brief set soundbank origin. soundbank must have dedicated bank(s)
    \param bank	bank address
*/
__pascal void spcSetBank(u8 *bank);

/*! \fn  spcLoad(u16 musIndex)
    \brief load module into sm-spc. this function may take some time to execute
    \param musIndex	module_id
*/
__pascal void spcLoad(u16 musIndex);

/*! \fn  spcLoadEffect(u16 sfxIndex)
    \brief load sound effect into sm-spc. this function may take some time to execute
    \param sfxIndex	sfx_id
*/
__pascal void spcLoadEffect(u16 sfxIndex);

/*! \fn  spcPlay(u8 startPos)
    \brief play module.
    note: this simply queues a message, use spcFlush if you want
    to wait until the message is processed

    another note: there may be a significant startup time from
    after the message is processed to when the song starts playing...
    to sync the program with the song start use spcFlush and then
    wait until SPC_P of the status register is set.
    \param startPos	starting position
*/
__pascal void spcPlay(u8 startPos);

/*! \fn  spcStop(void)
    \brief stop playing the current module.
*/
void spcStop(void);

/*! \fn  spcPauseMusic(void)
    \brief pause playing the current module and save the music position.
*/
void spcPauseMusic(void);

/*! \fn  spcResumeMusic(void)
    \brief Resume playing of current module at music position.

    spcPauseMusic has to be call before to restore correct position.
*/
void spcResumeMusic(void);

/*! \fn  spcSetModuleVolume(u8 vol)
    \brief set the module playback volume
    \param vol	volume (0..255)
*/
__pascal void spcSetModuleVolume(u8 vol);

/*! \fn  spcFadeModuleVolume(u16 vol, u16 fadespeed)
    \brief fade the module volume towards the target
    \param vol	volume (0..255)
    \param fadespeed	fade speed (volume(0..255) += y every 32ms)
*/
__pascal void spcFadeModuleVolume(u16 vol, u16 fadespeed);

/*! \fn  spcFlush(void)
    \brief Flush message queue (force sync)
*/
void spcFlush(void);

/*! \fn  spcProcess(void)
    \brief Process messages
    This function will try to give messages to the spc until a few
    scanlines pass

    this function MUST be called every frame if you are using
    streamed sounds
*/
void spcProcess(void);

/*! \fn  spcEffect(u16 pitch,u16 sfxIndex, u8 volpan)
    \brief Play sound effect (load with spcLoadEffect)
    \param pitch	pitch (0-15, 1=4Khz , 2=8khz, 4=16Khz, 8=32Khz)
    \param sfxIndex	effect index (0-15)
    \param volpan	volume(0..15) AND panning(0..15) (volume*16+pan)
*/
__pascal void spcEffect(u16 pitch, u16 sfxIndex, u8 volpan);

/*! \fn  spcGetMusicPosition(void)
    \brief Get current running pattern
    \return current pattern
*/
u8 spcGetMusicPosition(void);

/*! \fn  spcSetSoundTable(u16 sndTableAddr,u8 sndTableBank)
    \brief set the address of the SOUND TABLE
    \param sndTableAddr	address of sound table
    \param sndTableBank	bank of sound table
*/
__pascal void spcSetSoundTable(u16 sndTableAddr, u8 sndTableBank);

/*! \fn spcSetSoundEntry(u8 vol, u8 panning, u8 pitch, u16 length, u8 *sampleaddr, brrsamples *ptr)
    \brief set the values and address of the SOUND TABLE for a sound entry
    \param vol	volume (0..15)
    \param panning	panning (0..15)
    \param pitch	PITCH (1..6) (hz = PITCH * 2000)
    \param length	length of brr sample
    \param sampleaddr	address of brr sample
    \param ptr	address of variable where sounds values will be stored
*/
__pascal void spcSetSoundEntry(u8 vol, u8 panning, u8 pitch, u16 length, u8 *sampleaddr, brrsamples *ptr);

/*! \fn spcSetSoundDataEntry(u8 vol, u8 panning, u8 pitch, u16 length, u8 *sampleaddr, brrsamples *ptr)
    \brief set the values of a sound entry
    \param vol	volume (0..15)
    \param panning	panning (0..15)
    \param pitch	PITCH (1..6) (hz = PITCH * 2000)
    \param length	length of brr sample
    \param sampleaddr	address of brr sample
    \param ptr	address of variable where sounds values will be stored
*/
__pascal void spcSetSoundDataEntry(u8 vol, u8 panning, u8 pitch, u16 length, u8 *sampleaddr, brrsamples *ptr);

//---------------------------------------------------------------------------------
/*! \fn spcSetSoundTableEntry(brrsamples *ptr)
    \brief set the address of the SOUND TABLE for a sound entry
    \param ptr	address of variable where sounds values will be stored
*/
__pascal void spcSetSoundTableEntry(brrsamples *ptr);

/*! \fn  spcAllocateSoundRegion(u8 size);
    \brief Set the size of the sound region.

    Use it only if you are playing effects with BRR files. If not, just allocate the value 0 by calling spcAllocateSoundRegion(0);

    The value must be big enough to hold your longest/largest sound.
    This function will STOP module playback too.
    \param size	size of sound region (size*256 bytes)
*/
__pascal void spcAllocateSoundRegion(u8 size);

/*! \fn  spcPlaySound(u8 sndIndex)
    \brief Play sound from memory (using default arguments)
    \param sndIndex	index in sound table.
    Be careful: the index 0 corresponds to the LAST sound loaded.
    The index 1 is the penultimate sound loaded and so on...
*/
__pascal void spcPlaySound(u8 sndIndex);

/*! \fn  spcPlaySoundV(u8 sndIndex, u16 volume)
    \brief Play sound from memory (using default arguments)
    \param sndIndex	index in sound table.
    Be careful: the index 0 corresponds to the LAST sound loaded.
    The index 1 is the penultimate sound loaded and so on...
    \param volume	volume (0..15)
*/
__pascal void spcPlaySoundV(u8 sndIndex, u16 volume);

#endif /* _SPC700_H */
