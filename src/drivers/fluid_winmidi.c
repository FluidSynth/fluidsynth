/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


/* fluid_winmidi.c
 *
 * Driver for Windows MIDI
 *
 * NOTE: Unfortunately midiInAddBuffer(), for SYSEX data, should not be called
 * from within the MIDI input callback, despite many examples contrary to that
 * on the Internet.  Some MIDI devices will deadlock.  Therefore we add MIDIHDR
 * pointers to a queue and re-add them in a separate thread.  Lame-o API! :(
 *
 * Multiple/single devices handling capabilities:
 * This driver is able to handle multiple devices choosen by the user trough
 * the setting midi.winmidi.device. This will allow the driver to receive MIDI
 * messages comming from distinct devices and forward these messages on
 * distinct MIDI channels set.
 * 1)For example, if the user chooses 2 devices at index 0 and 1, the user must
 * specify this by putting the name "multi:0,1" in midi.winmidi.device setting.
 * We get a fictif device composed of real devices (0,1). This fictif device
 * behaves like a device with 32 MIDI channels whose messages are forwarded to
 * driver output as this.
 * - MIDI messages from real device 0 are output to MIDI channels set 0 to 15.
 * - MIDI messages from real device 1 are output to MIDI channels set 15 to 31.
 *
 * 2)Now another example with the name "multi:1,0" in midi.winmidi.device setting.
 * The driver will forward MIDI messages as this:
 * - MIDI messages from real device 1 are output to MIDI channels set 0 to 15.
 * - MIDI messages from real device 0 are output to MIDI channels set 15 to 31.
 * So, the order of real device index specified the setting allows the user to
 * choose the MIDI channel set associated with this real device at the driver
 * output.
 *
 * Note also that the driver handles single device choosen by putting the device
 * name in midi.winmidi.device setting.
 * For example, let the followings device names:
 * 1:Port MIDI SB Live! [CE00], 0:SB PCI External MIDI, default, multi:0[,1,..]
 * The user can set the name "1:Port MIDI SB Live! [CE00]" in the setting.
 * or use the multi device naming "multi:1" (specifying only device 1 index).
 * Both naming choice allows the driver to handle the same single device.
 *
 */

#include "fluidsynth_priv.h"

#if WINMIDI_SUPPORT

#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#define MIDI_SYSEX_MAX_SIZE     512
#define MIDI_SYSEX_BUF_COUNT    16

typedef struct
{
    fluid_midi_driver_t driver;
    HMIDIIN hmidiin;

    /* MIDI HDR for SYSEX buffer */
    MIDIHDR sysExHdrs[MIDI_SYSEX_BUF_COUNT];

    /* Thread for SYSEX re-add thread */
    HANDLE hThread;
    DWORD  dwThread;

    /* Sysex data buffer */
    unsigned char sysExBuf[MIDI_SYSEX_BUF_COUNT * MIDI_SYSEX_MAX_SIZE];

} fluid_winmidi_driver_t;

#define msg_type(_m)  ((unsigned char)(_m & 0xf0))
#define msg_chan(_m)  ((unsigned char)(_m & 0x0f))
#define msg_p1(_m)    ((_m >> 8) & 0x7f)
#define msg_p2(_m)    ((_m >> 16) & 0x7f)

static char *
fluid_winmidi_input_error(char *strError, MMRESULT no)
{
#ifdef _UNICODE
    WCHAR wStr[MAXERRORLENGTH];

    midiInGetErrorText(no, wStr, MAXERRORLENGTH);
    WideCharToMultiByte(CP_UTF8, 0, wStr, -1, strError, MAXERRORLENGTH, 0, 0);
#else
    midiInGetErrorText(no, strError, MAXERRORLENGTH);
#endif

    return strError;
}

static void CALLBACK
fluid_winmidi_callback(HMIDIIN hmi, UINT wMsg, DWORD_PTR dwInstance,
                       DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    fluid_winmidi_driver_t *dev = (fluid_winmidi_driver_t *) dwInstance;
    fluid_midi_event_t event;
    LPMIDIHDR pMidiHdr;
    unsigned char *data;
    unsigned int msg_param = (unsigned int) dwParam1;

    switch(wMsg)
    {
    case MIM_OPEN:
        break;

    case MIM_CLOSE:
        break;

    case MIM_DATA:
        event.type = msg_type(msg_param);
        event.channel = msg_chan(msg_param);

        if(event.type != PITCH_BEND)
        {
            event.param1 = msg_p1(msg_param);
            event.param2 = msg_p2(msg_param);
        }
        else      /* Pitch bend is a 14 bit value */
        {
            event.param1 = (msg_p2(msg_param) << 7) | msg_p1(msg_param);
            event.param2 = 0;
        }

        (*dev->driver.handler)(dev->driver.data, &event);
        break;

    case MIM_LONGDATA:    /* SYSEX data */
        if(dev->hThread == NULL)
        {
            break;
        }

        pMidiHdr = (LPMIDIHDR)dwParam1;
        data = (unsigned char *)(pMidiHdr->lpData);

        /* We only process complete SYSEX messages (discard those that are too small or too large) */
        if(pMidiHdr->dwBytesRecorded > 2 && data[0] == 0xF0
                && data[pMidiHdr->dwBytesRecorded - 1] == 0xF7)
        {
            fluid_midi_event_set_sysex(&event, pMidiHdr->lpData + 1,
                                       pMidiHdr->dwBytesRecorded - 2, FALSE);
            (*dev->driver.handler)(dev->driver.data, &event);
        }

        PostThreadMessage(dev->dwThread, MM_MIM_LONGDATA, 0, dwParam1);
        break;

    case MIM_ERROR:
        break;

    case MIM_LONGERROR:
        break;

    case MIM_MOREDATA:
        break;
    }
}

/**
 * build a device name prefixed by its index. The format of the returned
 * name is: dev_idx:dev_name
 * The name returned is convenient for midi.winmidi.device setting.
 * It allows the user to identify a device index through its name or vise
 * versa. This is useful allowing the user specify a multi device name
 * using a list of device index.
 *
 * @param dev_idx, device index
 * @param dev_name, name of the device
 * @return the new device name (that must be freed when finish with it) or
 *  NULL if memory allocation error.
 */
static char *fluid_winmidi_get_device_name(int dev_idx, char *dev_name)
{
    char *new_dev_name;

    int i =  dev_idx;
    size_t size = 0; /* index size */

    do
    {
        size++;
        i = i / 10 ;
    }
    while(i);

    /* index size + separator + name length + zero termination */
    new_dev_name = FLUID_MALLOC(size + 2 + FLUID_STRLEN(dev_name));
    if(new_dev_name)
    {
        /* the name is filled if allocation is successful */
        FLUID_SPRINTF(new_dev_name, "%d:%s", dev_idx, dev_name);
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
    }
    return new_dev_name;
}

void fluid_winmidi_midi_driver_settings(fluid_settings_t *settings)
{
    MMRESULT res;
    MIDIINCAPS in_caps;
    UINT i, num;
    fluid_settings_register_str(settings, "midi.winmidi.device", "default", 0);
    num = midiInGetNumDevs();

    if(num > 0)
    {
        fluid_settings_add_option(settings, "midi.winmidi.device", "default");

        for(i = 0; i < num; i++)
        {
            res = midiInGetDevCaps(i, &in_caps, sizeof(MIDIINCAPS));

            if(res == MMSYSERR_NOERROR)
            {
                /* add new device name (prefixed by its index) */
                char *new_dev_name = fluid_winmidi_get_device_name(i, in_caps.szPname);
                if(!new_dev_name)
                {
                    break;
                }
                fluid_settings_add_option(settings, "midi.winmidi.device",
                                          new_dev_name);
                FLUID_FREE(new_dev_name);
            }
        }
    }
}

/* Thread for re-adding SYSEX buffers */
static DWORD WINAPI fluid_winmidi_add_sysex_thread(void *data)
{
    fluid_winmidi_driver_t *dev = (fluid_winmidi_driver_t *)data;
    MSG msg;
    int code;

    for(;;)
    {
        code = GetMessage(&msg, NULL, 0, 0);

        if(code < 0)
        {
            FLUID_LOG(FLUID_ERR, "fluid_winmidi_add_sysex_thread: GetMessage() failed.");
            break;
        }

        if(msg.message == WM_CLOSE)
        {
            break;
        }

        switch(msg.message)
        {
        case MM_MIM_LONGDATA:
            midiInAddBuffer(dev->hmidiin, (LPMIDIHDR)msg.lParam, sizeof(MIDIHDR));
            break;
        }
    }

    return 0;
}

/*
 * new_fluid_winmidi_driver
 */
fluid_midi_driver_t *
new_fluid_winmidi_driver(fluid_settings_t *settings,
                         handle_midi_event_func_t handler, void *data)
{
    fluid_winmidi_driver_t *dev;
    MIDIHDR *hdr;
    MMRESULT res;
    UINT i, num, midi_num = 0;
    MIDIINCAPS in_caps;
    char strError[MAXERRORLENGTH];
    char dev_name[MAXPNAMELEN];

    /* not much use doing anything */
    if(handler == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Invalid argument");
        return NULL;
    }

    /* get the device name. if none is specified, use the default device. */
    if(fluid_settings_copystr(settings, "midi.winmidi.device", dev_name, MAXPNAMELEN) != FLUID_OK)
    {
        FLUID_LOG(FLUID_DBG, "No MIDI in device selected, using \"default\"");
        FLUID_STRCPY(dev_name, "default");
    }

    /* check if there any midi devices installed */
    num = midiInGetNumDevs();

    if(num == 0)
    {
        FLUID_LOG(FLUID_ERR, "no MIDI in devices found");
        return NULL;
    }

    /* find the device */
    if(FLUID_STRCASECMP("default", dev_name) != 0)
    {
        for(i = 0; i < num; i++)
        {
            res = midiInGetDevCaps(i, &in_caps, sizeof(MIDIINCAPS));

            if(res == MMSYSERR_NOERROR)
            {
                int str_cmp_res;
                char *new_dev_name = fluid_winmidi_get_device_name(i, in_caps.szPname);
                if(!new_dev_name)
                {
                    break;
                }
#ifdef _UNICODE
                WCHAR wDevName[MAXPNAMELEN];
                MultiByteToWideChar(CP_UTF8, 0, dev_name, -1, wDevName, MAXPNAMELEN);

                str_cmp_res = wcsicmp(wDevName, new_dev_name);
#else
                str_cmp_res = FLUID_STRCASECMP(dev_name, new_dev_name);
#endif

                FLUID_LOG(FLUID_DBG, "Testing midi device \"%s\"", new_dev_name);
                FLUID_FREE(new_dev_name);

                if(str_cmp_res == 0)
                {
                    FLUID_LOG(FLUID_DBG, "Selected midi device number: %u", i);
                    midi_num = i;
                    break;
                }
            }
            else
            {
                FLUID_LOG(FLUID_DBG, "Error testing midi device %u of %u: %s (error %d)", i, num, fluid_winmidi_input_error(strError, res), res);
            }
        }

        if(midi_num != i)
        {
            FLUID_LOG(FLUID_ERR, "Device \"%s\" does not exists", dev_name);
            return NULL;
        }
    }

    dev = FLUID_MALLOC(sizeof(fluid_winmidi_driver_t));

    if(dev == NULL)
    {
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_winmidi_driver_t));

    dev->hmidiin = NULL;
    dev->driver.handler = handler;
    dev->driver.data = data;

    /* try opening the device */
    res = midiInOpen(&dev->hmidiin, midi_num,
                     (DWORD_PTR) fluid_winmidi_callback,
                     (DWORD_PTR) dev, CALLBACK_FUNCTION);

    if(res != MMSYSERR_NOERROR)
    {
        FLUID_LOG(FLUID_ERR, "Couldn't open MIDI input: %s (error %d)",
                  fluid_winmidi_input_error(strError, res), res);
        goto error_recovery;
    }

    /* Prepare and add SYSEX buffers */
    for(i = 0; i < MIDI_SYSEX_BUF_COUNT; i++)
    {
        hdr = &dev->sysExHdrs[i];

        hdr->lpData = (LPSTR)&dev->sysExBuf[i * MIDI_SYSEX_MAX_SIZE];
        hdr->dwBufferLength = MIDI_SYSEX_MAX_SIZE;

        /* Prepare a buffer for SYSEX data and add it */
        res = midiInPrepareHeader(dev->hmidiin, hdr, sizeof(MIDIHDR));

        if(res == MMSYSERR_NOERROR)
        {
            res = midiInAddBuffer(dev->hmidiin, hdr, sizeof(MIDIHDR));

            if(res != MMSYSERR_NOERROR)
            {
                FLUID_LOG(FLUID_WARN, "Failed to prepare MIDI SYSEX buffer: %s (error %d)",
                          fluid_winmidi_input_error(strError, res), res);
                midiInUnprepareHeader(dev->hmidiin, hdr, sizeof(MIDIHDR));
            }
        }
        else
            FLUID_LOG(FLUID_WARN, "Failed to prepare MIDI SYSEX buffer: %s (error %d)",
                      fluid_winmidi_input_error(strError, res), res);
    }

    /* Create thread which processes re-adding SYSEX buffers */
    dev->hThread = CreateThread(
                       NULL,
                       0,
                       (LPTHREAD_START_ROUTINE)
                       fluid_winmidi_add_sysex_thread,
                       dev,
                       0,
                       &dev->dwThread);

    if(dev->hThread == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create SYSEX buffer processing thread");
        goto error_recovery;
    }

    /* Start the MIDI input interface */
    if(midiInStart(dev->hmidiin) != MMSYSERR_NOERROR)
    {
        FLUID_LOG(FLUID_ERR, "Failed to start the MIDI input. MIDI input not available.");
        goto error_recovery;
    }

    return (fluid_midi_driver_t *) dev;

error_recovery:

    delete_fluid_winmidi_driver((fluid_midi_driver_t *) dev);
    return NULL;
}

/*
 * delete_fluid_winmidi_driver
 */
void
delete_fluid_winmidi_driver(fluid_midi_driver_t *p)
{
    int i;

    fluid_winmidi_driver_t *dev = (fluid_winmidi_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    if(dev->hThread != NULL)
    {
        PostThreadMessage(dev->dwThread, WM_CLOSE, 0, 0);
        WaitForSingleObject(dev->hThread, INFINITE);

        CloseHandle(dev->hThread);
        dev->hThread = NULL;
    }

    if(dev->hmidiin != NULL)
    {
        midiInStop(dev->hmidiin);
        midiInReset(dev->hmidiin);

        for(i = 0; i < MIDI_SYSEX_BUF_COUNT; i++)
        {
            MIDIHDR *hdr = &dev->sysExHdrs[i];

            if ((hdr->dwFlags & MHDR_PREPARED))
            {
                midiInUnprepareHeader(dev->hmidiin, hdr, sizeof(MIDIHDR));
            }
        }

        midiInClose(dev->hmidiin);
    }

    FLUID_FREE(dev);
}

#endif /* WINMIDI_SUPPORT */
