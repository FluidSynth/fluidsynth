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


#include "fluid_sys.h"
#include "fluid_sfont.h"
#include "fluid_sf2.h"

/*=================================sfload.c========================
  Borrowed from Smurf SoundFont Editor by Josh Green
  =================================================================*/

/*
   functions for loading data from sfont files, with appropriate byte swapping
   on big endian machines. Sfont IDs are not swapped because the ID read is
   equivalent to the matching ID list in memory regardless of LE/BE machine
*/

#define READCHUNK(var,fd, fcbs)	do {		\
	if (fcbs->fread(var, 8, fd) == FLUID_FAILED)			\
		return(FAIL);				\
	((SFChunk *)(var))->size = FLUID_LE32TOH(((SFChunk *)(var))->size);  \
} while(0)

#define READD(var,fd, fcbs)		do {		\
	uint32_t _temp;				\
	if (fcbs->fread(&_temp, 4, fd) == FLUID_FAILED)			\
		return(FAIL);				\
	var = FLUID_LE32TOH(_temp);			\
} while(0)

#define READW(var,fd, fcbs)		do {		\
	uint16_t _temp;				\
	if (fcbs->fread(&_temp, 2, fd) == FLUID_FAILED)			\
		return(FAIL);				\
	var = FLUID_LE16TOH(_temp);			\
} while(0)

#define READID(var,fd, fcbs)		do {		\
    if (fcbs->fread(var, 4, fd) == FLUID_FAILED)			\
	return(FAIL);					\
} while(0)

#define READSTR(var,fd, fcbs)		do {		\
    if (fcbs->fread(var, 20, fd) == FLUID_FAILED)			\
	return(FAIL);					\
    (*var)[20] = '\0';					\
} while(0)

#define READB(var,fd, fcbs)		do {		\
    if (fcbs->fread(&var, 1, fd) == FLUID_FAILED)			\
	return(FAIL);					\
} while(0)

#define FSKIP(size,fd, fcbs)		do {		\
    if (fcbs->fseek(fd, size, SEEK_CUR) == FLUID_FAILED)		\
	return(FAIL);					\
} while(0)

#define FSKIPW(fd, fcbs)		do {		\
    if (fcbs->fseek(fd, 2, SEEK_CUR) == FLUID_FAILED)			\
	return(FAIL);					\
} while(0)

/* removes and advances a fluid_list_t pointer */
#define SLADVREM(list, item)	do {		\
    fluid_list_t *_temp = item;				\
    item = fluid_list_next(item);				\
    list = fluid_list_remove_link(list, _temp);		\
    delete1_fluid_list(_temp);				\
} while(0)

static int chunkid (unsigned int id);
static int load_body (unsigned int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int read_listchunk (SFChunk * chunk, void * fd, const fluid_file_callbacks_t* fcbs);
static int process_info (int size, SFData * sf, void* fd, const fluid_file_callbacks_t* fcbs);
static int process_sdta (unsigned int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int pdtahelper (unsigned int expid, unsigned int reclen, SFChunk * chunk,
  int * size, void * fd, const fluid_file_callbacks_t* fcbs);
static int process_pdta (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_phdr (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_pbag (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_pmod (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_pgen (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_ihdr (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_ibag (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_imod (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_igen (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int load_shdr (unsigned int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs);
static int fixup_pgen (SFData * sf);
static int fixup_igen (SFData * sf);
static int fixup_sample (SFData * sf);

static const char idlist[] = {
  "RIFFLISTsfbkINFOsdtapdtaifilisngINAMiromiverICRDIENGIPRD"
    "ICOPICMTISFTsnamsmplphdrpbagpmodpgeninstibagimodigenshdrsm24"
};

/* sound font file load functions */
static int
chunkid (unsigned int id)
{
  unsigned int i;
  unsigned int *p;

  p = (unsigned int *) & idlist;
  for (i = 0; i < sizeof (idlist) / sizeof (int); i++, p += 1)
    if (*p == id)
      return (i + 1);

  return (UNKN_ID);
}

SFData *
sfload_file (const char * fname, const fluid_file_callbacks_t* fcbs)
{
  SFData *sf = NULL;
  void *fd;
  int fsize = 0;
  int err = FALSE;

  if ((fd = fcbs->fopen (fname)) == NULL)
    {
      FLUID_LOG (FLUID_ERR, _("Unable to open file \"%s\""), fname);
      return (NULL);
    }

  if (!(sf = FLUID_NEW (SFData)))
    {
      FLUID_LOG(FLUID_ERR, "Out of memory");
      fclose(fd);
      err = TRUE;
    }

  if (!err)
    {
      memset (sf, 0, sizeof (SFData));	/* zero sfdata */
      sf->fname = FLUID_STRDUP (fname);	/* copy file name */
      sf->sffd = fd;
    }

  /* get size of file */
  if (!err && fcbs->fseek (fd, 0L, SEEK_END) == FLUID_FAILED)
    {				/* seek to end of file */
      err = TRUE;
      FLUID_LOG (FLUID_ERR, _("Seek to end of file failed"));
    }
  if (!err && (fsize = fcbs->ftell (fd)) == FLUID_FAILED)
    {				/* position = size */
      err = TRUE;
      FLUID_LOG (FLUID_ERR, _("Get end of file position failed"));
    }
  if (!err)
    rewind (fd);

  if (!err && !load_body (fsize, sf, fd, fcbs))
    err = TRUE;			/* load the sfont */

  if (err)
    {
      if (sf)
	sfont_close (sf, fcbs);
      return (NULL);
    }

  return (sf);
}

static int
load_body (unsigned int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  SFChunk chunk;

  READCHUNK (&chunk, fd, fcbs);	/* load RIFF chunk */
  if (chunkid (chunk.id) != RIFF_ID) {	/* error if not RIFF */
    FLUID_LOG (FLUID_ERR, _("Not a RIFF file"));
    return (FAIL);
  }

  READID (&chunk.id, fd, fcbs);	/* load file ID */
  if (chunkid (chunk.id) != SFBK_ID) {	/* error if not SFBK_ID */
    FLUID_LOG (FLUID_ERR, _("Not a SoundFont file"));
    return (FAIL);
  }

  if (chunk.size != size - 8) {
    gerr (ErrCorr, _("SoundFont file size mismatch"));
    return (FAIL);
  }

  /* Process INFO block */
  if (!read_listchunk (&chunk, fd, fcbs))
    return (FAIL);
  if (chunkid (chunk.id) != INFO_ID)
    return (gerr (ErrCorr, _("Invalid ID found when expecting INFO chunk")));
  if (!process_info (chunk.size, sf, fd, fcbs))
    return (FAIL);

  /* Process sample chunk */
  if (!read_listchunk (&chunk, fd, fcbs))
    return (FAIL);
  if (chunkid (chunk.id) != SDTA_ID)
    return (gerr (ErrCorr,
	_("Invalid ID found when expecting SAMPLE chunk")));
  if (!process_sdta (chunk.size, sf, fd, fcbs))
    return (FAIL);

  /* process HYDRA chunk */
  if (!read_listchunk (&chunk, fd, fcbs))
    return (FAIL);
  if (chunkid (chunk.id) != PDTA_ID)
    return (gerr (ErrCorr, _("Invalid ID found when expecting HYDRA chunk")));
  if (!process_pdta (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!fixup_pgen (sf))
    return (FAIL);
  if (!fixup_igen (sf))
    return (FAIL);
  if (!fixup_sample (sf))
    return (FAIL);

  /* sort preset list by bank, preset # */
  sf->preset = fluid_list_sort (sf->preset,
    (fluid_compare_func_t) sfont_preset_compare_func);

  return (OK);
}

static int
read_listchunk (SFChunk * chunk, void * fd, const fluid_file_callbacks_t* fcbs)
{
  READCHUNK (chunk, fd, fcbs);	/* read list chunk */
  if (chunkid (chunk->id) != LIST_ID)	/* error if ! list chunk */
    return (gerr (ErrCorr, _("Invalid chunk id in level 0 parse")));
  READID (&chunk->id, fd, fcbs);	/* read id string */
  chunk->size -= 4;
  return (OK);
}

static int
process_info (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  SFChunk chunk;
  unsigned char id;
  char *item;
  unsigned short ver;

  while (size > 0)
    {
      READCHUNK (&chunk, fd, fcbs);
      size -= 8;

      id = chunkid (chunk.id);

      if (id == IFIL_ID)
	{			/* sound font version chunk? */
	  if (chunk.size != 4)
	    return (gerr (ErrCorr,
		_("Sound font version info chunk has invalid size")));

	  READW (ver, fd, fcbs);
	  sf->version.major = ver;
	  READW (ver, fd, fcbs);
	  sf->version.minor = ver;

	  if (sf->version.major < 2) {
	    FLUID_LOG (FLUID_ERR,
		      _("Sound font version is %d.%d which is not"
			" supported, convert to version 2.0x"),
		      sf->version.major,
		      sf->version.minor);
	    return (FAIL);
	  }

	  if (sf->version.major == 3) {
#if !LIBSNDFILE_SUPPORT
	    FLUID_LOG (FLUID_WARN,
		      _("Sound font version is %d.%d but fluidsynth was compiled without"
			" support for (v3.x)"),
		      sf->version.major,
		      sf->version.minor);
	    return (FAIL);
#endif
	  }
	  else if (sf->version.major > 2) {
	    FLUID_LOG (FLUID_WARN,
		      _("Sound font version is %d.%d which is newer than"
			" what this version of fluidsynth was designed for (v2.0x)"),
		      sf->version.major,
		      sf->version.minor);
	    return (FAIL);
	  }
	}
      else if (id == IVER_ID)
	{			/* ROM version chunk? */
	  if (chunk.size != 4)
	    return (gerr (ErrCorr,
		_("ROM version info chunk has invalid size")));

	  READW (ver, fd, fcbs);
	  sf->romver.major = ver;
	  READW (ver, fd, fcbs);
	  sf->romver.minor = ver;
	}
      else if (id != UNKN_ID)
	{
	  if ((id != ICMT_ID && chunk.size > 256) || (chunk.size > 65536)
	    || (chunk.size % 2))
	    return (gerr (ErrCorr,
		_("INFO sub chunk %.4s has invalid chunk size"
		  " of %d bytes"), &chunk.id, chunk.size));

	  /* alloc for chunk id and da chunk */
	  if (!(item = FLUID_MALLOC (chunk.size + 1)))
	    {
	      FLUID_LOG(FLUID_ERR, "Out of memory");
	      return (FAIL);
	    }

	  /* attach to INFO list, sfont_close will cleanup if FAIL occurs */
	  sf->info = fluid_list_append (sf->info, item);

	  *(unsigned char *) item = id;
	  if (fcbs->fread(&item[1], chunk.size, fd) == FLUID_FAILED)
	    return (FAIL);

	  /* force terminate info item (don't forget uint8 info ID) */
	  *(item + chunk.size) = '\0';
	}
      else
	return (gerr (ErrCorr, _("Invalid chunk id in INFO chunk")));
      size -= chunk.size;
    }

  if (size < 0)
    return (gerr (ErrCorr, _("INFO chunk size mismatch")));

  return (OK);
}

static int
process_sdta (unsigned int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  SFChunk chunk;

  if (size == 0)
    return (OK);		/* no sample data? */

  /* read sub chunk */
  READCHUNK (&chunk, fd, fcbs);
  size -= 8;

  if (chunkid (chunk.id) != SMPL_ID)
    return (gerr (ErrCorr,
	_("Expected SMPL chunk found invalid id instead")));

  /* SDTA chunk may also contain sm24 chunk for 24 bit samples
   * (not yet supported), only an error if SMPL chunk size is
   * greater than SDTA. */
  if (chunk.size > size)
    return (gerr (ErrCorr, _("SDTA chunk size mismatch")));

  /* sample data follows */
  sf->samplepos = fcbs->ftell (fd);

  /* used in fixup_sample() to check validity of sample headers */
  sf->samplesize = chunk.size;

  FSKIP (chunk.size, fd, fcbs);
  size -= chunk.size;
  
  if(sf->version.major >= 2 && sf->version.minor >= 4)
  {
    /* any chance to find another chunk here? */
    if(size > 8)
    {
        /* read sub chunk */
        READCHUNK (&chunk, fd, fcbs);
        size -= 8;
        
        if (chunkid (chunk.id) == SM24_ID)
        {
            int sm24size, sdtahalfsize;
            
            FLUID_LOG(FLUID_DBG, "Found SM24 chunk");
            if (chunk.size > size)
            {
                FLUID_LOG(FLUID_WARN, "SM24 exeeds SDTA chunk, ignoring SM24");
                goto ret; // no error
            }
            
            sdtahalfsize = sf->samplesize/2;
            /* + 1 byte in the case that half the size of smpl chunk is an odd value */
            sdtahalfsize += sdtahalfsize%2;
            sm24size = chunk.size;
            
            if (sdtahalfsize != sm24size)
            {
                FLUID_LOG(FLUID_WARN, "SM24 not equal to half the size of SMPL chunk (0x%X != 0x%X), ignoring SM24", sm24size, sdtahalfsize);
                goto ret; // no error
            }
            
            /* sample data24 follows */
            sf->sample24pos = fcbs->ftell (fd);
            sf->sample24size = sm24size;
        }
    }
  }
  
ret:
  FSKIP (size, fd, fcbs);
  
  return (OK);
}

static int
pdtahelper (unsigned int expid, unsigned int reclen, SFChunk * chunk,
  int * size, void * fd, const fluid_file_callbacks_t* fcbs)
{
  unsigned int id;
  const char *expstr;

  expstr = CHNKIDSTR (expid);	/* in case we need it */

  READCHUNK (chunk, fd, fcbs);
  *size -= 8;

  if ((id = chunkid (chunk->id)) != expid)
    return (gerr (ErrCorr, _("Expected"
	  " PDTA sub-chunk \"%.4s\" found invalid id instead"), expstr));

  if (chunk->size % reclen)	/* valid chunk size? */
    return (gerr (ErrCorr,
	_("\"%.4s\" chunk size is not a multiple of %d bytes"), expstr,
	reclen));
  if ((*size -= chunk->size) < 0)
    return (gerr (ErrCorr,
	_("\"%.4s\" chunk size exceeds remaining PDTA chunk size"), expstr));
  return (OK);
}

static int
process_pdta (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  SFChunk chunk;

  if (!pdtahelper (PHDR_ID, SFPHDRSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_phdr (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (PBAG_ID, SFBAGSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_pbag (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (PMOD_ID, SFMODSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_pmod (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (PGEN_ID, SFGENSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_pgen (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (IHDR_ID, SFIHDRSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_ihdr (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (IBAG_ID, SFBAGSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_ibag (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (IMOD_ID, SFMODSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_imod (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (IGEN_ID, SFGENSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_igen (chunk.size, sf, fd, fcbs))
    return (FAIL);

  if (!pdtahelper (SHDR_ID, SFSHDRSIZE, &chunk, &size, fd, fcbs))
    return (FAIL);
  if (!load_shdr (chunk.size, sf, fd, fcbs))
    return (FAIL);

  return (OK);
}

/* preset header loader */
static int
load_phdr (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  int i, i2;
  SFPreset *p, *pr = NULL;	/* ptr to current & previous preset */
  unsigned short zndx, pzndx = 0;

  if (size % SFPHDRSIZE || size == 0)
    return (gerr (ErrCorr, _("Preset header chunk size is invalid")));

  i = size / SFPHDRSIZE - 1;
  if (i == 0)
    {				/* at least one preset + term record */
      FLUID_LOG (FLUID_WARN, _("File contains no presets"));
      FSKIP (SFPHDRSIZE, fd, fcbs);
      return (OK);
    }

  for (; i > 0; i--)
    {				/* load all preset headers */
      p = FLUID_NEW (SFPreset);
      sf->preset = fluid_list_append (sf->preset, p);
      p->zone = NULL;		/* In case of failure, sfont_close can cleanup */
      READSTR (&p->name, fd, fcbs);	/* possible read failure ^ */
      READW (p->prenum, fd, fcbs);
      READW (p->bank, fd, fcbs);
      READW (zndx, fd, fcbs);
      READD (p->libr, fd, fcbs);
      READD (p->genre, fd, fcbs);
      READD (p->morph, fd, fcbs);

      if (pr)
	{			/* not first preset? */
	  if (zndx < pzndx)
	    return (gerr (ErrCorr, _("Preset header indices not monotonic")));
	  i2 = zndx - pzndx;
	  while (i2--)
	    {
	      pr->zone = fluid_list_prepend (pr->zone, NULL);
	    }
	}
      else if (zndx > 0)	/* 1st preset, warn if ofs >0 */
	FLUID_LOG (FLUID_WARN, _("%d preset zones not referenced, discarding"), zndx);
      pr = p;			/* update preset ptr */
      pzndx = zndx;
    }

  FSKIP (24, fd, fcbs);
  READW (zndx, fd, fcbs);		/* Read terminal generator index */
  FSKIP (12, fd, fcbs);

  if (zndx < pzndx)
    return (gerr (ErrCorr, _("Preset header indices not monotonic")));
  i2 = zndx - pzndx;
  while (i2--)
    {
      pr->zone = fluid_list_prepend (pr->zone, NULL);
    }

  return (OK);
}

/* preset bag loader */
static int
load_pbag (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  fluid_list_t *p, *p2;
  SFZone *z, *pz = NULL;
  unsigned short genndx, modndx;
  unsigned short pgenndx = 0, pmodndx = 0;
  unsigned short i;

  if (size % SFBAGSIZE || size == 0)	/* size is multiple of SFBAGSIZE? */
    return (gerr (ErrCorr, _("Preset bag chunk size is invalid")));

  p = sf->preset;
  while (p)
    {				/* traverse through presets */
      p2 = ((SFPreset *) (p->data))->zone;
      while (p2)
	{			/* traverse preset's zones */
	  if ((size -= SFBAGSIZE) < 0)
	    return (gerr (ErrCorr, _("Preset bag chunk size mismatch")));
	  z = FLUID_NEW (SFZone);
	  p2->data = z;
	  z->gen = NULL;	/* Init gen and mod before possible failure, */
	  z->mod = NULL;	/* to ensure proper cleanup (sfont_close) */
	  READW (genndx, fd, fcbs);	/* possible read failure ^ */
	  READW (modndx, fd, fcbs);
	  z->instsamp = NULL;

	  if (pz)
	    {			/* if not first zone */
	      if (genndx < pgenndx)
		return (gerr (ErrCorr,
		    _("Preset bag generator indices not monotonic")));
	      if (modndx < pmodndx)
		return (gerr (ErrCorr,
		    _("Preset bag modulator indices not monotonic")));
	      i = genndx - pgenndx;
	      while (i--)
		pz->gen = fluid_list_prepend (pz->gen, NULL);
	      i = modndx - pmodndx;
	      while (i--)
		pz->mod = fluid_list_prepend (pz->mod, NULL);
	    }
	  pz = z;		/* update previous zone ptr */
	  pgenndx = genndx;	/* update previous zone gen index */
	  pmodndx = modndx;	/* update previous zone mod index */
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  size -= SFBAGSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Preset bag chunk size mismatch")));

  READW (genndx, fd, fcbs);
  READW (modndx, fd, fcbs);

  if (!pz)
    {
      if (genndx > 0)
	FLUID_LOG (FLUID_WARN, _("No preset generators and terminal index not 0"));
      if (modndx > 0)
	FLUID_LOG (FLUID_WARN, _("No preset modulators and terminal index not 0"));
      return (OK);
    }

  if (genndx < pgenndx)
    return (gerr (ErrCorr, _("Preset bag generator indices not monotonic")));
  if (modndx < pmodndx)
    return (gerr (ErrCorr, _("Preset bag modulator indices not monotonic")));
  i = genndx - pgenndx;
  while (i--)
    pz->gen = fluid_list_prepend (pz->gen, NULL);
  i = modndx - pmodndx;
  while (i--)
    pz->mod = fluid_list_prepend (pz->mod, NULL);

  return (OK);
}

/* preset modulator loader */
static int
load_pmod (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  fluid_list_t *p, *p2, *p3;
  SFMod *m;

  p = sf->preset;
  while (p)
    {				/* traverse through all presets */
      p2 = ((SFPreset *) (p->data))->zone;
      while (p2)
	{			/* traverse this preset's zones */
	  p3 = ((SFZone *) (p2->data))->mod;
	  while (p3)
	    {			/* load zone's modulators */
	      if ((size -= SFMODSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Preset modulator chunk size mismatch")));
	      m = FLUID_NEW (SFMod);
	      p3->data = m;
	      READW (m->src, fd, fcbs);
	      READW (m->dest, fd, fcbs);
	      READW (m->amount, fd, fcbs);
	      READW (m->amtsrc, fd, fcbs);
	      READW (m->trans, fd, fcbs);
	      p3 = fluid_list_next (p3);
	    }
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  /*
     If there isn't even a terminal record
     Hmmm, the specs say there should be one, but..
   */
  if (size == 0)
    return (OK);

  size -= SFMODSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Preset modulator chunk size mismatch")));
  FSKIP (SFMODSIZE, fd, fcbs);	/* terminal mod */

  return (OK);
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
static int
load_pgen (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
  SFZone *z;
  SFGen *g;
  SFGenAmount genval;
  unsigned short genid;
  int level, skip, drop, gzone, discarded;

  p = sf->preset;
  while (p)
    {				/* traverse through all presets */
      gzone = FALSE;
      discarded = FALSE;
      p2 = ((SFPreset *) (p->data))->zone;
      if (p2)
	hz = &p2;
      while (p2)
	{			/* traverse preset's zones */
	  level = 0;
	  z = (SFZone *) (p2->data);
	  p3 = z->gen;
	  while (p3)
	    {			/* load zone's generators */
	      dup = NULL;
	      skip = FALSE;
	      drop = FALSE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Preset generator chunk size mismatch")));

	      READW (genid, fd, fcbs);

	      if (genid == Gen_KeyRange)
		{		/* nothing precedes */
		  if (level == 0)
		    {
		      level = 1;
		      READB (genval.range.lo, fd, fcbs);
		      READB (genval.range.hi, fd, fcbs);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_VelRange)
		{		/* only KeyRange precedes */
		  if (level <= 1)
		    {
		      level = 2;
		      READB (genval.range.lo, fd, fcbs);
		      READB (genval.range.hi, fd, fcbs);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_Instrument)
		{		/* inst is last gen */
		  level = 3;
		  READW (genval.uword, fd, fcbs);
		  ((SFZone *) (p2->data))->instsamp = FLUID_INT_TO_POINTER (genval.uword + 1);
		  break;	/* break out of generator loop */
		}
	      else
		{
		  level = 2;
		  if (gen_validp (genid))
		    {		/* generator valid? */
		      READW (genval.sword, fd, fcbs);
		      dup = gen_inlist (genid, z->gen);
		    }
		  else
		    skip = TRUE;
		}

	      if (!skip)
		{
		  if (!dup)
		    {		/* if gen ! dup alloc new */
		      g = FLUID_NEW (SFGen);
		      p3->data = g;
		      g->id = genid;
		    }
		  else
		    {
		      g = (SFGen *) (dup->data);	/* ptr to orig gen */
		      drop = TRUE;
		    }
		  g->amount = genval;
		}
	      else
		{		/* Skip this generator */
		  discarded = TRUE;
		  drop = TRUE;
		  FSKIPW (fd, fcbs);
		}

	      if (!drop)
		p3 = fluid_list_next (p3);	/* next gen */
	      else
		SLADVREM (z->gen, p3);	/* drop place holder */

	    }			/* generator loop */

	  if (level == 3)
	    SLADVREM (z->gen, p3);	/* zone has inst? */
	  else
	    {			/* congratulations its a global zone */
	      if (!gzone)
		{		/* Prior global zones? */
		  gzone = TRUE;

		  /* if global zone is not 1st zone, relocate */
		  if (*hz != p2)
		    {
		      void* save = p2->data;
		      FLUID_LOG (FLUID_WARN,
			_("Preset \"%s\": Global zone is not first zone"),
			((SFPreset *) (p->data))->name);
		      SLADVREM (*hz, p2);
		      *hz = fluid_list_prepend (*hz, save);
		      continue;
		    }
		}
	      else
		{		/* previous global zone exists, discard */
		  FLUID_LOG (FLUID_WARN,
		    _("Preset \"%s\": Discarding invalid global zone"),
		    ((SFPreset *) (p->data))->name);
		  sfont_zone_delete (sf, hz, (SFZone *) (p2->data));
		}
	    }

	  while (p3)
	    {			/* Kill any zones following an instrument */
	      discarded = TRUE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Preset generator chunk size mismatch")));
	      FSKIP (SFGENSIZE, fd, fcbs);
	      SLADVREM (z->gen, p3);
	    }

	  p2 = fluid_list_next (p2);	/* next zone */
	}
      if (discarded)
	FLUID_LOG(FLUID_WARN,
	  _("Preset \"%s\": Some invalid generators were discarded"),
	  ((SFPreset *) (p->data))->name);
      p = fluid_list_next (p);
    }

  /* in case there isn't a terminal record */
  if (size == 0)
    return (OK);

  size -= SFGENSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Preset generator chunk size mismatch")));
  FSKIP (SFGENSIZE, fd, fcbs);	/* terminal gen */

  return (OK);
}

/* instrument header loader */
static int
load_ihdr (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  int i, i2;
  SFInst *p, *pr = NULL;	/* ptr to current & previous instrument */
  unsigned short zndx, pzndx = 0;

  if (size % SFIHDRSIZE || size == 0)	/* chunk size is valid? */
    return (gerr (ErrCorr, _("Instrument header has invalid size")));

  size = size / SFIHDRSIZE - 1;
  if (size == 0)
    {				/* at least one preset + term record */
      FLUID_LOG (FLUID_WARN, _("File contains no instruments"));
      FSKIP (SFIHDRSIZE, fd, fcbs);
      return (OK);
    }

  for (i = 0; i < size; i++)
    {				/* load all instrument headers */
      p = FLUID_NEW (SFInst);
      sf->inst = fluid_list_append (sf->inst, p);
      p->zone = NULL;		/* For proper cleanup if fail (sfont_close) */
      READSTR (&p->name, fd, fcbs);	/* Possible read failure ^ */
      READW (zndx, fd, fcbs);

      if (pr)
	{			/* not first instrument? */
	  if (zndx < pzndx)
	    return (gerr (ErrCorr,
		_("Instrument header indices not monotonic")));
	  i2 = zndx - pzndx;
	  while (i2--)
	    pr->zone = fluid_list_prepend (pr->zone, NULL);
	}
      else if (zndx > 0)	/* 1st inst, warn if ofs >0 */
	FLUID_LOG (FLUID_WARN, _("%d instrument zones not referenced, discarding"),
	  zndx);
      pzndx = zndx;
      pr = p;			/* update instrument ptr */
    }

  FSKIP (20, fd, fcbs);
  READW (zndx, fd, fcbs);

  if (zndx < pzndx)
    return (gerr (ErrCorr, _("Instrument header indices not monotonic")));
  i2 = zndx - pzndx;
  while (i2--)
    pr->zone = fluid_list_prepend (pr->zone, NULL);

  return (OK);
}

/* instrument bag loader */
static int
load_ibag (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  fluid_list_t *p, *p2;
  SFZone *z, *pz = NULL;
  unsigned short genndx, modndx, pgenndx = 0, pmodndx = 0;
  int i;

  if (size % SFBAGSIZE || size == 0)	/* size is multiple of SFBAGSIZE? */
    return (gerr (ErrCorr, _("Instrument bag chunk size is invalid")));

  p = sf->inst;
  while (p)
    {				/* traverse through inst */
      p2 = ((SFInst *) (p->data))->zone;
      while (p2)
	{			/* load this inst's zones */
	  if ((size -= SFBAGSIZE) < 0)
	    return (gerr (ErrCorr, _("Instrument bag chunk size mismatch")));
	  z = FLUID_NEW (SFZone);
	  p2->data = z;
	  z->gen = NULL;	/* In case of failure, */
	  z->mod = NULL;	/* sfont_close can clean up */
	  READW (genndx, fd, fcbs);	/* READW = possible read failure */
	  READW (modndx, fd, fcbs);
	  z->instsamp = NULL;

	  if (pz)
	    {			/* if not first zone */
	      if (genndx < pgenndx)
		return (gerr (ErrCorr,
		    _("Instrument generator indices not monotonic")));
	      if (modndx < pmodndx)
		return (gerr (ErrCorr,
		    _("Instrument modulator indices not monotonic")));
	      i = genndx - pgenndx;
	      while (i--)
		pz->gen = fluid_list_prepend (pz->gen, NULL);
	      i = modndx - pmodndx;
	      while (i--)
		pz->mod = fluid_list_prepend (pz->mod, NULL);
	    }
	  pz = z;		/* update previous zone ptr */
	  pgenndx = genndx;
	  pmodndx = modndx;
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  size -= SFBAGSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Instrument chunk size mismatch")));

  READW (genndx, fd, fcbs);
  READW (modndx, fd, fcbs);

  if (!pz)
    {				/* in case that all are no zoners */
      if (genndx > 0)
	FLUID_LOG (FLUID_WARN,
	  _("No instrument generators and terminal index not 0"));
      if (modndx > 0)
	FLUID_LOG (FLUID_WARN,
	  _("No instrument modulators and terminal index not 0"));
      return (OK);
    }

  if (genndx < pgenndx)
    return (gerr (ErrCorr, _("Instrument generator indices not monotonic")));
  if (modndx < pmodndx)
    return (gerr (ErrCorr, _("Instrument modulator indices not monotonic")));
  i = genndx - pgenndx;
  while (i--)
    pz->gen = fluid_list_prepend (pz->gen, NULL);
  i = modndx - pmodndx;
  while (i--)
    pz->mod = fluid_list_prepend (pz->mod, NULL);

  return (OK);
}

/* instrument modulator loader */
static int
load_imod (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  fluid_list_t *p, *p2, *p3;
  SFMod *m;

  p = sf->inst;
  while (p)
    {				/* traverse through all inst */
      p2 = ((SFInst *) (p->data))->zone;
      while (p2)
	{			/* traverse this inst's zones */
	  p3 = ((SFZone *) (p2->data))->mod;
	  while (p3)
	    {			/* load zone's modulators */
	      if ((size -= SFMODSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Instrument modulator chunk size mismatch")));
	      m = FLUID_NEW (SFMod);
	      p3->data = m;
	      READW (m->src, fd, fcbs);
	      READW (m->dest, fd, fcbs);
	      READW (m->amount, fd, fcbs);
	      READW (m->amtsrc, fd, fcbs);
	      READW (m->trans, fd, fcbs);
	      p3 = fluid_list_next (p3);
	    }
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  /*
     If there isn't even a terminal record
     Hmmm, the specs say there should be one, but..
   */
  if (size == 0)
    return (OK);

  size -= SFMODSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Instrument modulator chunk size mismatch")));
  FSKIP (SFMODSIZE, fd, fcbs);	/* terminal mod */

  return (OK);
}

/* load instrument generators (see load_pgen for loading rules) */
static int
load_igen (int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
  SFZone *z;
  SFGen *g;
  SFGenAmount genval;
  unsigned short genid;
  int level, skip, drop, gzone, discarded;

  p = sf->inst;
  while (p)
    {				/* traverse through all instruments */
      gzone = FALSE;
      discarded = FALSE;
      p2 = ((SFInst *) (p->data))->zone;
      if (p2)
	hz = &p2;
      while (p2)
	{			/* traverse this instrument's zones */
	  level = 0;
	  z = (SFZone *) (p2->data);
	  p3 = z->gen;
	  while (p3)
	    {			/* load zone's generators */
	      dup = NULL;
	      skip = FALSE;
	      drop = FALSE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr, _("IGEN chunk size mismatch")));

	      READW (genid, fd, fcbs);

	      if (genid == Gen_KeyRange)
		{		/* nothing precedes */
		  if (level == 0)
		    {
		      level = 1;
		      READB (genval.range.lo, fd, fcbs);
		      READB (genval.range.hi, fd, fcbs);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_VelRange)
		{		/* only KeyRange precedes */
		  if (level <= 1)
		    {
		      level = 2;
		      READB (genval.range.lo, fd, fcbs);
		      READB (genval.range.hi, fd, fcbs);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_SampleId)
		{		/* sample is last gen */
		  level = 3;
		  READW (genval.uword, fd, fcbs);
		  ((SFZone *) (p2->data))->instsamp = FLUID_INT_TO_POINTER (genval.uword + 1);
		  break;	/* break out of generator loop */
		}
	      else
		{
		  level = 2;
		  if (gen_valid (genid))
		    {		/* gen valid? */
		      READW (genval.sword, fd, fcbs);
		      dup = gen_inlist (genid, z->gen);
		    }
		  else
		    skip = TRUE;
		}

	      if (!skip)
		{
		  if (!dup)
		    {		/* if gen ! dup alloc new */
		      g = FLUID_NEW (SFGen);
		      p3->data = g;
		      g->id = genid;
		    }
		  else
		    {
		      g = (SFGen *) (dup->data);
		      drop = TRUE;
		    }
		  g->amount = genval;
		}
	      else
		{		/* skip this generator */
		  discarded = TRUE;
		  drop = TRUE;
		  FSKIPW (fd, fcbs);
		}

	      if (!drop)
		p3 = fluid_list_next (p3);	/* next gen */
	      else
		SLADVREM (z->gen, p3);

	    }			/* generator loop */

	  if (level == 3)
	    SLADVREM (z->gen, p3);	/* zone has sample? */
	  else
	    {			/* its a global zone */
	      if (!gzone)
		{
		  gzone = TRUE;

		  /* if global zone is not 1st zone, relocate */
		  if (*hz != p2)
		    {
		      void* save = p2->data;
		      FLUID_LOG (FLUID_WARN,
			_("Instrument \"%s\": Global zone is not first zone"),
			((SFPreset *) (p->data))->name);
		      SLADVREM (*hz, p2);
		      *hz = fluid_list_prepend (*hz, save);
		      continue;
		    }
		}
	      else
		{		/* previous global zone exists, discard */
		  FLUID_LOG (FLUID_WARN,
		    _("Instrument \"%s\": Discarding invalid global zone"),
		    ((SFInst *) (p->data))->name);
		  sfont_zone_delete (sf, hz, (SFZone *) (p2->data));
		}
	    }

	  while (p3)
	    {			/* Kill any zones following a sample */
	      discarded = TRUE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Instrument generator chunk size mismatch")));
	      FSKIP (SFGENSIZE, fd, fcbs);
	      SLADVREM (z->gen, p3);
	    }

	  p2 = fluid_list_next (p2);	/* next zone */
	}
      if (discarded)
	FLUID_LOG(FLUID_WARN,
	  _("Instrument \"%s\": Some invalid generators were discarded"),
	  ((SFInst *) (p->data))->name);
      p = fluid_list_next (p);
    }

  /* for those non-terminal record cases, grr! */
  if (size == 0)
    return (OK);

  size -= SFGENSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("IGEN chunk size mismatch")));
  FSKIP (SFGENSIZE, fd, fcbs);	/* terminal gen */

  return (OK);
}

/* sample header loader */
static int
load_shdr (unsigned int size, SFData * sf, void * fd, const fluid_file_callbacks_t* fcbs)
{
  unsigned int i;
  SFSample *p;

  if (size % SFSHDRSIZE || size == 0)	/* size is multiple of SHDR size? */
    return (gerr (ErrCorr, _("Sample header has invalid size")));

  size = size / SFSHDRSIZE - 1;
  if (size == 0)
    {				/* at least one sample + term record? */
      FLUID_LOG (FLUID_WARN, _("File contains no samples"));
      FSKIP (SFSHDRSIZE, fd, fcbs);
      return (OK);
    }

  /* load all sample headers */
  for (i = 0; i < size; i++)
    {
      p = FLUID_NEW (SFSample);
      sf->sample = fluid_list_append (sf->sample, p);
      READSTR (&p->name, fd, fcbs);
      READD (p->start, fd, fcbs);
      READD (p->end, fd, fcbs);	/* - end, loopstart and loopend */
      READD (p->loopstart, fd, fcbs);	/* - will be checked and turned into */
      READD (p->loopend, fd, fcbs);	/* - offsets in fixup_sample() */
      READD (p->samplerate, fd, fcbs);
      READB (p->origpitch, fd, fcbs);
      READB (p->pitchadj, fd, fcbs);
      FSKIPW (fd, fcbs);		/* skip sample link */
      READW (p->sampletype, fd, fcbs);
      p->samfile = 0;
    }

  FSKIP (SFSHDRSIZE, fd, fcbs);	/* skip terminal shdr */

  return (OK);
}

/* "fixup" (inst # -> inst ptr) instrument references in preset list */
static int
fixup_pgen (SFData * sf)
{
  fluid_list_t *p, *p2, *p3;
  SFZone *z;
  int i;

  p = sf->preset;
  while (p)
    {
      p2 = ((SFPreset *) (p->data))->zone;
      while (p2)
	{			/* traverse this preset's zones */
	  z = (SFZone *) (p2->data);
	  if ((i = FLUID_POINTER_TO_INT (z->instsamp)))
	    {			/* load instrument # */
	      p3 = fluid_list_nth (sf->inst, i - 1);
	      if (!p3)
		return (gerr (ErrCorr,
		    _("Preset %03d %03d: Invalid instrument reference"),
		    ((SFPreset *) (p->data))->bank,
		    ((SFPreset *) (p->data))->prenum));
	      z->instsamp = p3;
	    }
	  else
	    z->instsamp = NULL;
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  return (OK);
}

/* "fixup" (sample # -> sample ptr) sample references in instrument list */
static int
fixup_igen (SFData * sf)
{
  fluid_list_t *p, *p2, *p3;
  SFZone *z;
  int i;

  p = sf->inst;
  while (p)
    {
      p2 = ((SFInst *) (p->data))->zone;
      while (p2)
	{			/* traverse instrument's zones */
	  z = (SFZone *) (p2->data);
	  if ((i = FLUID_POINTER_TO_INT (z->instsamp)))
	    {			/* load sample # */
	      p3 = fluid_list_nth (sf->sample, i - 1);
	      if (!p3)
		return (gerr (ErrCorr,
		    _("Instrument \"%s\": Invalid sample reference"),
		    ((SFInst *) (p->data))->name));
	      z->instsamp = p3;
	    }
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  return (OK);
}

/* convert sample end, loopstart and loopend to offsets and check if valid */
static int
fixup_sample (SFData * sf)
{
  fluid_list_t *p;
  SFSample *sam;
  int invalid_loops=FALSE;
  int invalid_loopstart;
  int invalid_loopend, loopend_end_mismatch;
  unsigned int total_bytes = sf->samplesize;
  unsigned int total_samples = total_bytes / FLUID_MEMBER_SIZE(fluid_defsfont_t, sampledata[0]);

  p = sf->sample;
  while (p)
    {
      unsigned int max_end;

      sam = (SFSample *) (p->data);

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
       * FIXME: where does this number 4 come from? And do we need a different number for SF3 files?
       * Maybe we should check for the minimum Ogg Vorbis headers size? */
      if ((sam->end > max_end) || (sam->start > (sam->end - 4)))
      {
        FLUID_LOG (FLUID_WARN, _("Sample '%s' start/end file positions are invalid,"
                   " disabling and will not be saved"), sam->name);
        sam->start = sam->end = sam->loopstart = sam->loopend = 0;
        goto next_sample;
      }
      
      /* The SoundFont 2.4 spec defines the loopstart index as the first sample point of the loop */
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
             * however we cant use the logic below, because uncompressed samples are stored in individual buffers
             */
        }
      else if (invalid_loopstart || invalid_loopend || loopend_end_mismatch) /* loop is fowled?? (cluck cluck :) */
	    {
            /* though illegal, loopend may be set to loopstart to disable loop */
            /* is it worth informing the user? */
            invalid_loops |= (sam->loopend != sam->loopstart);
	  
	        /* force incorrect loop points into the sample range, ignore padding */
          if(invalid_loopstart)
          {
            FLUID_LOG (FLUID_DBG, _("Sample '%s' has unusable loop start '%d',"
              " setting to sample start at '%d'"), sam->name, sam->loopstart, sam->start);
            sam->loopstart = sam->start;
	        }
          
          if(invalid_loopend)
	        {
            FLUID_LOG (FLUID_DBG, _("Sample '%s' has unusable loop stop '%d',"
              " setting to sample stop at '%d'"), sam->name, sam->loopend, sam->end);
            /* since at this time sam->end points after valid sample data (will correct that few lines below),
             * set loopend to that first invalid sample, since it should never be played, but instead the last
             * valid sample will be played */
            sam->loopend = sam->end;
	        }
          else if(loopend_end_mismatch)
	        {
            FLUID_LOG (FLUID_DBG, _("Sample '%s' has invalid loop stop '%d',"
              " sample stop at '%d', using it anyway"), sam->name, sam->loopend, sam->end);
	        }
	    }

      /* convert sample end, loopstart, loopend to offsets from sam->start */
      sam->end -= sam->start + 1;	/* marks last sample, contrary to SF spec. */
      sam->loopstart -= sam->start;
      sam->loopend -= sam->start;

next_sample:
      p = fluid_list_next (p);
    }

    if(invalid_loops)
    {
      FLUID_LOG (FLUID_WARN, _("Found samples with invalid loops, audible glitches possible."));
    }

  return (OK);
}

/*=================================sfont.c========================
  Smurf SoundFont Editor
  ================================================================*/


/* optimum chunk area sizes (could be more optimum) */
#define PRESET_CHUNK_OPTIMUM_AREA	256
#define INST_CHUNK_OPTIMUM_AREA		256
#define SAMPLE_CHUNK_OPTIMUM_AREA	256
#define ZONE_CHUNK_OPTIMUM_AREA		256
#define MOD_CHUNK_OPTIMUM_AREA		256
#define GEN_CHUNK_OPTIMUM_AREA		256

static const unsigned short badgen[] = {
  Gen_Unused1, Gen_Unused2, Gen_Unused3, Gen_Unused4,
  Gen_Reserved1, Gen_Reserved2, Gen_Reserved3, 0
};

static const unsigned short badpgen[] = {
  Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
  Gen_EndLoopAddrOfs, Gen_StartAddrCoarseOfs, Gen_EndAddrCoarseOfs,
  Gen_StartLoopAddrCoarseOfs, Gen_Keynum, Gen_Velocity,
  Gen_EndLoopAddrCoarseOfs, Gen_SampleModes, Gen_ExclusiveClass,
  Gen_OverrideRootKey, 0
};

/* close SoundFont file and delete a SoundFont structure */
void
sfont_close (SFData * sf, const fluid_file_callbacks_t* fcbs)
{
  fluid_list_t *p, *p2;

  if (sf->sffd)
    fcbs->fclose (sf->sffd);

  if (sf->fname)
    free (sf->fname);

  p = sf->info;
  while (p)
    {
      free (p->data);
      p = fluid_list_next (p);
    }
  delete_fluid_list(sf->info);
  sf->info = NULL;

  p = sf->preset;
  while (p)
    {				/* loop over presets */
      p2 = ((SFPreset *) (p->data))->zone;
      while (p2)
	{			/* loop over preset's zones */
	  sfont_free_zone (p2->data);
	  p2 = fluid_list_next (p2);
	}			/* free preset's zone list */
      delete_fluid_list (((SFPreset *) (p->data))->zone);
      FLUID_FREE (p->data);	/* free preset chunk */
      p = fluid_list_next (p);
    }
  delete_fluid_list (sf->preset);
  sf->preset = NULL;

  p = sf->inst;
  while (p)
    {				/* loop over instruments */
      p2 = ((SFInst *) (p->data))->zone;
      while (p2)
	{			/* loop over inst's zones */
	  sfont_free_zone (p2->data);
	  p2 = fluid_list_next (p2);
	}			/* free inst's zone list */
      delete_fluid_list (((SFInst *) (p->data))->zone);
      FLUID_FREE (p->data);
      p = fluid_list_next (p);
    }
  delete_fluid_list (sf->inst);
  sf->inst = NULL;

  p = sf->sample;
  while (p)
    {
      FLUID_FREE (p->data);
      p = fluid_list_next (p);
    }
  delete_fluid_list (sf->sample);
  sf->sample = NULL;

  FLUID_FREE (sf);
}

/* free all elements of a zone (Preset or Instrument) */
void
sfont_free_zone (SFZone * zone)
{
  fluid_list_t *p;

  if (!zone)
    return;

  p = zone->gen;
  while (p)
    {				/* Free gen chunks for this zone */
      if (p->data)
	FLUID_FREE (p->data);
      p = fluid_list_next (p);
    }
  delete_fluid_list (zone->gen);	/* free genlist */

  p = zone->mod;
  while (p)
    {				/* Free mod chunks for this zone */
      if (p->data)
	FLUID_FREE (p->data);
      p = fluid_list_next (p);
    }
  delete_fluid_list (zone->mod);	/* free modlist */

  FLUID_FREE (zone);	/* free zone chunk */
}

/* preset sort function, first by bank, then by preset # */
int
sfont_preset_compare_func (void* a, void* b)
{
  int aval, bval;

  aval = (int) (((SFPreset *) a)->bank) << 16 | ((SFPreset *) a)->prenum;
  bval = (int) (((SFPreset *) b)->bank) << 16 | ((SFPreset *) b)->prenum;

  return (aval - bval);
}

/* delete zone from zone list */
void
sfont_zone_delete (SFData * sf, fluid_list_t ** zlist, SFZone * zone)
{
  *zlist = fluid_list_remove (*zlist, (void*) zone);
  sfont_free_zone (zone);
}

/* Find generator in gen list */
fluid_list_t *
gen_inlist (int gen, fluid_list_t * genlist)
{				/* is generator in gen list? */
  fluid_list_t *p;

  p = genlist;
  while (p)
    {
      if (p->data == NULL)
	return (NULL);
      if (gen == ((SFGen *) p->data)->id)
	break;
      p = fluid_list_next (p);
    }
  return (p);
}

/* check validity of instrument generator */
int
gen_valid (int gen)
{				/* is generator id valid? */
  int i = 0;

  if (gen > Gen_MaxValid)
    return (FALSE);
  while (badgen[i] && badgen[i] != gen)
    i++;
  return (badgen[i] == 0);
}

/* check validity of preset generator */
int
gen_validp (int gen)
{				/* is preset generator valid? */
  int i = 0;

  if (!gen_valid (gen))
    return (FALSE);
  while (badpgen[i] && badpgen[i] != (unsigned short) gen)
    i++;
  return (badpgen[i] == 0);
}

/*================================util.c===========================*/

/* Logging function, returns FAIL to use as a return value in calling funcs */
int
gerr (int ev, char * fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  vprintf(fmt, args);
  va_end (args);

  printf("\n");

  return (FAIL);
}
