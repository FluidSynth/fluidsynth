/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont file loading code borrowed from Smurf SoundFont Editor
 * Copyright (C) 1999-2001 Josh Green
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


#include "fluid_sffile.h"
#include "fluid_sfont.h"
#include "fluid_sys.h"

/*=================================sfload.c========================
  Borrowed from Smurf SoundFont Editor by Josh Green
  =================================================================*/

/*
   functions for loading data from sfont files, with appropriate byte swapping
   on big endian machines. Sfont IDs are not swapped because the ID read is
   equivalent to the matching ID list in memory regardless of LE/BE machine
*/

/* sf file chunk IDs */
enum
{
    UNKN_ID,
    RIFF_ID,
    LIST_ID,
    SFBK_ID,
    INFO_ID,
    SDTA_ID,
    PDTA_ID, /* info/sample/preset */

    IFIL_ID,
    ISNG_ID,
    INAM_ID,
    IROM_ID, /* info ids (1st byte of info strings) */
    IVER_ID,
    ICRD_ID,
    IENG_ID,
    IPRD_ID, /* more info ids */
    ICOP_ID,
    ICMT_ID,
    ISFT_ID, /* and yet more info ids */

    SNAM_ID,
    SMPL_ID, /* sample ids */
    PHDR_ID,
    PBAG_ID,
    PMOD_ID,
    PGEN_ID, /* preset ids */
    IHDR_ID,
    IBAG_ID,
    IMOD_ID,
    IGEN_ID, /* instrument ids */
    SHDR_ID, /* sample info */
    SM24_ID
};

static const char idlist[] = {"RIFFLISTsfbkINFOsdtapdtaifilisngINAMiromiverICRDIENGIPRD"
                              "ICOPICMTISFTsnamsmplphdrpbagpmodpgeninstibagimodigenshdrsm24"};


/* generator types */
typedef enum {
    Gen_StartAddrOfs,
    Gen_EndAddrOfs,
    Gen_StartLoopAddrOfs,
    Gen_EndLoopAddrOfs,
    Gen_StartAddrCoarseOfs,
    Gen_ModLFO2Pitch,
    Gen_VibLFO2Pitch,
    Gen_ModEnv2Pitch,
    Gen_FilterFc,
    Gen_FilterQ,
    Gen_ModLFO2FilterFc,
    Gen_ModEnv2FilterFc,
    Gen_EndAddrCoarseOfs,
    Gen_ModLFO2Vol,
    Gen_Unused1,
    Gen_ChorusSend,
    Gen_ReverbSend,
    Gen_Pan,
    Gen_Unused2,
    Gen_Unused3,
    Gen_Unused4,
    Gen_ModLFODelay,
    Gen_ModLFOFreq,
    Gen_VibLFODelay,
    Gen_VibLFOFreq,
    Gen_ModEnvDelay,
    Gen_ModEnvAttack,
    Gen_ModEnvHold,
    Gen_ModEnvDecay,
    Gen_ModEnvSustain,
    Gen_ModEnvRelease,
    Gen_Key2ModEnvHold,
    Gen_Key2ModEnvDecay,
    Gen_VolEnvDelay,
    Gen_VolEnvAttack,
    Gen_VolEnvHold,
    Gen_VolEnvDecay,
    Gen_VolEnvSustain,
    Gen_VolEnvRelease,
    Gen_Key2VolEnvHold,
    Gen_Key2VolEnvDecay,
    Gen_Instrument,
    Gen_Reserved1,
    Gen_KeyRange,
    Gen_VelRange,
    Gen_StartLoopAddrCoarseOfs,
    Gen_Keynum,
    Gen_Velocity,
    Gen_Attenuation,
    Gen_Reserved2,
    Gen_EndLoopAddrCoarseOfs,
    Gen_CoarseTune,
    Gen_FineTune,
    Gen_SampleId,
    Gen_SampleModes,
    Gen_Reserved3,
    Gen_ScaleTune,
    Gen_ExclusiveClass,
    Gen_OverrideRootKey,
    Gen_Dummy
} Gen_Type;

#define Gen_MaxValid Gen_Dummy - 1 /* maximum valid generator */
#define Gen_Count Gen_Dummy /* count of generators */
#define GenArrSize sizeof(SFGenAmount) * Gen_Count /* gen array size */


static const unsigned short invalid_inst_gen[] = {
    Gen_Unused1,
    Gen_Unused2,
    Gen_Unused3,
    Gen_Unused4,
    Gen_Reserved1,
    Gen_Reserved2,
    Gen_Reserved3,
    0
};

static const unsigned short invalid_preset_gen[] = {
    Gen_StartAddrOfs,
    Gen_EndAddrOfs,
    Gen_StartLoopAddrOfs,
    Gen_EndLoopAddrOfs,
    Gen_StartAddrCoarseOfs,
    Gen_EndAddrCoarseOfs,
    Gen_StartLoopAddrCoarseOfs,
    Gen_Keynum,
    Gen_Velocity,
    Gen_EndLoopAddrCoarseOfs,
    Gen_SampleModes,
    Gen_ExclusiveClass,
    Gen_OverrideRootKey,
    0
};


#define CHNKIDSTR(id) &idlist[(id - 1) * 4]

/* sfont file chunk sizes */
#define SF_PHDR_SIZE (38)
#define SF_BAG_SIZE  (4)
#define SF_MOD_SIZE  (10)
#define SF_GEN_SIZE  (4)
#define SF_IHDR_SIZE (22)
#define SF_SHDR_SIZE (46)


#define READCHUNK(sf, var)                                                  \
    do                                                                      \
    {                                                                       \
        if (sf->fcbs->fread(var, 8, sf->sffd) == FLUID_FAILED)              \
            return FALSE;                                                   \
        ((SFChunk *)(var))->size = FLUID_LE32TOH(((SFChunk *)(var))->size); \
    } while (0)

#define READD(sf, var)                                            \
    do                                                            \
    {                                                             \
        uint32_t _temp;                                           \
        if (sf->fcbs->fread(&_temp, 4, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                         \
        var = FLUID_LE32TOH(_temp);                               \
    } while (0)

#define READW(sf, var)                                            \
    do                                                            \
    {                                                             \
        uint16_t _temp;                                           \
        if (sf->fcbs->fread(&_temp, 2, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                         \
        var = FLUID_LE16TOH(_temp);                               \
    } while (0)

#define READID(sf, var)                                        \
    do                                                         \
    {                                                          \
        if (sf->fcbs->fread(var, 4, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                      \
    } while (0)

#define READSTR(sf, var)                                        \
    do                                                          \
    {                                                           \
        if (sf->fcbs->fread(var, 20, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                       \
        (*var)[20] = '\0';                                      \
    } while (0)

#define READB(sf, var)                                          \
    do                                                          \
    {                                                           \
        if (sf->fcbs->fread(&var, 1, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                       \
    } while (0)

#define FSKIP(sf, size)                                                \
    do                                                                 \
    {                                                                  \
        if (sf->fcbs->fseek(sf->sffd, size, SEEK_CUR) == FLUID_FAILED) \
            return FALSE;                                              \
    } while (0)

#define FSKIPW(sf)                                                  \
    do                                                              \
    {                                                               \
        if (sf->fcbs->fseek(sf->sffd, 2, SEEK_CUR) == FLUID_FAILED) \
            return FALSE;                                           \
    } while (0)

/* removes and advances a fluid_list_t pointer */
#define SLADVREM(list, item)                        \
    do                                              \
    {                                               \
        fluid_list_t *_temp = item;                 \
        item = fluid_list_next(item);               \
        list = fluid_list_remove_link(list, _temp); \
        delete1_fluid_list(_temp);                  \
    } while (0)

static int load_body(SFData *sf, unsigned int size);
static int process_info(SFData *sf, int size);
static int process_sdta(SFData *sf, unsigned int size);
static int process_pdta(SFData *sf, int size);
static int load_phdr(SFData *sf, int size);
static int load_pbag(SFData *sf, int size);
static int load_pmod(SFData *sf, int size);
static int load_pgen(SFData *sf, int size);
static int load_ihdr(SFData *sf, int size);
static int load_ibag(SFData *sf, int size);
static int load_imod(SFData *sf, int size);
static int load_igen(SFData *sf, int size);
static int load_shdr(SFData *sf, unsigned int size);
static int fixup_pgen(SFData *sf);
static int fixup_igen(SFData *sf);
static int fixup_sample(SFData *sf);
static void free_zone(SFZone *zone);

static int chunkid(unsigned int id);
static int read_listchunk(SFData *sf, SFChunk *chunk);
static int pdtahelper(SFData *sf, unsigned int expid, unsigned int reclen, SFChunk *chunk, int *size);
static int preset_compare_func(void *a, void *b);
static fluid_list_t *find_gen_by_id(int gen, fluid_list_t *genlist);
static int valid_inst_genid(unsigned short genid);
static int valid_preset_genid(unsigned short genid);


/*
 * Open a SoundFont file and parse it's contents into a SFData structure.
 *
 * @param fname filename
 * @param fcbs file callback structure
 * @return the parsed SoundFont as SFData structure or NULL on error
 */
SFData *fluid_sf2_load(const char *fname, const fluid_file_callbacks_t *fcbs)
{
    SFData *sf;
    int fsize = 0;

    if (!(sf = FLUID_NEW(SFData)))
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(sf, 0, sizeof(SFData));

    sf->fcbs = fcbs;

    if ((sf->sffd = fcbs->fopen(fname)) == NULL)
    {
        FLUID_LOG(FLUID_ERR, _("Unable to open file \"%s\""), fname);
        goto error_exit;
    }

    sf->fname = FLUID_STRDUP(fname);
    if (sf->fname == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_exit;
    }

    /* get size of file by seeking to end */
    if (fcbs->fseek(sf->sffd, 0L, SEEK_END) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, _("Seek to end of file failed"));
        goto error_exit;
    }
    if ((fsize = fcbs->ftell(sf->sffd)) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, _("Get end of file position failed"));
        goto error_exit;
    }

    if (fcbs->fseek(sf->sffd, 0, SEEK_SET) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, _("Rewind to start of file failed"));
        goto error_exit;
    }

    if (!load_body(sf, fsize))
    {
        goto error_exit;
    }

    return sf;

error_exit:
    fluid_sf2_close(sf);
    return NULL;
}

/*
 * Close a SoundFont file and free the SFData structure.
 *
 * @param sf pointer to SFData structure
 * @param fcbs file callback structure
 */
void fluid_sf2_close(SFData *sf)
{
    fluid_list_t *p, *p2;

    if (sf->sffd)
        sf->fcbs->fclose(sf->sffd);

    if (sf->fname)
        FLUID_FREE(sf->fname);

    p = sf->info;
    while (p)
    {
        FLUID_FREE(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->info);
    sf->info = NULL;

    p = sf->preset;
    while (p)
    { /* loop over presets */
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2)
        { /* loop over preset's zones */
            free_zone(p2->data);
            p2 = fluid_list_next(p2);
        } /* free preset's zone list */
        delete_fluid_list(((SFPreset *)(p->data))->zone);
        FLUID_FREE(p->data); /* free preset chunk */
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->preset);
    sf->preset = NULL;

    p = sf->inst;
    while (p)
    { /* loop over instruments */
        p2 = ((SFInst *)(p->data))->zone;
        while (p2)
        { /* loop over inst's zones */
            free_zone(p2->data);
            p2 = fluid_list_next(p2);
        } /* free inst's zone list */
        delete_fluid_list(((SFInst *)(p->data))->zone);
        FLUID_FREE(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->inst);
    sf->inst = NULL;

    p = sf->sample;
    while (p)
    {
        FLUID_FREE(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->sample);
    sf->sample = NULL;

    FLUID_FREE(sf);
}


/*
 * Private functions
 */

/* sound font file load functions */
static int chunkid(unsigned int id)
{
    unsigned int i;
    unsigned int *p;

    p = (unsigned int *)&idlist;
    for (i = 0; i < sizeof(idlist) / sizeof(int); i++, p += 1)
        if (*p == id)
            return (i + 1);

    return UNKN_ID;
}

static int load_body(SFData *sf, unsigned int size)
{
    SFChunk chunk;

    READCHUNK(sf, &chunk); /* load RIFF chunk */
    if (chunkid(chunk.id) != RIFF_ID)
    { /* error if not RIFF */
        FLUID_LOG(FLUID_ERR, _("Not a RIFF file"));
        return FALSE;
    }

    READID(sf, &chunk.id); /* load file ID */
    if (chunkid(chunk.id) != SFBK_ID)
    { /* error if not SFBK_ID */
        FLUID_LOG(FLUID_ERR, _("Not a SoundFont file"));
        return FALSE;
    }

    if (chunk.size != size - 8)
    {
        FLUID_LOG(FLUID_ERR, _("SoundFont file size mismatch"));
        return FALSE;
    }

    /* Process INFO block */
    if (!read_listchunk(sf, &chunk))
        return FALSE;
    if (chunkid(chunk.id) != INFO_ID)
    {
        FLUID_LOG(FLUID_ERR, _("Invalid ID found when expecting INFO chunk"));
        return FALSE;
    }
    if (!process_info(sf, chunk.size))
        return FALSE;

    /* Process sample chunk */
    if (!read_listchunk(sf, &chunk))
        return FALSE;
    if (chunkid(chunk.id) != SDTA_ID)
    {
        FLUID_LOG(FLUID_ERR, _("Invalid ID found when expecting SAMPLE chunk"));
        return FALSE;
    }
    if (!process_sdta(sf, chunk.size))
        return FALSE;

    /* process HYDRA chunk */
    if (!read_listchunk(sf, &chunk))
        return FALSE;
    if (chunkid(chunk.id) != PDTA_ID)
    {
        FLUID_LOG(FLUID_ERR, _("Invalid ID found when expecting HYDRA chunk"));
        return FALSE;
    }
    if (!process_pdta(sf, chunk.size))
        return FALSE;

    if (!fixup_pgen(sf))
        return FALSE;
    if (!fixup_igen(sf))
        return FALSE;
    if (!fixup_sample(sf))
        return FALSE;

    /* sort preset list by bank, preset # */
    sf->preset = fluid_list_sort(sf->preset, (fluid_compare_func_t)preset_compare_func);

    return TRUE;
}

static int read_listchunk(SFData *sf, SFChunk *chunk)
{
    READCHUNK(sf, chunk); /* read list chunk */
    if (chunkid(chunk->id) != LIST_ID) /* error if ! list chunk */
    {
        FLUID_LOG(FLUID_ERR, _("Invalid chunk id in level 0 parse"));
        return FALSE;
    }
    READID(sf, &chunk->id); /* read id string */
    chunk->size -= 4;
    return TRUE;
}

static int process_info(SFData *sf, int size)
{
    SFChunk chunk;
    unsigned char id;
    char *item;
    unsigned short ver;

    while (size > 0)
    {
        READCHUNK(sf, &chunk);
        size -= 8;

        id = chunkid(chunk.id);

        if (id == IFIL_ID)
        { /* sound font version chunk? */
            if (chunk.size != 4)
            {
                FLUID_LOG(FLUID_ERR, _("Sound font version info chunk has invalid size"));
                return FALSE;
            }

            READW(sf, ver);
            sf->version.major = ver;
            READW(sf, ver);
            sf->version.minor = ver;

            if (sf->version.major < 2)
            {
                FLUID_LOG(FLUID_ERR, _("Sound font version is %d.%d which is not"
                                       " supported, convert to version 2.0x"),
                          sf->version.major, sf->version.minor);
                return FALSE;
            }

            if (sf->version.major == 3)
            {
#if !LIBSNDFILE_SUPPORT
                FLUID_LOG(FLUID_WARN,
                          _("Sound font version is %d.%d but fluidsynth was compiled without"
                            " support for (v3.x)"),
                          sf->version.major, sf->version.minor);
                return FALSE;
#endif
            }
            else if (sf->version.major > 2)
            {
                FLUID_LOG(FLUID_WARN,
                          _("Sound font version is %d.%d which is newer than"
                            " what this version of fluidsynth was designed for (v2.0x)"),
                          sf->version.major, sf->version.minor);
                return FALSE;
            }
        }
        else if (id == IVER_ID)
        { /* ROM version chunk? */
            if (chunk.size != 4)
            {
                FLUID_LOG(FLUID_ERR, _("ROM version info chunk has invalid size"));
                return FALSE;
            }

            READW(sf, ver);
            sf->romver.major = ver;
            READW(sf, ver);
            sf->romver.minor = ver;
        }
        else if (id != UNKN_ID)
        {
            if ((id != ICMT_ID && chunk.size > 256) || (chunk.size > 65536) || (chunk.size % 2))
            {
                FLUID_LOG(FLUID_ERR, _("INFO sub chunk %.4s has invalid chunk size of %d bytes"),
                          &chunk.id, chunk.size);
                return FALSE;
            }

            /* alloc for chunk id and da chunk */
            if (!(item = FLUID_MALLOC(chunk.size + 1)))
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                return FALSE;
            }

            /* attach to INFO list, fluid_sf2_close will cleanup if FAIL occurs */
            sf->info = fluid_list_append(sf->info, item);

            *(unsigned char *)item = id;
            if (sf->fcbs->fread(&item[1], chunk.size, sf->sffd) == FLUID_FAILED)
                return FALSE;

            /* force terminate info item (don't forget uint8 info ID) */
            *(item + chunk.size) = '\0';
        }
        else
        {
            FLUID_LOG(FLUID_ERR, _("Invalid chunk id in INFO chunk"));
            return FALSE;
        }
        size -= chunk.size;
    }

    if (size < 0)
    {
        FLUID_LOG(FLUID_ERR, _("INFO chunk size mismatch"));
        return FALSE;
    }

    return TRUE;
}

static int process_sdta(SFData *sf, unsigned int size)
{
    SFChunk chunk;

    if (size == 0)
        return TRUE; /* no sample data? */

    /* read sub chunk */
    READCHUNK(sf, &chunk);
    size -= 8;

    if (chunkid(chunk.id) != SMPL_ID)
    {
        FLUID_LOG(FLUID_ERR, _("Expected SMPL chunk found invalid id instead"));
        return FALSE;
    }

    /* SDTA chunk may also contain sm24 chunk for 24 bit samples
     * (not yet supported), only an error if SMPL chunk size is
     * greater than SDTA. */
    if (chunk.size > size)
    {
        FLUID_LOG(FLUID_ERR, _("SDTA chunk size mismatch"));
        return FALSE;
    }

    /* sample data follows */
    sf->samplepos = sf->fcbs->ftell(sf->sffd);

    /* used in fixup_sample() to check validity of sample headers */
    sf->samplesize = chunk.size;

    FSKIP(sf, chunk.size);
    size -= chunk.size;

    if (sf->version.major >= 2 && sf->version.minor >= 4)
    {
        /* any chance to find another chunk here? */
        if (size > 8)
        {
            /* read sub chunk */
            READCHUNK(sf, &chunk);
            size -= 8;

            if (chunkid(chunk.id) == SM24_ID)
            {
                int sm24size, sdtahalfsize;

                FLUID_LOG(FLUID_DBG, "Found SM24 chunk");
                if (chunk.size > size)
                {
                    FLUID_LOG(FLUID_WARN, "SM24 exeeds SDTA chunk, ignoring SM24");
                    goto ret; // no error
                }

                sdtahalfsize = sf->samplesize / 2;
                /* + 1 byte in the case that half the size of smpl chunk is an odd value */
                sdtahalfsize += sdtahalfsize % 2;
                sm24size = chunk.size;

                if (sdtahalfsize != sm24size)
                {
                    FLUID_LOG(FLUID_WARN, "SM24 not equal to half the size of SMPL chunk (0x%X != "
                                          "0x%X), ignoring SM24",
                              sm24size, sdtahalfsize);
                    goto ret; // no error
                }

                /* sample data24 follows */
                sf->sample24pos = sf->fcbs->ftell(sf->sffd);
                sf->sample24size = sm24size;
            }
        }
    }

ret:
    FSKIP(sf, size);

    return TRUE;
}

static int pdtahelper(SFData *sf, unsigned int expid, unsigned int reclen, SFChunk *chunk, int *size)
{
    unsigned int id;
    const char *expstr;

    expstr = CHNKIDSTR(expid); /* in case we need it */

    READCHUNK(sf, chunk);
    *size -= 8;

    if ((id = chunkid(chunk->id)) != expid)
    {
        FLUID_LOG(FLUID_ERR, _("Expected PDTA sub-chunk \"%.4s\" found invalid id instead"), expstr);
        return FALSE;
    }

    if (chunk->size % reclen) /* valid chunk size? */
    {
        FLUID_LOG(FLUID_ERR, _("\"%.4s\" chunk size is not a multiple of %d bytes"), expstr, reclen);
        return FALSE;
    }
    if ((*size -= chunk->size) < 0)
    {
        FLUID_LOG(FLUID_ERR, _("\"%.4s\" chunk size exceeds remaining PDTA chunk size"), expstr);
        return FALSE;
    }
    return TRUE;
}

static int process_pdta(SFData *sf, int size)
{
    SFChunk chunk;

    if (!pdtahelper(sf, PHDR_ID, SF_PHDR_SIZE, &chunk, &size))
        return FALSE;
    if (!load_phdr(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, PBAG_ID, SF_BAG_SIZE, &chunk, &size))
        return FALSE;
    if (!load_pbag(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, PMOD_ID, SF_MOD_SIZE, &chunk, &size))
        return FALSE;
    if (!load_pmod(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, PGEN_ID, SF_GEN_SIZE, &chunk, &size))
        return FALSE;
    if (!load_pgen(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, IHDR_ID, SF_IHDR_SIZE, &chunk, &size))
        return FALSE;
    if (!load_ihdr(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, IBAG_ID, SF_BAG_SIZE, &chunk, &size))
        return FALSE;
    if (!load_ibag(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, IMOD_ID, SF_MOD_SIZE, &chunk, &size))
        return FALSE;
    if (!load_imod(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, IGEN_ID, SF_GEN_SIZE, &chunk, &size))
        return FALSE;
    if (!load_igen(sf, chunk.size))
        return FALSE;

    if (!pdtahelper(sf, SHDR_ID, SF_SHDR_SIZE, &chunk, &size))
        return FALSE;
    if (!load_shdr(sf, chunk.size))
        return FALSE;

    return TRUE;
}

/* preset header loader */
static int load_phdr(SFData *sf, int size)
{
    int i, i2;
    SFPreset *preset, *prev_preset = NULL;
    unsigned short pbag_idx, prev_pbag_idx = 0;

    if (size % SF_PHDR_SIZE || size == 0)
    {
        FLUID_LOG(FLUID_ERR, _("Preset header chunk size is invalid"));
        return FALSE;
    }

    i = size / SF_PHDR_SIZE - 1;
    if (i == 0)
    { /* at least one preset + term record */
        FLUID_LOG(FLUID_WARN, _("File contains no presets"));
        FSKIP(sf, SF_PHDR_SIZE);
        return TRUE;
    }

    for (; i > 0; i--)
    { /* load all preset headers */
        preset = FLUID_NEW(SFPreset);
        sf->preset = fluid_list_append(sf->preset, preset);
        preset->zone = NULL; /* In case of failure, fluid_sf2_close can cleanup */
        READSTR(sf, &preset->name); /* possible read failure ^ */
        READW(sf, preset->prenum);
        READW(sf, preset->bank);
        READW(sf, pbag_idx);
        READD(sf, preset->libr);
        READD(sf, preset->genre);
        READD(sf, preset->morph);

        if (prev_preset)
        { /* not first preset? */
            if (pbag_idx < prev_pbag_idx)
            {
                FLUID_LOG(FLUID_ERR, _("Preset header indices not monotonic"));
                return FALSE;
            }
            i2 = pbag_idx - prev_pbag_idx;
            while (i2--)
            {
                prev_preset->zone = fluid_list_prepend(prev_preset->zone, NULL);
            }
        }
        else if (pbag_idx > 0) /* 1st preset, warn if ofs >0 */
            FLUID_LOG(FLUID_WARN, _("%d preset zones not referenced, discarding"), pbag_idx);
        prev_preset = preset; /* update preset ptr */
        prev_pbag_idx = pbag_idx;
    }

    FSKIP(sf, 24);
    READW(sf, pbag_idx); /* Read terminal generator index */
    FSKIP(sf, 12);

    if (pbag_idx < prev_pbag_idx)
    {
        FLUID_LOG(FLUID_ERR, _("Preset header indices not monotonic"));
        return FALSE;
    }
    i2 = pbag_idx - prev_pbag_idx;
    while (i2--)
    {
        prev_preset->zone = fluid_list_prepend(prev_preset->zone, NULL);
    }

    return TRUE;
}

/* preset bag loader */
static int load_pbag(SFData *sf, int size)
{
    fluid_list_t *p, *p2;
    SFZone *z, *pz = NULL;
    unsigned short genndx, modndx;
    unsigned short pgenndx = 0, pmodndx = 0;
    unsigned short i;

    if (size % SF_BAG_SIZE || size == 0) /* size is multiple of SF_BAG_SIZE? */
    {
        FLUID_LOG(FLUID_ERR, _("Preset bag chunk size is invalid"));
        return FALSE;
    }

    p = sf->preset;
    while (p)
    { /* traverse through presets */
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2)
        { /* traverse preset's zones */
            if ((size -= SF_BAG_SIZE) < 0)
            {
                FLUID_LOG(FLUID_ERR, _("Preset bag chunk size mismatch"));
                return FALSE;
            }
            z = FLUID_NEW(SFZone);
            p2->data = z;
            z->gen = NULL; /* Init gen and mod before possible failure, */
            z->mod = NULL; /* to ensure proper cleanup (fluid_sf2_close) */
            READW(sf, genndx); /* possible read failure ^ */
            READW(sf, modndx);
            z->instsamp = NULL;

            if (pz)
            { /* if not first zone */
                if (genndx < pgenndx)
                {
                    FLUID_LOG(FLUID_ERR, _("Preset bag generator indices not monotonic"));
                    return FALSE;
                }
                if (modndx < pmodndx)
                {
                    FLUID_LOG(FLUID_ERR, _("Preset bag modulator indices not monotonic"));
                    return FALSE;
                }
                i = genndx - pgenndx;
                while (i--)
                    pz->gen = fluid_list_prepend(pz->gen, NULL);
                i = modndx - pmodndx;
                while (i--)
                    pz->mod = fluid_list_prepend(pz->mod, NULL);
            }
            pz = z; /* update previous zone ptr */
            pgenndx = genndx; /* update previous zone gen index */
            pmodndx = modndx; /* update previous zone mod index */
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    size -= SF_BAG_SIZE;
    if (size != 0)
    {
        FLUID_LOG(FLUID_ERR, _("Preset bag chunk size mismatch"));
        return FALSE;
    }

    READW(sf, genndx);
    READW(sf, modndx);

    if (!pz)
    {
        if (genndx > 0)
            FLUID_LOG(FLUID_WARN, _("No preset generators and terminal index not 0"));
        if (modndx > 0)
            FLUID_LOG(FLUID_WARN, _("No preset modulators and terminal index not 0"));
        return TRUE;
    }

    if (genndx < pgenndx)
    {
        FLUID_LOG(FLUID_ERR, _("Preset bag generator indices not monotonic"));
        return FALSE;
    }
    if (modndx < pmodndx)
    {
        FLUID_LOG(FLUID_ERR, _("Preset bag modulator indices not monotonic"));
        return FALSE;
    }
    i = genndx - pgenndx;
    while (i--)
        pz->gen = fluid_list_prepend(pz->gen, NULL);
    i = modndx - pmodndx;
    while (i--)
        pz->mod = fluid_list_prepend(pz->mod, NULL);

    return TRUE;
}

/* preset modulator loader */
static int load_pmod(SFData *sf, int size)
{
    fluid_list_t *p, *p2, *p3;
    SFMod *m;

    p = sf->preset;
    while (p)
    { /* traverse through all presets */
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2)
        { /* traverse this preset's zones */
            p3 = ((SFZone *)(p2->data))->mod;
            while (p3)
            { /* load zone's modulators */
                if ((size -= SF_MOD_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, _("Preset modulator chunk size mismatch"));
                    return FALSE;
                }
                m = FLUID_NEW(SFMod);
                p3->data = m;
                READW(sf, m->src);
                READW(sf, m->dest);
                READW(sf, m->amount);
                READW(sf, m->amtsrc);
                READW(sf, m->trans);
                p3 = fluid_list_next(p3);
            }
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    /*
       If there isn't even a terminal record
       Hmmm, the specs say there should be one, but..
     */
    if (size == 0)
        return TRUE;

    size -= SF_MOD_SIZE;
    if (size != 0)
    {
        FLUID_LOG(FLUID_ERR, _("Preset modulator chunk size mismatch"));
        return FALSE;
    }
    FSKIP(sf, SF_MOD_SIZE); /* terminal mod */

    return TRUE;
}

/* -------------------------------------------------------------------
 * preset generator loader
 * generator (per preset) loading rules:
 * Zones with no generators or modulators shall be annihilated
 * Global zone must be 1st zone, discard additional ones (instrumentless zones)
 *
 * generator (per zone) loading rules (in order of decreasing precedence):
 * KeyRange is 1st in list (if exists), else discard
 * if a VelRange exists only preceded by a KeyRange, else discard
 * if a generator follows an instrument discard it
 * if a duplicate generator exists replace previous one
 * ------------------------------------------------------------------- */
static int load_pgen(SFData *sf, int size)
{
    fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
    SFZone *z;
    SFGen *g;
    SFGenAmount genval;
    unsigned short genid;
    int level, skip, drop, gzone, discarded;

    p = sf->preset;
    while (p)
    { /* traverse through all presets */
        gzone = FALSE;
        discarded = FALSE;
        p2 = ((SFPreset *)(p->data))->zone;
        if (p2)
            hz = &p2;
        while (p2)
        { /* traverse preset's zones */
            level = 0;
            z = (SFZone *)(p2->data);
            p3 = z->gen;
            while (p3)
            { /* load zone's generators */
                dup = NULL;
                skip = FALSE;
                drop = FALSE;
                if ((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, _("Preset generator chunk size mismatch"));
                    return FALSE;
                }

                READW(sf, genid);

                if (genid == Gen_KeyRange)
                { /* nothing precedes */
                    if (level == 0)
                    {
                        level = 1;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                        skip = TRUE;
                }
                else if (genid == Gen_VelRange)
                { /* only KeyRange precedes */
                    if (level <= 1)
                    {
                        level = 2;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                        skip = TRUE;
                }
                else if (genid == Gen_Instrument)
                { /* inst is last gen */
                    level = 3;
                    READW(sf, genval.uword);
                    ((SFZone *)(p2->data))->instsamp = FLUID_INT_TO_POINTER(genval.uword + 1);
                    break; /* break out of generator loop */
                }
                else
                {
                    level = 2;
                    if (valid_preset_genid(genid))
                    { /* generator valid? */
                        READW(sf, genval.sword);
                        dup = find_gen_by_id(genid, z->gen);
                    }
                    else
                        skip = TRUE;
                }

                if (!skip)
                {
                    if (!dup)
                    { /* if gen ! dup alloc new */
                        g = FLUID_NEW(SFGen);
                        p3->data = g;
                        g->id = genid;
                    }
                    else
                    {
                        g = (SFGen *)(dup->data); /* ptr to orig gen */
                        drop = TRUE;
                    }
                    g->amount = genval;
                }
                else
                { /* Skip this generator */
                    discarded = TRUE;
                    drop = TRUE;
                    FSKIPW(sf);
                }

                if (!drop)
                    p3 = fluid_list_next(p3); /* next gen */
                else
                    SLADVREM(z->gen, p3); /* drop place holder */

            } /* generator loop */

            if (level == 3)
                SLADVREM(z->gen, p3); /* zone has inst? */
            else
            { /* congratulations its a global zone */
                if (!gzone)
                { /* Prior global zones? */
                    gzone = TRUE;

                    /* if global zone is not 1st zone, relocate */
                    if (*hz != p2)
                    {
                        void *save = p2->data;
                        FLUID_LOG(FLUID_WARN, _("Preset \"%s\": Global zone is not first zone"),
                                  ((SFPreset *)(p->data))->name);
                        SLADVREM(*hz, p2);
                        *hz = fluid_list_prepend(*hz, save);
                        continue;
                    }
                }
                else
                { /* previous global zone exists, discard */
                    FLUID_LOG(FLUID_WARN, _("Preset \"%s\": Discarding invalid global zone"),
                              ((SFPreset *)(p->data))->name);
                    *hz = fluid_list_remove(*hz, p2->data);
                    free_zone((SFZone *)fluid_list_get(p2));
                }
            }

            while (p3)
            { /* Kill any zones following an instrument */
                discarded = TRUE;
                if ((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, _("Preset generator chunk size mismatch"));
                    return FALSE;
                }
                FSKIP(sf, SF_GEN_SIZE);
                SLADVREM(z->gen, p3);
            }

            p2 = fluid_list_next(p2); /* next zone */
        }
        if (discarded)
            FLUID_LOG(FLUID_WARN, _("Preset \"%s\": Some invalid generators were discarded"),
                      ((SFPreset *)(p->data))->name);
        p = fluid_list_next(p);
    }

    /* in case there isn't a terminal record */
    if (size == 0)
        return TRUE;

    size -= SF_GEN_SIZE;
    if (size != 0)
    {
        FLUID_LOG(FLUID_ERR, _("Preset generator chunk size mismatch"));
        return FALSE;
    }
    FSKIP(sf, SF_GEN_SIZE); /* terminal gen */

    return TRUE;
}

/* instrument header loader */
static int load_ihdr(SFData *sf, int size)
{
    int i, i2;
    SFInst *p, *pr = NULL; /* ptr to current & previous instrument */
    unsigned short zndx, pzndx = 0;

    if (size % SF_IHDR_SIZE || size == 0) /* chunk size is valid? */
    {
        FLUID_LOG(FLUID_ERR, _("Instrument header has invalid size"));
        return FALSE;
    }

    size = size / SF_IHDR_SIZE - 1;
    if (size == 0)
    { /* at least one preset + term record */
        FLUID_LOG(FLUID_WARN, _("File contains no instruments"));
        FSKIP(sf, SF_IHDR_SIZE);
        return TRUE;
    }

    for (i = 0; i < size; i++)
    { /* load all instrument headers */
        p = FLUID_NEW(SFInst);
        sf->inst = fluid_list_append(sf->inst, p);
        p->zone = NULL; /* For proper cleanup if fail (fluid_sf2_close) */
        READSTR(sf, &p->name); /* Possible read failure ^ */
        READW(sf, zndx);

        if (pr)
        { /* not first instrument? */
            if (zndx < pzndx)
            {
                FLUID_LOG(FLUID_ERR, _("Instrument header indices not monotonic"));
                return FALSE;
            }
            i2 = zndx - pzndx;
            while (i2--)
                pr->zone = fluid_list_prepend(pr->zone, NULL);
        }
        else if (zndx > 0) /* 1st inst, warn if ofs >0 */
            FLUID_LOG(FLUID_WARN, _("%d instrument zones not referenced, discarding"), zndx);
        pzndx = zndx;
        pr = p; /* update instrument ptr */
    }

    FSKIP(sf, 20);
    READW(sf, zndx);

    if (zndx < pzndx)
    {
        FLUID_LOG(FLUID_ERR, _("Instrument header indices not monotonic"));
        return FALSE;
    }
    i2 = zndx - pzndx;
    while (i2--)
        pr->zone = fluid_list_prepend(pr->zone, NULL);

    return TRUE;
}

/* instrument bag loader */
static int load_ibag(SFData *sf, int size)
{
    fluid_list_t *p, *p2;
    SFZone *z, *pz = NULL;
    unsigned short genndx, modndx, pgenndx = 0, pmodndx = 0;
    int i;

    if (size % SF_BAG_SIZE || size == 0) /* size is multiple of SF_BAG_SIZE? */
    {
        FLUID_LOG(FLUID_ERR, _("Instrument bag chunk size is invalid"));
        return FALSE;
    }

    p = sf->inst;
    while (p)
    { /* traverse through inst */
        p2 = ((SFInst *)(p->data))->zone;
        while (p2)
        { /* load this inst's zones */
            if ((size -= SF_BAG_SIZE) < 0)
            {
                FLUID_LOG(FLUID_ERR, _("Instrument bag chunk size mismatch"));
                return FALSE;
            }
            z = FLUID_NEW(SFZone);
            p2->data = z;
            z->gen = NULL; /* In case of failure, */
            z->mod = NULL; /* fluid_sf2_close can clean up */
            READW(sf, genndx); /* READW = possible read failure */
            READW(sf, modndx);
            z->instsamp = NULL;

            if (pz)
            { /* if not first zone */
                if (genndx < pgenndx)
                {
                    FLUID_LOG(FLUID_ERR, _("Instrument generator indices not monotonic"));
                    return FALSE;
                }
                if (modndx < pmodndx)
                {
                    FLUID_LOG(FLUID_ERR, _("Instrument modulator indices not monotonic"));
                    return FALSE;
                }
                i = genndx - pgenndx;
                while (i--)
                    pz->gen = fluid_list_prepend(pz->gen, NULL);
                i = modndx - pmodndx;
                while (i--)
                    pz->mod = fluid_list_prepend(pz->mod, NULL);
            }
            pz = z; /* update previous zone ptr */
            pgenndx = genndx;
            pmodndx = modndx;
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    size -= SF_BAG_SIZE;
    if (size != 0)
    {
        FLUID_LOG(FLUID_ERR, _("Instrument chunk size mismatch"));
        return FALSE;
    }

    READW(sf, genndx);
    READW(sf, modndx);

    if (!pz)
    { /* in case that all are no zoners */
        if (genndx > 0)
            FLUID_LOG(FLUID_WARN, _("No instrument generators and terminal index not 0"));
        if (modndx > 0)
            FLUID_LOG(FLUID_WARN, _("No instrument modulators and terminal index not 0"));
        return TRUE;
    }

    if (genndx < pgenndx)
    {
        FLUID_LOG(FLUID_ERR, _("Instrument generator indices not monotonic"));
        return FALSE;
    }
    if (modndx < pmodndx)
    {
        FLUID_LOG(FLUID_ERR, _("Instrument modulator indices not monotonic"));
        return FALSE;
    }
    i = genndx - pgenndx;
    while (i--)
        pz->gen = fluid_list_prepend(pz->gen, NULL);
    i = modndx - pmodndx;
    while (i--)
        pz->mod = fluid_list_prepend(pz->mod, NULL);

    return TRUE;
}

/* instrument modulator loader */
static int load_imod(SFData *sf, int size)
{
    fluid_list_t *p, *p2, *p3;
    SFMod *m;

    p = sf->inst;
    while (p)
    { /* traverse through all inst */
        p2 = ((SFInst *)(p->data))->zone;
        while (p2)
        { /* traverse this inst's zones */
            p3 = ((SFZone *)(p2->data))->mod;
            while (p3)
            { /* load zone's modulators */
                if ((size -= SF_MOD_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, _("Instrument modulator chunk size mismatch"));
                    return FALSE;
                }
                m = FLUID_NEW(SFMod);
                p3->data = m;
                READW(sf, m->src);
                READW(sf, m->dest);
                READW(sf, m->amount);
                READW(sf, m->amtsrc);
                READW(sf, m->trans);
                p3 = fluid_list_next(p3);
            }
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    /*
       If there isn't even a terminal record
       Hmmm, the specs say there should be one, but..
     */
    if (size == 0)
        return TRUE;

    size -= SF_MOD_SIZE;
    if (size != 0)
    {
        FLUID_LOG(FLUID_ERR, _("Instrument modulator chunk size mismatch"));
        return FALSE;
    }
    FSKIP(sf, SF_MOD_SIZE); /* terminal mod */

    return TRUE;
}

/* load instrument generators (see load_pgen for loading rules) */
static int load_igen(SFData *sf, int size)
{
    fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
    SFZone *z;
    SFGen *g;
    SFGenAmount genval;
    unsigned short genid;
    int level, skip, drop, gzone, discarded;

    p = sf->inst;
    while (p)
    { /* traverse through all instruments */
        gzone = FALSE;
        discarded = FALSE;
        p2 = ((SFInst *)(p->data))->zone;
        if (p2)
            hz = &p2;
        while (p2)
        { /* traverse this instrument's zones */
            level = 0;
            z = (SFZone *)(p2->data);
            p3 = z->gen;
            while (p3)
            { /* load zone's generators */
                dup = NULL;
                skip = FALSE;
                drop = FALSE;
                if ((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, _("IGEN chunk size mismatch"));
                    return FALSE;
                }

                READW(sf, genid);

                if (genid == Gen_KeyRange)
                { /* nothing precedes */
                    if (level == 0)
                    {
                        level = 1;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                        skip = TRUE;
                }
                else if (genid == Gen_VelRange)
                { /* only KeyRange precedes */
                    if (level <= 1)
                    {
                        level = 2;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                        skip = TRUE;
                }
                else if (genid == Gen_SampleId)
                { /* sample is last gen */
                    level = 3;
                    READW(sf, genval.uword);
                    ((SFZone *)(p2->data))->instsamp = FLUID_INT_TO_POINTER(genval.uword + 1);
                    break; /* break out of generator loop */
                }
                else
                {
                    level = 2;
                    if (valid_inst_genid(genid))
                    { /* gen valid? */
                        READW(sf, genval.sword);
                        dup = find_gen_by_id(genid, z->gen);
                    }
                    else
                        skip = TRUE;
                }

                if (!skip)
                {
                    if (!dup)
                    { /* if gen ! dup alloc new */
                        g = FLUID_NEW(SFGen);
                        p3->data = g;
                        g->id = genid;
                    }
                    else
                    {
                        g = (SFGen *)(dup->data);
                        drop = TRUE;
                    }
                    g->amount = genval;
                }
                else
                { /* skip this generator */
                    discarded = TRUE;
                    drop = TRUE;
                    FSKIPW(sf);
                }

                if (!drop)
                    p3 = fluid_list_next(p3); /* next gen */
                else
                    SLADVREM(z->gen, p3);

            } /* generator loop */

            if (level == 3)
                SLADVREM(z->gen, p3); /* zone has sample? */
            else
            { /* its a global zone */
                if (!gzone)
                {
                    gzone = TRUE;

                    /* if global zone is not 1st zone, relocate */
                    if (*hz != p2)
                    {
                        void *save = p2->data;
                        FLUID_LOG(FLUID_WARN, _("Instrument \"%s\": Global zone is not first zone"),
                                  ((SFPreset *)(p->data))->name);
                        SLADVREM(*hz, p2);
                        *hz = fluid_list_prepend(*hz, save);
                        continue;
                    }
                }
                else
                { /* previous global zone exists, discard */
                    FLUID_LOG(FLUID_WARN, _("Instrument \"%s\": Discarding invalid global zone"),
                              ((SFInst *)(p->data))->name);
                    *hz = fluid_list_remove(*hz, p2->data);
                    free_zone((SFZone *)fluid_list_get(p2));
                }
            }

            while (p3)
            { /* Kill any zones following a sample */
                discarded = TRUE;
                if ((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, _("Instrument generator chunk size mismatch"));
                    return FALSE;
                }
                FSKIP(sf, SF_GEN_SIZE);
                SLADVREM(z->gen, p3);
            }

            p2 = fluid_list_next(p2); /* next zone */
        }
        if (discarded)
            FLUID_LOG(FLUID_WARN, _("Instrument \"%s\": Some invalid generators were discarded"),
                      ((SFInst *)(p->data))->name);
        p = fluid_list_next(p);
    }

    /* for those non-terminal record cases, grr! */
    if (size == 0)
        return TRUE;

    size -= SF_GEN_SIZE;
    if (size != 0)
    {
        FLUID_LOG(FLUID_ERR, _("IGEN chunk size mismatch"));
        return FALSE;
    }
    FSKIP(sf, SF_GEN_SIZE); /* terminal gen */

    return TRUE;
}

/* sample header loader */
static int load_shdr(SFData *sf, unsigned int size)
{
    unsigned int i;
    SFSample *p;

    if (size % SF_SHDR_SIZE || size == 0) /* size is multiple of SHDR size? */
    {
        FLUID_LOG(FLUID_ERR, _("Sample header has invalid size"));
        return FALSE;
    }

    size = size / SF_SHDR_SIZE - 1;
    if (size == 0)
    { /* at least one sample + term record? */
        FLUID_LOG(FLUID_WARN, _("File contains no samples"));
        FSKIP(sf, SF_SHDR_SIZE);
        return TRUE;
    }

    /* load all sample headers */
    for (i = 0; i < size; i++)
    {
        p = FLUID_NEW(SFSample);
        sf->sample = fluid_list_append(sf->sample, p);
        READSTR(sf, &p->name);
        READD(sf, p->start);
        READD(sf, p->end); /* - end, loopstart and loopend */
        READD(sf, p->loopstart); /* - will be checked and turned into */
        READD(sf, p->loopend); /* - offsets in fixup_sample() */
        READD(sf, p->samplerate);
        READB(sf, p->origpitch);
        READB(sf, p->pitchadj);
        FSKIPW(sf); /* skip sample link */
        READW(sf, p->sampletype);
        p->samfile = 0;
    }

    FSKIP(sf, SF_SHDR_SIZE); /* skip terminal shdr */

    return TRUE;
}

/* "fixup" (inst # -> inst ptr) instrument references in preset list */
static int fixup_pgen(SFData *sf)
{
    fluid_list_t *p, *p2, *p3;
    SFZone *z;
    int i;

    p = sf->preset;
    while (p)
    {
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2)
        { /* traverse this preset's zones */
            z = (SFZone *)(p2->data);
            if ((i = FLUID_POINTER_TO_INT(z->instsamp)))
            { /* load instrument # */
                p3 = fluid_list_nth(sf->inst, i - 1);
                if (!p3)
                {
                    FLUID_LOG(FLUID_ERR, _("Preset %03d %03d: Invalid instrument reference"),
                                 ((SFPreset *)(p->data))->bank, ((SFPreset *)(p->data))->prenum);
                    return FALSE;
                }
                z->instsamp = p3;
            }
            else
                z->instsamp = NULL;
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    return TRUE;
}

/* "fixup" (sample # -> sample ptr) sample references in instrument list */
static int fixup_igen(SFData *sf)
{
    fluid_list_t *p, *p2, *p3;
    SFZone *z;
    int i;

    p = sf->inst;
    while (p)
    {
        p2 = ((SFInst *)(p->data))->zone;
        while (p2)
        { /* traverse instrument's zones */
            z = (SFZone *)(p2->data);
            if ((i = FLUID_POINTER_TO_INT(z->instsamp)))
            { /* load sample # */
                p3 = fluid_list_nth(sf->sample, i - 1);
                if (!p3)
                {
                    FLUID_LOG(FLUID_ERR, _("Instrument \"%s\": Invalid sample reference"),
                            ((SFInst *)(p->data))->name);
                    return FALSE;
                }
                z->instsamp = p3;
            }
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    return TRUE;
}

/* convert sample end, loopstart and loopend to offsets and check if valid */
static int fixup_sample(SFData *sf)
{
    fluid_list_t *p;
    SFSample *sam;
    int invalid_loops = FALSE;
    int invalid_loopstart;
    int invalid_loopend, loopend_end_mismatch;
    unsigned int total_bytes = sf->samplesize;
    unsigned int total_samples = total_bytes / sizeof(short);

    p = sf->sample;
    while (p)
    {
        unsigned int max_end;

        sam = (SFSample *)(p->data);

        /* Standard SoundFont files (SF2) use sample word indices for sample start and end pointers,
         * but SF3 files with Ogg Vorbis compression use byte indices for start and end. */
        max_end = (sam->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS) ? total_bytes : total_samples;

        /* ROM samples are unusable for us by definition, so simply ignore them. */
        if (sam->sampletype & FLUID_SAMPLETYPE_ROM)
        {
            sam->start = sam->end = sam->loopstart = sam->loopend = 0;
            goto next_sample;
        }

        /* If end is over the sample data chunk or sam start is greater than 4
         * less than the end (at least 4 samples).
         *
         * FIXME: where does this number 4 come from? And do we need a different number for SF3
         * files?
         * Maybe we should check for the minimum Ogg Vorbis headers size? */
        if ((sam->end > max_end) || (sam->start > (sam->end - 4)))
        {
            FLUID_LOG(FLUID_WARN, _("Sample '%s' start/end file positions are invalid,"
                                    " disabling and will not be saved"),
                      sam->name);
            sam->start = sam->end = sam->loopstart = sam->loopend = 0;
            goto next_sample;
        }

        /* The SoundFont 2.4 spec defines the loopstart index as the first sample point of the loop
         */
        invalid_loopstart = (sam->loopstart < sam->start) || (sam->loopstart >= sam->loopend);
        /* while loopend is the first point AFTER the last sample of the loop.
         * this is as it should be. however we cannot be sure whether any of sam.loopend or sam.end
         * is correct. hours of thinking through this have concluded, that it would be best practice
         * to mangle with loops as little as necessary by only making sure loopend is within
         * max_end. incorrect soundfont shall preferably fail loudly. */
        invalid_loopend = (sam->loopend > max_end) || (sam->loopstart >= sam->loopend);

        loopend_end_mismatch = (sam->loopend > sam->end);

        if (sam->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS)
        {
            /*
             * compressed samples get fixed up after decompression
             *
             * however we cant use the logic below, because uncompressed samples are stored in
             * individual buffers
             */
        }
        else if (invalid_loopstart || invalid_loopend ||
                 loopend_end_mismatch) /* loop is fowled?? (cluck cluck :) */
        {
            /* though illegal, loopend may be set to loopstart to disable loop */
            /* is it worth informing the user? */
            invalid_loops |= (sam->loopend != sam->loopstart);

            /* force incorrect loop points into the sample range, ignore padding */
            if (invalid_loopstart)
            {
                FLUID_LOG(FLUID_DBG, _("Sample '%s' has unusable loop start '%d',"
                                       " setting to sample start at '%d'"),
                          sam->name, sam->loopstart, sam->start);
                sam->loopstart = sam->start;
            }

            if (invalid_loopend)
            {
                FLUID_LOG(FLUID_DBG, _("Sample '%s' has unusable loop stop '%d',"
                                       " setting to sample stop at '%d'"),
                          sam->name, sam->loopend, sam->end);
                /* since at this time sam->end points after valid sample data (will correct that few
                 * lines below),
                 * set loopend to that first invalid sample, since it should never be played, but
                 * instead the last
                 * valid sample will be played */
                sam->loopend = sam->end;
            }
            else if (loopend_end_mismatch)
            {
                FLUID_LOG(FLUID_DBG, _("Sample '%s' has invalid loop stop '%d',"
                                       " sample stop at '%d', using it anyway"),
                          sam->name, sam->loopend, sam->end);
            }
        }

        /* convert sample end, loopstart, loopend to offsets from sam->start */
        sam->end -= sam->start + 1; /* marks last sample, contrary to SF spec. */
        sam->loopstart -= sam->start;
        sam->loopend -= sam->start;

    next_sample:
        p = fluid_list_next(p);
    }

    if (invalid_loops)
    {
        FLUID_LOG(FLUID_WARN, _("Found samples with invalid loops, audible glitches possible."));
    }

    return TRUE;
}

/* free all elements of a zone (Preset or Instrument) */
static void free_zone(SFZone *zone)
{
    fluid_list_t *p;

    if (!zone)
        return;

    p = zone->gen;
    while (p)
    { /* Free gen chunks for this zone */
        if (p->data)
            FLUID_FREE(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(zone->gen); /* free genlist */

    p = zone->mod;
    while (p)
    { /* Free mod chunks for this zone */
        if (p->data)
            FLUID_FREE(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(zone->mod); /* free modlist */

    FLUID_FREE(zone); /* free zone chunk */
}

/* preset sort function, first by bank, then by preset # */
static int preset_compare_func(void *a, void *b)
{
    int aval, bval;

    aval = (int)(((SFPreset *)a)->bank) << 16 | ((SFPreset *)a)->prenum;
    bval = (int)(((SFPreset *)b)->bank) << 16 | ((SFPreset *)b)->prenum;

    return (aval - bval);
}

/* Find a generator by its id in the passed in list.
 *
 * @return pointer to SFGen if found, otherwise NULL
 */
static fluid_list_t *find_gen_by_id(int gen, fluid_list_t *genlist)
{ /* is generator in gen list? */
    fluid_list_t *p;

    p = genlist;
    while (p)
    {
        if (p->data == NULL)
            return NULL;
        if (gen == ((SFGen *)p->data)->id)
            break;
        p = fluid_list_next(p);
    }
    return p;
}

/* check validity of instrument generator */
static int valid_inst_genid(unsigned short genid)
{
    int i = 0;

    if (genid > Gen_MaxValid)
        return FALSE;
    while (invalid_inst_gen[i] && invalid_inst_gen[i] != genid)
        i++;
    return (invalid_inst_gen[i] == 0);
}

/* check validity of preset generator */
static int valid_preset_genid(unsigned short genid)
{
    int i = 0;

    if (!valid_inst_genid(genid))
        return FALSE;
    while (invalid_preset_gen[i] && invalid_preset_gen[i] != genid)
        i++;
    return (invalid_preset_gen[i] == 0);
}
