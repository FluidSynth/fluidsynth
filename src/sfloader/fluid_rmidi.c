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
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "fluid_rmidi.h"

#define RIFF_FCC    FLUID_FOURCC('R', 'I', 'F', 'F')
#define RMID_FCC    FLUID_FOURCC('R', 'M', 'I', 'D')
#define DATA_FCC    FLUID_FOURCC('d', 'a', 't', 'a')
#define LIST_FCC    FLUID_FOURCC('L', 'I', 'S', 'T')
#define INFO_FCC    FLUID_FOURCC('I', 'N', 'F', 'O')
#define DBNK_FCC    FLUID_FOURCC('D', 'B', 'N', 'K')
#define SFBK_FCC    FLUID_FOURCC('s', 'f', 'b', 'k')
#define DLS_FCC     FLUID_FOURCC('D', 'L', 'S', ' ')

typedef struct _fluid_rmidi_src_t
{
    const unsigned char *data;            /* the whole file if it is in memory, otherwise NULL */
    const fluid_file_callbacks_t *fcbs;   /* file callbacks, only used if data is NULL */
    void *file;
    fluid_long_long_t size;
} fluid_rmidi_src_t;

/* Read bytes at the given offset, without reading past the given end. */
static int fluid_rmidi_read_at(const fluid_rmidi_src_t *src, fluid_long_long_t pos,
                               fluid_long_long_t end, void *buf, size_t length)
{
    if(pos < 0 || pos > end || (fluid_long_long_t)length > end - pos)
    {
        return FLUID_FAILED;
    }

    if(src->data != NULL)
    {
        FLUID_MEMCPY(buf, src->data + pos, length);
        return FLUID_OK;
    }

    if(src->fcbs->fseek(src->file, pos, SEEK_SET) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    return src->fcbs->fread(buf, (fluid_long_long_t)length, src->file);
}

/* Read and validate a RIFF chunk header at the given offset. */
static int fluid_rmidi_read_chunk(const fluid_rmidi_src_t *src, fluid_long_long_t pos,
                                  fluid_long_long_t end, uint32_t *id, uint32_t *size)
{
    uint32_t header[2];

    if(fluid_rmidi_read_at(src, pos, end, header, sizeof(header)) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    *id = header[0];
    *size = FLUID_LE32TOH(header[1]);

    if((fluid_long_long_t)*size > end - (pos + 8))
    {
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/*
 * Read the chunks inside a LIST[INFO] chunk and remember the DBNK bank offset.
 * All other INFO chunks are metadata and are ignored.
 */
static int fluid_rmidi_parse_info(const fluid_rmidi_src_t *src,
                                  fluid_long_long_t start, fluid_long_long_t end,
                                  int *bank_offset, int *have_bank_offset)
{
    while(start + 8 <= end)
    {
        uint32_t id, size;

        if(fluid_rmidi_read_chunk(src, start, end, &id, &size) != FLUID_OK)
        {
            return FLUID_FAILED;
        }

        if(id == DBNK_FCC)
        {
            uint16_t value, offset;

            if(size != 2)
            {
                FLUID_LOG(FLUID_ERR, "RMIDI DBNK chunk has invalid size of %u bytes", size);
                return FLUID_FAILED;
            }

            if(fluid_rmidi_read_at(src, start + 8, end, &value, sizeof(value)) != FLUID_OK)
            {
                return FLUID_FAILED;
            }

            offset = FLUID_LE16TOH(value);

            if(offset > 127)
            {
                FLUID_LOG(FLUID_ERR, "RMIDI DBNK bank offset %u is out of range", offset);
                return FLUID_FAILED;
            }

            *bank_offset = offset;
            *have_bank_offset = TRUE;
        }

        start = start + 8 + size + (size & 1);
    }

    return FLUID_OK;
}

/*
 * Parse an RMIDI container.
 *
 * @return 1 if the file is an RMIDI file and was parsed successfully, 0 if the
 *  file is not an RMIDI file, or -1 if the RMIDI file is malformed.
 */
static int fluid_rmidi_parse_impl(const fluid_rmidi_src_t *src, fluid_rmidi_info_t *info)
{
    uint32_t header[3];
    fluid_long_long_t filesize = src->size;
    fluid_long_long_t end, pos;
    uint32_t riff_size;
    int bank_offset = 0;
    int have_bank_offset = FALSE;

    FLUID_MEMSET(info, 0, sizeof(*info));

    if(filesize < (fluid_long_long_t)sizeof(header))
    {
        return 0;
    }

    if(fluid_rmidi_read_at(src, 0, filesize, header, sizeof(header)) != FLUID_OK)
    {
        return 0;
    }

    if(header[0] != RIFF_FCC || header[2] != RMID_FCC)
    {
        return 0;
    }

    info->is_rmidi = TRUE;

    riff_size = FLUID_LE32TOH(header[1]);

    if((fluid_long_long_t)riff_size > filesize - 8)
    {
        FLUID_LOG(FLUID_ERR, "RMIDI file size mismatch");
        return -1;
    }

    end = (fluid_long_long_t)riff_size + 8;
    pos = 12;

    /* The first chunk must contain the MIDI data. */
    {
        uint32_t id, size;

        if(fluid_rmidi_read_chunk(src, pos, end, &id, &size) != FLUID_OK ||
                id != DATA_FCC || size == 0)
        {
            FLUID_LOG(FLUID_ERR, "RMIDI file does not start with a valid data chunk");
            return -1;
        }

        info->midi_offset = pos + 8;
        info->midi_size = size;
        pos = pos + 8 + size + (size & 1);
    }

    /* Scan the remaining chunks. Unknown chunks are ignored, as required by
     * the specification. */
    while(pos + 8 <= end)
    {
        uint32_t id, size;

        if(fluid_rmidi_read_chunk(src, pos, end, &id, &size) != FLUID_OK)
        {
            FLUID_LOG(FLUID_ERR, "RMIDI chunk size exceeds file size");
            return -1;
        }

        if(id == LIST_FCC)
        {
            uint32_t type;

            if(size < 4)
            {
                FLUID_LOG(FLUID_ERR, "RMIDI LIST chunk is too small");
                return -1;
            }

            if(fluid_rmidi_read_at(src, pos + 8, end, &type, sizeof(type)) != FLUID_OK)
            {
                return -1;
            }

            if(type == INFO_FCC &&
                    fluid_rmidi_parse_info(src, pos + 12, pos + 8 + size,
                                           &bank_offset, &have_bank_offset) != FLUID_OK)
            {
                return -1;
            }
        }
        else if(id == RIFF_FCC)
        {
            uint32_t form_id;

            if(size < 4)
            {
                FLUID_LOG(FLUID_ERR, "RMIDI embedded RIFF chunk is too small");
                return -1;
            }

            if(fluid_rmidi_read_at(src, pos + 8, end, &form_id, sizeof(form_id)) != FLUID_OK)
            {
                return -1;
            }

            if(form_id == SFBK_FCC || form_id == DLS_FCC)
            {
                info->bank_type = (form_id == SFBK_FCC) ? FLUID_RMIDI_BANK_SF2
                                  : FLUID_RMIDI_BANK_DLS;
                info->bank_pos = pos;
                info->bank_size = 8 + (fluid_long_long_t)size;
                info->bank_offset = have_bank_offset ? bank_offset
                                    : FLUID_RMIDI_DEFAULT_BANK_OFFSET;

                /* The embedded bank is the last chunk of an RMIDI file. */
                break;
            }
        }

        pos = pos + 8 + size + (size & 1);
    }

    /* Without an embedded bank the DBNK chunk is ignored and no offset is
     * applied. */
    if(info->bank_type == FLUID_RMIDI_BANK_NONE)
    {
        info->bank_offset = 0;
    }

    return 1;
}

/* Read callback for a FILE based RMIDI file. */
static int fluid_rmidi_fp_read(void *buf, fluid_long_long_t count, void *handle)
{
    return fluid_file_read(buf, count, (FILE *)handle);
}

/* Seek callback for a FILE based RMIDI file. */
static int fluid_rmidi_fp_seek(void *handle, fluid_long_long_t offset, int whence)
{
    return fluid_file_seek((FILE *)handle, offset, whence);
}

static const fluid_file_callbacks_t fluid_rmidi_fp_callbacks =
{
    NULL,
    fluid_rmidi_fp_read,
    fluid_rmidi_fp_seek,
    NULL,
    NULL
};

/* Check whether a memory buffer starts with an RMIDI header. */
int fluid_rmidi_is_rmidi_mem(const void *buffer, size_t length)
{
    uint32_t header[3];

    if(buffer == NULL || length < sizeof(header))
    {
        return FALSE;
    }

    FLUID_MEMCPY(header, buffer, sizeof(header));
    return header[0] == RIFF_FCC && header[2] == RMID_FCC;
}

/* Parse an RMIDI file that is already loaded into memory. */
int fluid_rmidi_parse_mem(const void *buffer, size_t length, fluid_rmidi_info_t *info)
{
    fluid_rmidi_src_t src = { (const unsigned char *)buffer, NULL, NULL, (fluid_long_long_t)length };

    if(buffer == NULL || info == NULL)
    {
        return 0;
    }

    return fluid_rmidi_parse_impl(&src, info);
}

/* Parse an RMIDI file through the given file callbacks. */
int fluid_rmidi_parse(const fluid_file_callbacks_t *fcbs, void *file,
                      fluid_long_long_t filesize, fluid_rmidi_info_t *info)
{
    fluid_rmidi_src_t src = { NULL, fcbs, file, filesize };

    if(fcbs == NULL || file == NULL || info == NULL)
    {
        return 0;
    }

    return fluid_rmidi_parse_impl(&src, info);
}

/* Parse an RMIDI file from a standard FILE handle. */
int fluid_rmidi_parse_fp(FILE *fp, fluid_long_long_t filesize, fluid_rmidi_info_t *info)
{
    return fluid_rmidi_parse(&fluid_rmidi_fp_callbacks, fp, filesize, info);
}

/*
 * Seek to the start of the sound bank in an open file. For an RMIDI file this is
 * the embedded bank, for any other file this is the start of the file.
 *
 * @return FLUID_FAILED if the file is a malformed RMIDI file, or contains a
 *  bank of another type than the requested one, FLUID_OK otherwise.
 */
int fluid_rmidi_seek_bank(const fluid_file_callbacks_t *fcbs, void *file,
                          fluid_long_long_t filesize, fluid_rmidi_bank_type type,
                          fluid_rmidi_info_t *info)
{
    fluid_long_long_t pos = 0;
    int status = fluid_rmidi_parse(fcbs, file, filesize, info);

    if(status < 0)
    {
        return FLUID_FAILED;
    }

    if(status > 0)
    {
        if(info->bank_type != type)
        {
            FLUID_LOG(FLUID_DBG, "RMIDI file does not contain the expected embedded bank");
            return FLUID_FAILED;
        }

        pos = info->bank_pos;
    }

    return fcbs->fseek(file, pos, SEEK_SET);
}
