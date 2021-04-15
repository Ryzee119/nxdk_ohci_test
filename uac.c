// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "uac.h"

#define SAMPLE_RATE 16000
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
    int32_t chunk_size = len < sizeof(sdata->record_data) ? len : sizeof(sdata->record_data);
    memcpy(sdata->record_data, rdata, chunk_size);

    int32_t average = 0;
    for (int j = 0; j < chunk_size; j++)
        average += (int16_t)sdata->record_data[j];
    average /= chunk_size;
    sdata->record_amplitude = average;
    return 0;
}

static int32_t uac_audio_out_callback(UAC_DEV_T *adev, uint8_t *tdata, int len)
{
    audio_stream_data *sdata = (audio_stream_data *)adev->user_data;
    int32_t packet_size = SAMPLE_RATE * 2 / 500;

    int32_t chunk_size = MIN(sdata->voice_len - sdata->voice_pos, packet_size);
    memcpy(tdata, sdata->voice_data + sdata->voice_pos, chunk_size);

    int32_t is_final = (sdata->voice_pos + chunk_size) >= sdata->voice_len;
    sdata->voice_pos = (is_final) ? 0 : (sdata->voice_pos + chunk_size);

    return chunk_size;
}

static uint32_t uac_init_device(UAC_DEV_T *adev)
{
    uint32_t val32 = 0, ret = 0;
    usbh_uac_open(adev);
    val32 = SAMPLE_RATE;
    ret |= usbh_uac_sampling_rate_control(adev, UAC_SPEAKER, UAC_SET_CUR, &val32);
    ret |= usbh_uac_sampling_rate_control(adev, UAC_MICROPHONE, UAC_SET_CUR, &val32);

    if (usbh_uac_sampling_rate_control(adev, UAC_SPEAKER, UAC_GET_CUR, &val32) == UAC_RET_OK)
        debugPrint("Speakers's current sampling rate is %d.\n", val32);
    if (usbh_uac_sampling_rate_control(adev, UAC_MICROPHONE, UAC_GET_CUR, &val32) == UAC_RET_OK)
        debugPrint("Microphone's current sampling rate is %d.\n", val32);

    uac_set_voice_data(adev, nxdk_wav_h_bin, nxdk_wav_h_bin_len);

    ret |= usbh_uac_start_audio_out(adev, uac_audio_out_callback);
    ret |= usbh_uac_start_audio_in(adev, uac_audio_in_callback);

    return ret;
}

void uac_connection_callback(UAC_DEV_T *adev, int status)
{
    debugPrint("USB Audio device connected\n");
    uac_init_device(adev);
}
void uac_disconnect_callback(UAC_DEV_T *adev, int status)
{
    debugPrint("USB Audio device disconnected\n");
    if (adev->user_data)
        free(adev->user_data);
}

//Expects stereo PCM 16khz
void uac_set_voice_data(UAC_DEV_T *adev, uint8_t *voice_data, uint32_t voice_len)
{
    if (adev->user_data == NULL)
        adev->user_data = malloc(sizeof(audio_stream_data));
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
        debugPrint("AUDIO #%d: %s ;", i, adev->asif_out.flag_streaming ? "Playing" : "No Speaker");
        debugPrint("MIC %s %i \n", adev->asif_in.flag_streaming ? "Recording, Level: " : "No Mic", abs(sdata->record_amplitude));
        adev = adev->next;
        i++;
    }
}