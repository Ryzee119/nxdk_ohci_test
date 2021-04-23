// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "uac.h"

#define MIC_SAMPLE_RATE 16000

//These values are set to match "uac_wav".
#define SPEAKER_SAMPLE_RATE 16000
#define SPEAKER_NUM_CHANNELS 2
#define SPEAKER_BIT_RES 16
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#include "uac_wav.h"

typedef struct audio_stream_data
{
    uint8_t *voice_data;
    uint32_t voice_len;
    uint32_t voice_pos;
    int16_t record_data[256];
    int32_t record_amplitude;
} audio_stream_data, *pjaudio_stream_data;

static int32_t uac_audio_in_callback(UAC_DEV_T *adev, uint8_t *rdata, int len)
{
    audio_stream_data *sdata = (audio_stream_data *)adev->user_data;

    //The actual packet size is based on the sample rate of the audio.
    static const int32_t packet_size = MIC_SAMPLE_RATE * sizeof(uint16_t) / 1000;
    int32_t chunk_size = MIN(sizeof(sdata->record_data), packet_size);
    memcpy(sdata->record_data, rdata, chunk_size);
    return 0;
}

static int32_t uac_audio_out_callback(UAC_DEV_T *adev, uint8_t *tdata, int len)
{
    audio_stream_data *sdata = (audio_stream_data *)adev->user_data;

    //The actual packet size is based on the sample rate of the audio.
    static const int32_t packet_size = SPEAKER_SAMPLE_RATE * SPEAKER_NUM_CHANNELS * (SPEAKER_BIT_RES / 8) / 1000;
    int32_t chunk_size = MIN(sdata->voice_len - sdata->voice_pos, packet_size);
    memcpy(tdata, sdata->voice_data + sdata->voice_pos, chunk_size);

    //Check if end of audio, loop back to start
    int32_t is_final = (sdata->voice_pos + chunk_size) >= sdata->voice_len;
    sdata->voice_pos = (is_final) ? 0 : (sdata->voice_pos + chunk_size);

    return chunk_size;
}

uint32_t uac_init_device(UAC_DEV_T *adev)
{
    int32_t ret = 0;
    uint32_t speaker_rate = SPEAKER_SAMPLE_RATE;
    uint32_t mic_rate = MIC_SAMPLE_RATE;

    ret = usbh_uac_open(adev);
    if (ret)
    {
        speaker_rate = 0;
        debugPrint("UAC: Failed to open device. Error: %d\n", ret);
        return ret;
    }

    //The number of Channels in an AudioStreaming interface and the audio sample bit resolution (16 bits or 24
    //bits) can be derived from the wMaxPacketSize (len) field.
    //Ref table 8-26 UNIVERSAL SERIAL BUS DEVICE CLASS DEFINITION FOR BASIC AUDIO FUNCTIONS
    //Mono, 16 bits (96)
    //Mono, 24 bits (144)
    //Stereo, 16 bits (192)
    //Stereo, 24 bits (288)
    uint32_t minimum_pkt_size = SPEAKER_BIT_RES * SPEAKER_NUM_CHANNELS * 6;
    if (adev->asif_out.ep->wMaxPacketSize < minimum_pkt_size)
    {
        debugPrint("UAC: Warning, this sample expects a %d channel, %d bit compatible audio device\n",
                   SPEAKER_NUM_CHANNELS,
                   SPEAKER_BIT_RES);
    }

    //Attempt to set the sample rate of the speaker and microphone
    ret = usbh_uac_sampling_rate_control(adev, UAC_SPEAKER, UAC_SET_CUR, &speaker_rate);
    ret |= usbh_uac_sampling_rate_control(adev, UAC_SPEAKER, UAC_GET_CUR, &speaker_rate);
    if (ret)
    {
        speaker_rate = 0;
        debugPrint("UAC: Failed to setup speaker. Error: %d\n", ret);
    }

    ret = usbh_uac_sampling_rate_control(adev, UAC_MICROPHONE, UAC_SET_CUR, &mic_rate);
    ret |= usbh_uac_sampling_rate_control(adev, UAC_MICROPHONE, UAC_GET_CUR, &mic_rate);
    if (ret)
    {
        mic_rate = 0;
        debugPrint("UAC: Failed to setup microphone. Error: %d\n", ret);
    }

    //Set sound to play
    uac_set_voice_data(adev, nxdk_wav_h_bin, nxdk_wav_h_bin_len);

    if (speaker_rate && speaker_rate == SPEAKER_SAMPLE_RATE)
    {
        ret = usbh_uac_start_audio_out(adev, uac_audio_out_callback);
        if (ret)
        {
            debugPrint("UAC: Failed starting audio. Error: %d\n", ret);
        }
    }
    else
    {
        debugPrint("UAC: Warning, speaker does not support %dHz audio\n", SPEAKER_SAMPLE_RATE);
    }

    if (mic_rate && mic_rate == MIC_SAMPLE_RATE)
    {
        ret = usbh_uac_start_audio_in(adev, uac_audio_in_callback);
        if (ret)
        {
            debugPrint("UAC: Failed starting microphone input. Error: %d\n", ret);
        }
    }
    else
    {
        debugPrint("UAC: Warning, microphone does not support %dHz recording\n", MIC_SAMPLE_RATE);
    }

    return ret;
}

void uac_connection_callback(UAC_DEV_T *adev, int status)
{
    //Dont handle uac init at interface level.
}

void uac_disconnect_callback(UAC_DEV_T *adev, int status)
{
    debugPrint("USB Audio device disconnected\n");
    if (adev->user_data)
        free(adev->user_data);
}

//Expects stereo 16bit PCM 16khz
void uac_set_voice_data(UAC_DEV_T *adev, uint8_t *voice_data, uint32_t voice_len)
{
    if (adev->user_data == NULL)
    {
        adev->user_data = malloc(sizeof(audio_stream_data));
    }
    assert(adev->user_data != NULL);
    memset(adev->user_data, 0x00, sizeof(audio_stream_data));
    audio_stream_data *sdata = (audio_stream_data *)adev->user_data;
    sdata->voice_data = voice_data;
    sdata->voice_len = voice_len;
    sdata->voice_pos = 0;
}

void uac_print_all_devices()
{
    UAC_DEV_T *adev = usbh_uac_get_device_list();
    if (adev == NULL)
        return;
    if (adev->user_data == NULL)
        return;

    int32_t i = 0;
    while (adev != NULL)
    {
        if (adev->user_data == NULL)
        {
            adev = adev->next;
            continue;
        }
        audio_stream_data *sdata = (audio_stream_data *)adev->user_data;

        //Calculate the average amplitude of the input stream to illustrate this sample
        static const int32_t packet_size = MIC_SAMPLE_RATE * sizeof(uint16_t) / 1000;
        int32_t average = 0;
        for (int j = 0; j < packet_size; j++)
            average += (int16_t)sdata->record_data[j];
        average /= packet_size;
        sdata->record_amplitude = average;

        debugPrint("AUDIO #%d: %s ; ", i, adev->asif_out.flag_streaming ? "Streaming audio..." : "No Speaker");
        debugPrint("MIC %s %i \n", adev->asif_in.flag_streaming ? "Level: " : "No Mic", abs(sdata->record_amplitude));
        debugPrint("\n");
        adev = adev->next;
        i++;
    }
}