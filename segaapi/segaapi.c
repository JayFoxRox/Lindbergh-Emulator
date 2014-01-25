#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <AL/al.h> 
#include <AL/alc.h> 
#include <AL/alext.h> 
#include <AL/alut.h>

//#include "eax4.h"
#include "segadef.h"
#include "segaerr.h"
#include "segaeax.h"

#include "segaapi.h"

const GUID EAX_NULL_GUID;
const GUID EAX_FREQUENCYSHIFTER_EFFECT;
const GUID EAX_ECHO_EFFECT;
const GUID EAX_REVERB_EFFECT;
const GUID EAX_EQUALIZER_EFFECT;
const GUID EAX_DISTORTION_EFFECT;
const GUID EAX_AGCCOMPRESSOR_EFFECT;
const GUID EAX_PITCHSHIFTER_EFFECT;
const GUID EAX_FLANGER_EFFECT;
const GUID EAX_VOCALMORPHER_EFFECT;
const GUID EAX_AUTOWAH_EFFECT;
const GUID EAX_RINGMODULATOR_EFFECT;
const GUID EAX_CHORUS_EFFECT;

const GUID EAXPROPERTYID_EAX40_FXSlot0;
const GUID EAXPROPERTYID_EAX40_FXSlot1;
const GUID EAXPROPERTYID_EAX40_FXSlot2;
const GUID EAXPROPERTYID_EAX40_FXSlot3;


//TODO: Status handling

typedef struct {
  void* userData;
  HAWOSEGABUFFERCALLBACK callback;
  bool synthesizer;
  bool loop;
  unsigned int channels;
  unsigned int startLoop;
  unsigned int endLoop;
  unsigned int endOffset;
  unsigned int sampleRate; 
  unsigned int sampleFormat;
  uint8_t* data;
  size_t size;
  ALuint alBuffer;
  ALuint alSource;
} segaapiBuffer_t;

PFNALBUFFERSUBDATASOFTPROC alBufferSubDataSOFT;
LPALBUFFERSAMPLESSOFT alBufferSamplesSOFT;
LPALGETBUFFERSAMPLESSOFT alGetBufferSamplesSOFT;
LPALBUFFERSUBSAMPLESSOFT alBufferSubSamplesSOFT;

#define LOG(...) //printf(__VA_ARGS__); // { FILE* f = fopen("segaapi.log","a"); fprintf(f,__VA_ARGS__); printf(__VA_ARGS__); fclose(f); }
#define CHECK() { unsigned int err; if ((err = alGetError()) != AL_NO_ERROR) { LOG(":%i AL Error: 0x%08X\n",__LINE__,err); exit(2); } }

static void dumpWaveBuffer(const char* path, unsigned int channels, unsigned int sampleRate, unsigned int sampleBits, void* data, size_t size) {

  struct RIFF_Header {
    char chunkID[4];
    long chunkSize;//size not including chunkSize or chunkID
    char format[4];
  };

  struct WAVE_Format {
    char subChunkID[4];
    long subChunkSize;
    short audioFormat;
    short numChannels;
    long sampleRate;
    long byteRate;
    short blockAlign;
    short bitsPerSample;
  };

  struct WAVE_Data {
    char subChunkID[4]; //should contain the word data
    long subChunk2Size; //Stores the size of the data block
  };

  //Local Declarations
  FILE* soundFile = NULL;
  struct WAVE_Format wave_format;
  struct RIFF_Header riff_header;
  struct WAVE_Data wave_data;
 

  soundFile = fopen(path, "wb");

  //check for RIFF and WAVE tag in memeory
  riff_header.chunkID[0] = 'R';
  riff_header.chunkID[1] = 'I';
  riff_header.chunkID[2] = 'F';
  riff_header.chunkID[3] = 'F';
  riff_header.format[0] = 'W';
  riff_header.format[1] = 'A';
  riff_header.format[2] = 'V';
  riff_header.format[3] = 'E';

  // Read in the first chunk into the struct
  fwrite(&riff_header, sizeof(struct RIFF_Header), 1, soundFile);

  //check for fmt tag in memory
  wave_format.subChunkID[0] = 'f';
  wave_format.subChunkID[1] = 'm';
  wave_format.subChunkID[2] = 't';
  wave_format.subChunkID[3] = ' ';

  wave_format.audioFormat = 1;
  wave_format.sampleRate = sampleRate;
  wave_format.numChannels = channels;
  wave_format.bitsPerSample = sampleBits;
  wave_format.byteRate = (sampleRate * sampleBits * channels) / 8;
  wave_format.blockAlign = (sampleBits * channels) / 8;
  wave_format.subChunkSize = 16;

  //Read in the 2nd chunk for the wave info
  fwrite(&wave_format, sizeof(struct WAVE_Format), 1, soundFile);

  //Read in the the last byte of data before the sound file

  //check for data tag in memory
  wave_data.subChunkID[0] = 'd';
  wave_data.subChunkID[1] = 'a';
  wave_data.subChunkID[2] = 't';
  wave_data.subChunkID[3] = 'a';

  wave_data.subChunk2Size = size;

  fwrite(&wave_data, sizeof(struct WAVE_Data), 1, soundFile);

  // Read in the sound data into the soundData variable
  fwrite(data, wave_data.subChunk2Size, 1, soundFile);

  //clean up and return true if successful
  fclose(soundFile);

  return;
}

static unsigned int bufferSampleSize(segaapiBuffer_t* buffer) {
  return buffer->channels * ((buffer->sampleFormat == HASF_SIGNED_16PCM)?2:1);
}

static void updateBufferLoop(segaapiBuffer_t* buffer) {
  unsigned int sampleSize = bufferSampleSize(buffer);
//  alSourcei(buffer->alSource, AL_BUFFER, AL_NONE);
//  CHECK();
/*
FIXME: Re-enable, only crashed before - so fix this too..
  ALint loopPoints[] = { buffer->startLoop / sampleSize, buffer->endLoop / sampleSize };
  alBufferiv(buffer->alBuffer,AL_LOOP_POINTS_SOFT,loopPoints);
  CHECK();
*/
  return;
}

static void updateBufferData(segaapiBuffer_t* buffer, unsigned int offset, size_t length) {

  ALenum alFormat = -1;
  ALenum alChannels = -1;
  ALenum alType;
  if (buffer->sampleFormat == HASF_UNSIGNED_8PCM) { /* Unsigned (offset 128) 8-bit PCM */
    alType = AL_BYTE_SOFT;    
    if (buffer->channels == 1) { alFormat = AL_MONO8_SOFT; alChannels = AL_MONO_SOFT; }
    if (buffer->channels == 2) { alFormat = AL_STEREO8_SOFT; alChannels = AL_STEREO_SOFT; }
  }
  if (buffer->sampleFormat == HASF_SIGNED_16PCM) { /* Signed 16-bit PCM */
    alType = AL_SHORT_SOFT;
    if (buffer->channels == 1) { alFormat = AL_MONO16_SOFT; alChannels = AL_MONO_SOFT; }
    if (buffer->channels == 2) { alFormat = AL_STEREO16_SOFT; alChannels = AL_STEREO_SOFT; }
  }
  if (alFormat == -1) {
    LOG("Unknown format! 0x%X with %u channels!\n",buffer->sampleFormat,buffer->channels);
  }


  ALint position;
  alGetSourcei(buffer->alSource, AL_SAMPLE_OFFSET, &position); //TODO: Patch if looping is active
  CHECK();
  ALint unsafe[2];
  alGetSourceiv(buffer->alSource, AL_BYTE_RW_OFFSETS_SOFT, unsafe);
  CHECK();
  if (offset != -1) {
    alBufferSubSamplesSOFT(buffer->alBuffer, offset / bufferSampleSize(buffer), length / bufferSampleSize(buffer), alChannels, alType, &buffer->data[offset]);
    CHECK();
    LOG("Soft update in buffer %X at %u (%u bytes) - buffer playing at %u, unsafe region is %u to %u\n",(uintptr_t)buffer,offset,length,position,unsafe[0],unsafe[1]);
  } else {
  	alSourcei(buffer->alSource, AL_BUFFER, AL_NONE);
    CHECK();
    alBufferSamplesSOFT(buffer->alBuffer, buffer->sampleRate, alFormat, buffer->size / bufferSampleSize(buffer), alChannels, alType, buffer->data);
    CHECK();
  	alSourcei(buffer->alSource, AL_BUFFER, buffer->alBuffer);
    CHECK();
    updateBufferLoop(buffer);
    LOG("Hard update in buffer %X (%u bytes) - buffer playing at %u, unsafe region is %u to %u\n",(uintptr_t)buffer,buffer->size,position,unsafe[0],unsafe[1]);
  }

/*

  // This dumps the buffer

  uint8_t* nonZero = buffer->data;
  while(*nonZero++ == 0x00);
  if ((nonZero-(uint8_t*)buffer->data) < buffer->size) {
    char buf[1000];
    sprintf(buf,"%X-%i-%04X-%u.wav",(uintptr_t)buffer,buffer->channels,buffer->sampleFormat,buffer->sampleRate);

    void* tmp = malloc(buffer->size);
    alGetBufferSamplesSOFT(buffer->alBuffer, 0, buffer->size / bufferSampleSize(buffer), alChannels, alType, tmp);
    CHECK();
    dumpWaveBuffer(buf,buffer->channels,buffer->sampleRate,(buffer->sampleFormat==HASF_SIGNED_16PCM)?16:8,tmp,buffer->size);
    free(tmp);

  }
*/

  return;
}

static void resetBuffer(segaapiBuffer_t* buffer) {
// *   - Send Routing 
// *      - for 1 channel buffer, channel is routed to Front-Left and Front-Right.
// *      - for 2 channel buffer, channel 0 is routed Front-Left, channel 1 is routed Front-Right  
// *   - Send Levels are set to 0 (infinite attenuation)
// *   - Channel Volume is set to 0xFFFFFFFF (no attenuation)
// *   - No notification.
// *   - StartLoopOffset is set to 0.
  buffer->startLoop = 0;
// *   - EndLoopOffset and EndOffset are set to pConfig->mapdata.dwSize.
  buffer->endOffset = buffer->size;
  buffer->endLoop = buffer->size;
// *   - No loop.
  buffer->loop = false;
// *   - Buffer is in the stop state. 
// *   - Play position is set to 0.
  updateBufferData(buffer, -1, -1);
}

SEGASTATUS SEGAAPI_CreateBuffer(HAWOSEBUFFERCONFIG* pConfig, HAWOSEGABUFFERCALLBACK pCallback, CTDWORD dwFlags, CTHANDLE *phHandle) {

  if ((phHandle == NULL) || (pConfig == NULL)) {
    return SEGAERR_BAD_POINTER;
  }

/*
TODO:
 * If all the voices are currently in use, CreateBuffer will perform voice-stealing 
 * to fulfill the request. Note that voice stealing may fail if all voices that are 
 * currently in use are set to HAWOSEP_MAXIMUM priority.
*/

  segaapiBuffer_t* buffer = malloc(sizeof(segaapiBuffer_t));
 
  buffer->callback = pCallback;
  buffer->synthesizer = dwFlags & HABUF_SYNTH_BUFFER;

  LOG("CTDWORD     dwPriority = 0x%08X;         // The priority with which the voices should be allocated.  This is used when voices need to be ripped off. \n",pConfig->dwPriority);
  LOG("CTDWORD     dwSampleRate = %u;       // The sample rate the voice desires \n",pConfig->dwSampleRate);
  buffer->sampleRate = pConfig->dwSampleRate;
  LOG("CTDWORD     dwSampleFormat = 0x%08X;     // The sample format the voice will use \n",pConfig->dwSampleFormat);
  buffer->sampleFormat = pConfig->dwSampleFormat;
  LOG("CTDWORD     byNumChans = 0x%08X;         // The number of samples in the sample frame. (1 = mono, 2 = stereo). \n",pConfig->byNumChans);
  buffer->channels = pConfig->byNumChans;
  LOG("CTDWORD     dwReserved = 0x%08X;         // Reserved field \n",pConfig->dwReserved);
  LOG("CTHANDLE    hUserData = 0x%08X;          // User data \n",(uintptr_t)pConfig->hUserData);
  buffer->userData = pConfig->hUserData;
  LOG("HAWOSEMAPDATA mapData {                   // The sample memory mapping for the buffer. \n");
  LOG("  CTDWORD     dwSize = 0x%08X;       // Supply by caller. Size (in bytes) of the valid sample data \n",pConfig->mapData.dwSize);
  buffer->size = pConfig->mapData.dwSize;
  LOG("  CTDWORD     dwOffset = 0x%08X;     // Return by driver. Offset of buffer where the the first valid sample should be written to \n",pConfig->mapData.dwOffset);
  pConfig->mapData.dwOffset = 0;
  LOG("  CTHANDLE    hBufferHdr = 0x%08X;   // Memory address that user-space application can access, or mapped memory handle. \n",(uintptr_t)pConfig->mapData.hBufferHdr);

  if (dwFlags & HABUF_ALLOC_USER_MEM) {

    buffer->data = pConfig->mapData.hBufferHdr;

  } else if (dwFlags & HABUF_USE_MAPPED_MEM) {
      
    // Not sure what you pass in here..

    buffer->data = pConfig->mapData.hBufferHdr; //((buffer_t*)(pConfig->mapData.hBufferHdr))->data; 

  } else {

    buffer->data = malloc(buffer->size);

  }

  // Write back result

  pConfig->mapData.hBufferHdr = buffer->data;

  LOG("}\n");

  alGenBuffers(1, &buffer->alBuffer);
  CHECK();
	alGenSources(1, &buffer->alSource);
  CHECK();

//alSourcef(buffer->alSource,AL_PITCH,0.25f); // SLOOOOWWWWWW

  //TODO: Link in a buffer list or array

  if (buffer->synthesizer) {
    LOG("Doesn't support synth buffers yet!\n");
//    usleep(1000*1000);
  }
/*
TODO:
 * HABUF_ALLOC_USER_MEM bit when set indicates caller allocate sound data memory buffer.  
 * HABUF_USE_MAPPED_MEM
Can't be used at the same time!!!
*/
  resetBuffer(buffer);

  *phHandle = buffer;

  return SEGA_SUCCESS;
}



SEGASTATUS SEGAAPI_SetUserData(CTHANDLE hHandle, CTHANDLE hUserData) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  buffer->userData = hUserData;
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_UpdateBuffer(CTHANDLE hHandle, CTDWORD dwStartOffset, CTDWORD dwLength) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  updateBufferData(buffer,dwStartOffset,dwLength);
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetEndLoopOffset(CTHANDLE hHandle, CTDWORD dwOffset) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  buffer->endLoop = dwOffset;
  updateBufferLoop(buffer);
  LOG("Set loop end to %u\n",dwOffset);
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetStartLoopOffset(CTHANDLE hHandle, CTDWORD dwOffset) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  buffer->startLoop = dwOffset;
  updateBufferLoop(buffer);
  LOG("Set loop start to %u\n",dwOffset);
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetSampleRate(CTHANDLE hHandle, CTDWORD dwSampleRate) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  buffer->sampleRate = dwSampleRate;
  updateBufferData(buffer,-1,-1);
  LOG("Setting sample rate to %u\n",dwSampleRate)
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetLoopState(CTHANDLE hHandle, CTBOOL bDoContinuousLooping) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  buffer->loop = bDoContinuousLooping;
  LOG("Setting loop state to %i\n",bDoContinuousLooping);
  alSourcei(buffer->alSource, AL_LOOPING, buffer->loop?AL_TRUE:AL_FALSE);
  CHECK();
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetPlaybackPosition(CTHANDLE hHandle, CTDWORD dwPlaybackPos) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  LOG("Setting playback position to %u\n",dwPlaybackPos);
  alSourcei(buffer->alSource, AL_BYTE_OFFSET, dwPlaybackPos);
  CHECK();
  return SEGA_SUCCESS;
}

CTDWORD SEGAAPI_GetPlaybackPosition(CTHANDLE hHandle) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return 0;
  }
  segaapiBuffer_t* buffer = hHandle;
  ALint position;
  alGetSourcei(buffer->alSource, AL_BYTE_OFFSET, &position);
  CHECK();
  return position;
}

SEGASTATUS SEGAAPI_Play(CTHANDLE hHandle) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  alSourcei(buffer->alSource, AL_LOOPING, buffer->loop?AL_TRUE:AL_FALSE);
  CHECK();
	alSourcePlay(buffer->alSource);
  CHECK();
  LOG("Starting to play\n");
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_Stop(CTHANDLE hHandle) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
	alSourceStop(buffer->alSource);
  CHECK();
  return SEGA_SUCCESS;
}

HAWOSTATUS SEGAAPI_GetPlaybackStatus(CTHANDLE hHandle) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  ALint state;
  alGetSourcei(buffer->alSource, AL_SOURCE_STATE, &state);
  CHECK();
  if ((state == AL_INITIAL) || (state == AL_STOPPED)) {
    return HAWOSTATUS_STOP;                     /* The voice is stopped */
  }
  if (state == AL_PLAYING) {
    return HAWOSTATUS_ACTIVE;                    /* The voice is playing */
  }
  if (state == AL_PAUSED) {
    return HAWOSTATUS_PAUSE;                     /* The voice is paused */
  }  
  return HAWOSTATUS_INVALID;
}

SEGASTATUS SEGAAPI_SetReleaseState(CTHANDLE hHandle, CTBOOL bSet) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  //TODO!!!!
  if (bSet) {
//    alSourceStop(buffer->alSource);
    alSourcei(buffer->alSource, AL_LOOPING, AL_FALSE);
    CHECK();
  }
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_DestroyBuffer(CTHANDLE hHandle) {
  if (hHandle == NULL) { // Not sure if this is correct here, but ABC currently tries to call this with a null pointer..
    return SEGAERR_BAD_HANDLE;
  }
  segaapiBuffer_t* buffer = hHandle;
  alDeleteSources(1,&buffer->alSource);
  CHECK();
  alDeleteBuffers(1,&buffer->alBuffer);
  CHECK();
/*
  free(buffer->data); //FIXME: don't free if this existed before!
*/
  free(buffer);  
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_Init(void) {

/*
  static unsigned int count = 0;
  printf("Init count is %i\n",++count);
*/

/*
  fclose(fopen("segaapi.log","w"));
*/

	int res = alutInit(NULL, NULL); 
  if (res == AL_FALSE) {
    LOG("Could not init alut!\n");
    exit(1);
  } 

/*
  alBufferSubDataSOFT = alGetProcAddress("alBufferSubDataSOFT");
  if (alBufferSubDataSOFT == NULL) {
    LOG("Could not resolve AL extension!\n");
    exit(1);
  }
*/
  alBufferSamplesSOFT = alGetProcAddress("alBufferSamplesSOFT");
  if (alBufferSamplesSOFT == NULL) {
    LOG("Could not resolve AL extension!\n");
    exit(1);
  }
  alGetBufferSamplesSOFT = alGetProcAddress("alGetBufferSamplesSOFT");
  if (alGetBufferSamplesSOFT == NULL) {
    LOG("Could not resolve AL extension!\n");
    exit(1);
  }
  alBufferSubSamplesSOFT = alGetProcAddress("alBufferSubSamplesSOFT");
  if (alBufferSubSamplesSOFT == NULL) {
    LOG("Could not resolve AL extension!\n");
    exit(1);
  }
  

//TODO: Create thread which watches all buffers so the callback can be invoked

  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_Exit(void) {
  alutExit();
  return SEGA_SUCCESS;
}






SEGASTATUS SEGAAPI_Reset(void) {
/*
TODO:
 * This includes but not limited to the followings:
 *  - Stop and destroy all the currently playing buffers. All previous buffer 
 *    handles are no longer valid after returning from this call.
 *  - Resets all volume levels to its default.
 *  - Resets EAX property values to their defaults.
 *  - Resets SPDIF Out sampling rate and routing to its default. 
 *   
 *
 * @return  
 * The SEGASTATUS code.  
 */
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetIOVolume(HAPHYSICALIO dwPhysIO, CTDWORD dwVolume) {
  //TODO!!!!
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetSendRouting(CTHANDLE hHandle, CTDWORD dwChannel, CTDWORD dwSend, HAROUTING dwDest) {
  //TODO!!!!
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetSendLevel(CTHANDLE hHandle, CTDWORD dwChannel, CTDWORD dwSend, CTDWORD dwLevel) {
  //TODO!!!!
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetSynthParam(CTHANDLE hHandle, HASYNTHPARAMSEXT param, CTLONG lPARWValue) {
  //TODO!!!!
  LOG("Setting synth param %i / 0x%X\n",param,param);
  return SEGA_SUCCESS;
}

SEGASTATUS SEGAAPI_SetChannelVolume(CTHANDLE hHandle, CTDWORD dwChannel, CTDWORD dwVolume) {
  //TODO!!!!
  return SEGA_SUCCESS;
}
