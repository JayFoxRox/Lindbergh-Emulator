// Small test code for the segaapi implementation

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "segaapi.h"

void* loadWaveFile(char *fname, unsigned int* channels, unsigned int* sampleRate, unsigned int* sampleBits, size_t* size) {
    FILE *fp;

    fp = fopen(fname,"rb");
    if (fp)
    {
        uint8_t id[4];
        uint8_t* sound_buffer; //four bytes to hold 'RIFF'
        uint32_t _size; //32 bit value to hold file size
        uint16_t format_tag, _channels, block_align, bits_per_sample; //our 16 values
        uint32_t format_length, sample_rate, avg_bytes_sec, data_size, i; //our 32 bit values

        fread(id, sizeof(uint8_t), 4, fp); //read in first four bytes
        if (!strncmp((char*)id, "RIFF",4))
        { //we had 'RIFF' let's continue
            fread(&_size, sizeof(uint32_t), 1, fp); //read in 32bit size value
            fread(id, sizeof(uint8_t), 4, fp); //read in 4 byte string now
            if (!strncmp((char*)id,"WAVE",4))
            { //this is probably a wave file since it contained "WAVE"

                fread(id, sizeof(uint8_t), 4, fp); //read in 4 bytes "fmt ";
                fread(&format_length, sizeof(uint32_t),1,fp);

                fread(&format_tag, sizeof(uint16_t), 1, fp); //check mmreg.h (i think?) for other possible format tags like ADPCM
                fread(&_channels, sizeof(uint16_t),1,fp); //1 mono, 2 stereo
                fread(&sample_rate, sizeof(uint32_t), 1, fp); //like 44100, 22050, etc...
                fread(&avg_bytes_sec, sizeof(uint32_t), 1, fp); //probably won't need this
                fread(&block_align, sizeof(uint16_t), 1, fp); //probably won't need this
                fread(&bits_per_sample, sizeof(uint16_t), 1, fp); //8 bit or 16 bit file?
/*
  printf("block align: %i\n",block_align);
  printf("bits per sample: %i\n",bits_per_sample);
*/
                fread(id, sizeof(uint8_t), 4, fp); //read in 'data'
                fread(&data_size, sizeof(uint32_t), 1, fp); //how many bytes of sound data we have

                sound_buffer = (uint8_t *) malloc (sizeof(uint8_t) * data_size); //set aside sound buffer space
                fread(sound_buffer, sizeof(uint8_t), data_size, fp); //read in our whole sound data chunk

                *channels = _channels;
                *sampleRate = sample_rate;
                *sampleBits = bits_per_sample;
                *size = data_size;
                return sound_buffer;

            }
            else
                printf("Error: RIFF file but not a wave file\n");
        }
        else
            printf("Error: not a RIFF file\n");
    }
  return NULL;
}

int main(int argc, char* argv[]) {

  SEGAAPI_Init();

  HAWOSEBUFFERCONFIG config;  
  CTHANDLE handle;

  int i;
  for(i = 1; i < argc; i++) { 
  
    unsigned int channels;
    unsigned int sampleRate;
    unsigned int sampleBits;
    size_t size;
    void* buffer = loadWaveFile(argv[i],&channels,&sampleRate,&sampleBits,&size);
  
//    printf("SampleBits: %i\n",sampleBits);

    config.dwPriority = -1;
    config.dwSampleRate = sampleRate;
    config.dwSampleFormat = (sampleBits==16)?HASF_SIGNED_16PCM:HASF_UNSIGNED_8PCM;
    config.byNumChans = channels;
    config.dwReserved = 0;
    config.hUserData = NULL;
    config.mapData.dwSize = size;
//    config.mapData.hBufferHdr = buffer;





    #if 0
    CTuint32_t     dwPriority = 0xFFFFFFFF;         /* The priority with which the voices should be allocated.  This is used when voices need to be ripped off. */
    CTuint32_t     dwSampleRate = 0x0000BB80;       /* The sample rate the voice desires */
    CTuint32_t     dwSampleFormat = 0x00000020;     /* The sample format the voice will use */
    CTuint32_t     byNumChans = 0x00000001;         /* The number of samples in the sample frame. (1 = mono, 2 = stereo). */
    CTuint32_t     dwReserved = 0x00000000;         /* Reserved field */
    CTHANDLE    hUserData = 0xF5ED7E76;          /* User data */
    HAWOSEMAPDATA mapData {                   /* The sample memory mapping for the buffer. */
      CTuint32_t     dwSize = 0x0000F8F4;       /* Supply by caller. Size (in bytes) of the valid sample data */
      CTuint32_t     dwOffset = 0x00000000;     /* Return by driver. Offset of buffer where the the first valid sample should be written to */
      CTHANDLE    hBufferHdr = 0x00000000;   /* Memory address that user-space application can access, or mapped memory handle. */
    }
    Doesn't support synth buffers yet!
    #endif

    //second param:   HAWOSEGABUFFERCALLBACK pCallback
    SEGASTATUS ret = SEGAAPI_CreateBuffer(&config, NULL, 0, &handle);
    memcpy(config.mapData.hBufferHdr, buffer, size);
    SEGAAPI_UpdateBuffer(handle,0,size);
//    SEGAAPI_SetSampleRate(handle, 44100);

    SEGAAPI_Play(handle);
    while(SEGAAPI_GetPlaybackStatus(handle) == HAWOSTATUS_ACTIVE) {
      usleep(1000);
    }
    SEGAAPI_DestroyBuffer(handle);

  }

  SEGAAPI_Exit();
  return 0;
}
