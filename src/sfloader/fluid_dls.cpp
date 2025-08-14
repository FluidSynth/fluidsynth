#define NOMINMAX // std::numeric_limits::max

#include "fluid_dls.h"

#include "fluid_sys.h"
#include "fluid_sfont.h"
#include "fluidsynth_priv.h"
#include "fluid_defsfont.h"
#include "fluid_mod.h"
#include "fluid_synth.h"
#include "fluid_chan.h"

#include "pcm_aulaw.h"

#include <cstring>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <utility>
#include <functional>
#include <algorithm>
#include <limits>
#include <string>

using std::int16_t;
using std::int32_t;
using std::int8_t;
using std::size_t;
using std::uint16_t;
using std::uint32_t;
using std::uint8_t;

#define FMT_4CC_SPEC "%c%c%c%c"
#define FMT_4CC_ARG(x)                                                                    \
    (reinterpret_cast<const char *>(&(x)))[0], (reinterpret_cast<const char *>(&(x)))[1], \
    (reinterpret_cast<const char *>(&(x)))[2], (reinterpret_cast<const char *>(&(x)))[3]

template<class Callable> struct scope_guard
{
    Callable on_exit;
    ~scope_guard() noexcept(noexcept(on_exit()))
    {
        on_exit();
    }
};

struct mlock_guard
{
    void *ptr;
    fluid_long_long_t size;
    bool locked;

    mlock_guard() noexcept : ptr(nullptr), size(0), locked(false)
    {
    }
    mlock_guard(void *ptr, fluid_long_long_t size) noexcept : ptr(ptr), size(size), locked(false)
    {
    }

    mlock_guard(const mlock_guard &) = delete;
    mlock_guard &operator=(const mlock_guard &) = delete;
    mlock_guard(mlock_guard &&other) noexcept
    : ptr(std::exchange(other.ptr, nullptr)), size(std::exchange(other.size, 0)),
      locked(std::exchange(other.locked, false))
    {
    }
    mlock_guard &operator=(mlock_guard &&other) noexcept
    {
        if (this == &other)
        {
            return *this; // self-assignment
        }
        unlock();
        ptr = std::exchange(other.ptr, nullptr);
        size = std::exchange(other.size, 0);
        locked = std::exchange(other.locked, false);
        return *this;
    }

    int lock() noexcept
    {
        if (ptr == nullptr || locked)
        {
            return 0;
        }
        int result = fluid_mlock(ptr, size);
        if (result == 0)
        {
            locked = true;
        }
        return result;
    }

    void unlock() noexcept
    {
        if (ptr == nullptr || !locked)
        {
            return;
        }
        fluid_munlock(ptr, size);
        locked = false;
    }

    ~mlock_guard() noexcept
    {
        unlock();
    }
};

// fluid_sfloader_t interface
static fluid_sfont_t *fluid_dls_loader_load(fluid_sfloader_t *loader, const char *filename) noexcept;
static void fluid_dls_loader_delete(fluid_sfloader_t *loader) noexcept;

// fluid_sfont_t interface
static const char *fluid_dls_sfont_get_name(fluid_sfont_t *sfont) noexcept;
static fluid_preset_t *fluid_dls_sfont_get_preset(fluid_sfont_t *sfont, int bank, int prenum) noexcept;
static void fluid_dls_iteration_start(fluid_sfont_t *sfont) noexcept;
static fluid_preset_t *fluid_dls_iteration_next(fluid_sfont_t *sfont) noexcept;
static int fluid_dls_sfont_delete(fluid_sfont_t *sfont) noexcept;

// fluid_preset_t interface
static const char *fluid_dls_preset_get_name(fluid_preset_t *preset) noexcept;
static int fluid_dls_preset_get_banknum(fluid_preset_t *preset) noexcept;
static int fluid_dls_preset_get_num(fluid_preset_t *preset) noexcept;
static int fluid_dls_preset_noteon(fluid_preset_t *preset, fluid_synth_t *synth, int chan, int key, int vel) noexcept;
static void fluid_dls_preset_free(fluid_preset_t *preset) noexcept;

// internal struct for keeping some nice information of the DLS

// note that it is at the level of lart
struct fluid_dls_articulation
{
    std::optional<fluid_real_t> gens[GEN_LAST];
    std::vector<fluid_mod_t> mods;

    fluid_real_t keynum_scale = 1.0; // this is not scaletuning

    fluid_dls_articulation()
    {
        gens[GEN_MODLFOFREQ] = -851.3179423647571;  // 5 Hz
        gens[GEN_MODLFODELAY] = -7972.627427729669; // 10 ms
        gens[GEN_VIBLFOFREQ] = -851.3179423647571;  // 5 Hz
        gens[GEN_VIBLFODELAY] = -7972.627427729669; // 10 ms

        // CC 91 -> reverb send 100%
        mods.push_back(fluid_mod_t{
        GEN_REVERBSEND, 91, FLUID_MOD_CC, 0, 0, FLUID_MOD_TRANSFORM_LINEAR, 1000, nullptr, nullptr, nullptr });
        // CC 93 -> chorus send 100%
        mods.push_back(fluid_mod_t{
        GEN_CHORUSSEND, 93, FLUID_MOD_CC, 0, 0, FLUID_MOD_TRANSFORM_LINEAR, 1000, nullptr, nullptr, nullptr });
        // velocity -> filter cutoff disabled
        // actually it is already disabled by fluidsynth
        mods.push_back(fluid_mod_t{
        GEN_FILTERFC, FLUID_MOD_VELOCITY, FLUID_MOD_GC, 0, 0, FLUID_MOD_TRANSFORM_LINEAR, 0, nullptr, nullptr, nullptr });
        // vib lfo --(midi channel pressure)-> pitch disabled
        mods.push_back(fluid_mod_t{
        GEN_VIBLFOTOPITCH, FLUID_MOD_CHANNELPRESSURE, FLUID_MOD_GC, 0, 0, FLUID_MOD_TRANSFORM_LINEAR, 0, nullptr, nullptr, nullptr });
        // pitch wheel --(rpn 0)-> pitch 12800 cents
        // see also the comment in gen.h about GEN_PITCH
        mods.push_back(fluid_mod_t{ GEN_FINETUNE,
                                    FLUID_MOD_PITCHWHEEL,
                                    FLUID_MOD_GC | FLUID_MOD_BIPOLAR,
                                    FLUID_MOD_PITCHWHEELSENS,
                                    FLUID_MOD_GC,
                                    FLUID_MOD_TRANSFORM_LINEAR,
                                    12800,
                                    nullptr,
                                    nullptr,
                                    nullptr });
    }
};

// wsmp is used in LIST[wave] chunk and LIST[rgn ] chunk, converting to different fluid structures
struct fluid_dls_wsmp
{
    uint16_t unity_note;
    int16_t fine_tune; // in cents
    int32_t gain;      // in (cB/65536)
    uint32_t loop_type;
    uint32_t loop_start;
    uint32_t loop_length;
};

struct fluid_dls_sample
{
    // N.B. wlnk is at region level in DLS, but sample level in SF2 (FluidSynth)
    // as wave link is not implemented now, it is simply ignored.

    std::string name;
    unsigned samplerate;
    unsigned start;
    unsigned end; // past the end
    std::optional<fluid_dls_wsmp> wsmp;
};

struct fluid_dls_region
{
    fluid_zone_range_t range{};
    std::optional<fluid_dls_wsmp> wsmp;
    int exclusive_class{};

    uint32_t artindex = -1;
    uint32_t sampleindex{};
    unsigned char samplemode_inherited{}; // loop_type from sample's wsmp
    fluid_real_t gain_inherited{};        // gain from sample's wsmp, in cB
};

// SF2's instrument level does not exist in DLS, preset is called instrument in DLS
struct fluid_dls_instrument
{
    int pcnum;
    int bankmsb;
    int banklsb;
    bool is_drums{};

    std::string name;
    std::vector<fluid_dls_region> regions;

    fluid_preset_t fluid{}; // its user data is `this` (fluid_dls_instrument*)

    fluid_sample_t *samples_fluid;
    fluid_dls_articulation *articulations;
    fluid_synth_t *synth;
};

struct DLSID;

struct fluid_dls_font
{
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    const fluid_file_callbacks_t *fcbs;
    uint32_t output_sample_rate; // only used for cdl eval
    std::string filename;

    void *file{};
    scope_guard<std::function<void()>> on_file_exit{ [this]() { // Once `this` is captured, it must not be copied or moved
        if (file != nullptr)
        {
            fcbs->fclose(file);
            file = nullptr;
        }
    } };
    fluid_long_long_t filesize{};

    // offset to chunk header
    fluid_long_long_t linsoffset{};
    fluid_long_long_t wvploffset{};

    // this MUST NOT be modified after initalization, because of probable mlock
    std::vector<int16_t> sampledata;
    mlock_guard sampledata_mlock;
    std::vector<uint32_t> poolcues; // data of ptbl

    std::vector<fluid_dls_sample> samples;
    // this MUST NOT be modified after initalization, because of instrument.articulations pointer
    std::vector<fluid_dls_articulation> articulations;
    // this MUST NOT be modified after initalization, because of instrument.fluid self-reference
    std::vector<fluid_dls_instrument> instruments;
    // this MUST NOT be modified after initalization, because of instrument.samples_fluid pointer
    std::vector<fluid_sample_t> samples_fluid;

    decltype(instruments)::iterator fluid_preset_iterator;

    // ---

    // The constructor throws exceptions, so exception safety must be guaranteed
    inline fluid_dls_font(fluid_synth_t *synth,
                          fluid_sfont_t *sfont,
                          const fluid_file_callbacks_t *fcbs,
                          const char *filename,
                          uint32_t output_sample_rate,
                          bool try_mlock);

    fluid_dls_font(const fluid_dls_font &) = delete;
    fluid_dls_font &operator=(const fluid_dls_font &) = delete;
    fluid_dls_font(fluid_dls_font &&) = delete;
    fluid_dls_font &operator=(fluid_dls_font &&) noexcept = delete;

    ~fluid_dls_font() = default;

    // parsing functions

    // cdl
    inline std::optional<uint32_t> eval_dlsid_query(const DLSID &dlsid);
    // offset is at chunk data
    inline bool execute_cdls(fluid_long_long_t offset, int size);

    // wave, offset is at chunk header
    inline void parse_wvpl(fluid_long_long_t offset);
    inline void parse_wave(fluid_long_long_t offset, fluid_dls_sample &sample);
    // offset is at wsmpnnnn^... (chunk data)
    inline uint32_t parse_wsmp(fluid_long_long_t offset, fluid_dls_wsmp &wsmp);

    // lins, offset is at chunk header
    inline void parse_lins(fluid_long_long_t offset);
    inline void parse_ins(fluid_long_long_t offset, fluid_dls_instrument &instrument);
    inline bool parse_lart(fluid_long_long_t offset, fluid_dls_articulation &articulation);
    inline void parse_art(fluid_long_long_t offset, fluid_dls_articulation &articulation);
    inline void parse_lrgn(fluid_long_long_t offset, fluid_dls_instrument &instrument);
    inline bool parse_rgn(fluid_long_long_t offset, fluid_dls_region &region);

    // info
    inline std::string read_name_from_info_entries(fluid_long_long_t offset, uint32_t size);

    // utilities
    void fseek(fluid_long_long_t pos, int whence)
    {
        if (fcbs->fseek(file, pos, whence) != FLUID_OK)
        {
            throw std::runtime_error{ "Failed to seek" };
        }
    }

    void fskip(fluid_long_long_t pos)
    {
        fseek(pos, SEEK_CUR);
    }
};

// helper function to create a new fluid_dls_font_t using fluid_new
template<class... Args>
static std::enable_if_t<std::is_constructible_v<fluid_dls_font, Args...>, fluid_dls_font *> // definitely there should be a requires clause
new_fluid_dls_font(Args &&...args) noexcept
{
    auto *dlsfont = FLUID_NEW(fluid_dls_font);
    if (dlsfont == nullptr)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return nullptr;
    }

    try
    {
        new (dlsfont) fluid_dls_font(std::forward<Args>(args)...);
        return dlsfont;
    }
    catch (const std::bad_alloc &)
    {
        FLUID_FREE(dlsfont);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return nullptr;
    }
    catch (const std::out_of_range &err)
    {
        FLUID_FREE(dlsfont);
        FLUID_LOG(FLUID_ERR, "std::out_of_range thrown: %s", err.what());
        return nullptr;
    }
    catch (const std::runtime_error &err)
    {
        FLUID_FREE(dlsfont);
        FLUID_LOG(FLUID_ERR, "%s", err.what());
        return nullptr;
    }
    catch (...)
    {
        FLUID_FREE(dlsfont);
        return nullptr;
    }
}

static void delete_fluid_dls_font(fluid_dls_font *dlsfont) noexcept
{
    if (dlsfont == nullptr)
    {
        return;
    }

    try
    {
        dlsfont->~fluid_dls_font();
    }
    catch (const std::exception &err)
    {
        FLUID_LOG(FLUID_ERR,
                  "Exception thrown in fluid_dls_font destructor: %s: %s",
                  typeid(err).name(),
                  err.what());
    }
    FLUID_FREE(dlsfont);
}

// Basic structures for DLS

#define RIFF_FCC FLUID_FOURCC('R', 'I', 'F', 'F')
#define LIST_FCC FLUID_FOURCC('L', 'I', 'S', 'T')
#define INFO_FCC FLUID_FOURCC('I', 'N', 'F', 'O')
#define DLS_FCC FLUID_FOURCC('D', 'L', 'S', ' ')
#define LINS_FCC FLUID_FOURCC('l', 'i', 'n', 's')
#define DLID_FCC FLUID_FOURCC('d', 'l', 'i', 'd')
#define CDL_FCC FLUID_FOURCC('c', 'd', 'l', ' ')
#define PTBL_FCC FLUID_FOURCC('p', 't', 'b', 'l')
#define VERS_FCC FLUID_FOURCC('v', 'e', 'r', 's')
#define WVPL_FCC FLUID_FOURCC('w', 'v', 'p', 'l')
#define COLH_FCC FLUID_FOURCC('c', 'o', 'l', 'h')
#define INS_FCC FLUID_FOURCC('i', 'n', 's', ' ')
#define WAVE_FCC FLUID_FOURCC('w', 'a', 'v', 'e')
#define LART_FCC FLUID_FOURCC('l', 'a', 'r', 't')
#define INSH_FCC FLUID_FOURCC('i', 'n', 's', 'h')
#define LRGN_FCC FLUID_FOURCC('l', 'r', 'g', 'n')
#define LAR2_FCC FLUID_FOURCC('l', 'a', 'r', '2')
#define WSMP_FCC FLUID_FOURCC('w', 's', 'm', 'p')
#define FMT_FCC FLUID_FOURCC('f', 'm', 't', ' ')
#define DATA_FCC FLUID_FOURCC('d', 'a', 't', 'a')
#define ART1_FCC FLUID_FOURCC('a', 'r', 't', '1')
#define ART2_FCC FLUID_FOURCC('a', 'r', 't', '2')
#define RGN_FCC FLUID_FOURCC('r', 'g', 'n', ' ')
#define RGN2_FCC FLUID_FOURCC('r', 'g', 'n', '2')
#define RGNH_FCC FLUID_FOURCC('r', 'g', 'n', 'h')
#define WLNK_FCC FLUID_FOURCC('w', 'l', 'n', 'k')

// not required but recognized
#define FACT_FCC FLUID_FOURCC('f', 'a', 'c', 't')
#define CUE_FCC FLUID_FOURCC('c', 'u', 'e', ' ')

// this is seen as another 'dlid' in LIST[wave]
#define GUID_FCC FLUID_FOURCC('g', 'u', 'i', 'd')

// The 'crs1' chunk is seen in Crystal's DLS (The CrysDLS, see https://www.vogons.org/viewtopic.php?f=62&t=70307)
// It has wrong size and need manual fix
// See https://github.com/spessasus/spessasynth_core/issues/5 and https://github.com/FluidSynth/fluidsynth/pull/1626
#define CRS1_FCC FLUID_FOURCC('c', 'r', 's', '1')

// info
#define INAM_FCC FLUID_FOURCC('I', 'N', 'A', 'M')

struct RIFFChunk
{
    uint32_t id;   // char[4]
    uint32_t size; // native endian
};

#define READCHUNK_RAW(sf, var)                                                  \
    do                                                                          \
    {                                                                           \
        if ((sf)->fcbs->fread(var, 8, (sf)->file) == FLUID_FAILED)              \
            throw std::runtime_error{ "Failed when reading chunk" };            \
        ((RIFFChunk *)(var))->size = FLUID_LE32TOH(((RIFFChunk *)(var))->size); \
    } while (0)

#define READID(sf, var)                                                  \
    do                                                                   \
    {                                                                    \
        if ((sf)->fcbs->fread(var, 4, (sf)->file) == FLUID_FAILED)       \
            throw std::runtime_error{ "Failed when reading FOURCC ID" }; \
    } while (0)

#define READ8(sf, var)                                              \
    do                                                              \
    {                                                               \
        if ((sf)->fcbs->fread(&var, 1, (sf)->file) == FLUID_FAILED) \
            throw std::runtime_error{ "Failed when reading byte" }; \
    } while (0)

#define READ16(sf, var)                                               \
    do                                                                \
    {                                                                 \
        uint16_t _temp;                                               \
        if ((sf)->fcbs->fread(&_temp, 2, (sf)->file) == FLUID_FAILED) \
            throw std::runtime_error{ "Failed when reading word" };   \
        (var) = FLUID_LE16TOH(_temp);                                 \
    } while (0)

#define READ32(sf, var)                                               \
    do                                                                \
    {                                                                 \
        uint32_t _temp;                                               \
        if ((sf)->fcbs->fread(&_temp, 4, (sf)->file) == FLUID_FAILED) \
            throw std::runtime_error{ "Failed when reading dword" };  \
        (var) = FLUID_LE32TOH(_temp);                                 \
    } while (0)

// return header size
static inline int READCHUNK(fluid_dls_font *dlsfont, RIFFChunk &chunk)
{
    READCHUNK_RAW(dlsfont, &chunk);
    if (chunk.id == LIST_FCC)
    {
        if (chunk.size < 4)
        {
            throw std::runtime_error{ "Bad LIST header: size < 4" };
        }
        READID(dlsfont, &chunk.id);
        chunk.size -= 4;
        return 12;
    }
    if (chunk.id == CRS1_FCC)
    {
        // see CRS1_FCC comment
        chunk.size = 28;
    }
    return 8;
}

inline std::string fluid_dls_font::read_name_from_info_entries(fluid_long_long_t offset, uint32_t size)
{
    fseek(offset, SEEK_SET);
    fluid_long_long_t pos = offset;
    while (pos < offset + size)
    {
        RIFFChunk chunk;
        fseek(pos, SEEK_SET);
        pos += READCHUNK(this, chunk);
        if (chunk.id == INAM_FCC)
        {
            std::string name;
            name.resize(chunk.size);
            if (fcbs->fread(name.data(), chunk.size, file) != FLUID_OK)
            {
                throw std::runtime_error{ "Failed when reading INAM chunk" };
            }
            FLUID_LOG(FLUID_DBG, "Read INAM: '%s'", name.c_str());
            return name;
        }
        pos += chunk.size;
        if (chunk.size % 2 != 0)
        {
            pos++;
        }
    }

    return "";
}

struct DLSID
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];

    friend bool operator==(const DLSID &lhs, const DLSID &rhs) noexcept
    {
        return lhs.Data1 == rhs.Data1 && lhs.Data2 == rhs.Data2 && lhs.Data3 == rhs.Data3 &&
               std::equal(std::begin(lhs.Data4), std::end(lhs.Data4), std::begin(rhs.Data4));
    }
};

#define DEFINE_DLSID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const DLSID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

DEFINE_DLSID(DLSID_GMInHardware, 0x178f2f24, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_DLSID(DLSID_GSInHardware, 0x178f2f25, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_DLSID(DLSID_XGInHardware, 0x178f2f26, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_DLSID(DLSID_SupportsDLS1, 0x178f2f27, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_DLSID(DLSID_SupportsDLS2, 0xf14599e5, 0x4689, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_DLSID(DLSID_SampleMemorySize, 0x178f2f28, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_DLSID(DLSID_ManufacturersID, 0xb03e1181, 0x8095, 0x11d2, 0xa1, 0xef, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8);
DEFINE_DLSID(DLSID_ProductID, 0xb03e1182, 0x8095, 0x11d2, 0xa1, 0xef, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8);
DEFINE_DLSID(DLSID_SamplePlaybackRate, 0x2a91f713, 0xa4bf, 0x11d2, 0xbb, 0xdf, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8);

inline static void READGUID(fluid_dls_font *sf, DLSID &id)
{
    READ32(sf, id.Data1);
    READ16(sf, id.Data2);
    READ16(sf, id.Data3);
    if (sf->fcbs->fread(id.Data4, 8, sf->file) == FLUID_FAILED)
    {
        throw std::runtime_error{ "Failed when reading GUID" };
    }
}

#ifndef WAVE_FORMAT_PCM
constexpr uint16_t WAVE_FORMAT_PCM = 0x0001;
#endif
#ifndef WAVE_FORMAT_ALAW
constexpr uint16_t WAVE_FORMAT_ALAW = 0x0006;
constexpr uint16_t WAVE_FORMAT_MULAW = 0x0007;
#endif

static inline void read_data_lpcm(void *dest, const void *data, fluid_long_long_t size, int bitdepth)
{
    if (bitdepth == 16)
    {
        memcpy(dest, data, size);
        return;
    }
    if (bitdepth != 8)
    {
        // this should never happen!
        throw std::runtime_error{ "Unsupported bitdepth" };
    }

    const auto *src = static_cast<const uint8_t *>(data);
    auto *dest16 = static_cast<int16_t *>(dest);
    for (fluid_long_long_t i = 0; i < size; ++i)
    {
        dest16[i] = (static_cast<int8_t>(src[i] - 128) << 8);
    }
}

static inline void read_data_xlaw(void *dest, const void *data, fluid_long_long_t size, uint16_t format)
{
    const auto *src = static_cast<const int8_t *>(data);
    auto *dest16 = static_cast<int16_t *>(dest);
    if (format == WAVE_FORMAT_ALAW)
    {
        for (fluid_long_long_t i = 0; i < size; ++i)
        {
            dest16[i] = alaw2linear(src[i]);
        }
    }
    else if (format == WAVE_FORMAT_MULAW)
    {
        for (fluid_long_long_t i = 0; i < size; ++i)
        {
            dest16[i] = ulaw2linear(src[i]);
        }
    }
    else
    {
        // this should never happen!
        throw std::runtime_error{ "Unsupported xlaw format" };
    }
}

constexpr uint16_t F_RGN_OPTION_SELFNONEXCLUSIVE = 0x0001;

// DLS Level 2 (1 is included) Sources, Controls, Destinations and Transforms
// Modulator Sources
constexpr uint16_t CONN_SRC_NONE = 0x0000;            // No Source
constexpr uint16_t CONN_SRC_LFO = 0x0001;             // Low Frequency Oscillator
constexpr uint16_t CONN_SRC_KEYONVELOCITY = 0x0002;   // Note-On Velocity
constexpr uint16_t CONN_SRC_KEYNUMBER = 0x0003;       // Note Number
constexpr uint16_t CONN_SRC_EG1 = 0x0004;             // Envelope Generator 1
constexpr uint16_t CONN_SRC_EG2 = 0x0005;             // Envelope Generator 2
constexpr uint16_t CONN_SRC_PITCHWHEEL = 0x0006;      // Pitch Wheel
constexpr uint16_t CONN_SRC_POLYPRESSURE = 0x0007;    // Polyphonic Pressure
constexpr uint16_t CONN_SRC_CHANNELPRESSURE = 0x0008; // Channel Pressure
constexpr uint16_t CONN_SRC_VIBRATO = 0x0009;         // Vibrato LFO

// MIDI Controller Sources
constexpr uint16_t CONN_SRC_CC1 = 0x0081;  // Modulation
constexpr uint16_t CONN_SRC_CC7 = 0x0087;  // Channel Volume
constexpr uint16_t CONN_SRC_CC10 = 0x008a; // Pan
constexpr uint16_t CONN_SRC_CC11 = 0x008b; // Expression
constexpr uint16_t CONN_SRC_CC91 = 0x00db; // Chorus Send
constexpr uint16_t CONN_SRC_CC93 = 0x00dd; // Reverb Send

// Registered Parameter Numbers
constexpr uint16_t CONN_SRC_RPN0 = 0x0100; // RPN0 - Pitch Bend Range
constexpr uint16_t CONN_SRC_RPN1 = 0x0101; // RPN1 - Fine Tune
constexpr uint16_t CONN_SRC_RPN2 = 0x0102; // RPN2 - Coarse Tune

// Generic Destinations
constexpr uint16_t CONN_DST_NONE = 0x0000;      // No Destination
constexpr uint16_t CONN_DST_GAIN = 0x0001;      // Gain
constexpr uint16_t CONN_DST_RESERVED = 0x0002;  // Reserved
constexpr uint16_t CONN_DST_PITCH = 0x0003;     // Pitch
constexpr uint16_t CONN_DST_PAN = 0x0004;       // Pan
constexpr uint16_t CONN_DST_KEYNUMBER = 0x0005; // Key Number Generator

// Channel Output Destinations
constexpr uint16_t CONN_DST_LEFT = 0x0010;        // Left Channel Send
constexpr uint16_t CONN_DST_RIGHT = 0x0011;       // Right Channel Send
constexpr uint16_t CONN_DST_CENTER = 0x0012;      // Center Channel Send
constexpr uint16_t CONN_DST_LFE_CHANNEL = 0x0013; // LFE Channel Send
constexpr uint16_t CONN_DST_LEFTREAR = 0x0014;    // Left Rear Channel Send
constexpr uint16_t CONN_DST_RIGHTREAR = 0x0015;   // Right Rear Channel Send
constexpr uint16_t CONN_DST_CHORUS = 0x0080;      // Chorus Send
constexpr uint16_t CONN_DST_REVERB = 0x0081;      // Reverb Send

// Modulator LFO Destinations
constexpr uint16_t CONN_DST_LFO_FREQUENCY = 0x0104;  // LFO Frequency
constexpr uint16_t CONN_DST_LFO_STARTDELAY = 0x0105; // LFO Start Delay Time

// Vibrato LFO Destinations
constexpr uint16_t CONN_DST_VIB_FREQUENCY = 0x0114;  // Vibrato Frequency
constexpr uint16_t CONN_DST_VIB_STARTDELAY = 0x0115; // Vibrato Start Delay

// EG Destinations
constexpr uint16_t CONN_DST_EG1_ATTACKTIME = 0x0206;   // EG1 Attack Time
constexpr uint16_t CONN_DST_EG1_DECAYTIME = 0x0207;    // EG1 Decay Time
constexpr uint16_t CONN_DST_EG1_RESERVED = 0x0208;     // EG1 Reserved
constexpr uint16_t CONN_DST_EG1_RELEASETIME = 0x0209;  // EG1 Release Time
constexpr uint16_t CONN_DST_EG1_SUSTAINLEVEL = 0x020A; // EG1 Sustain Level
constexpr uint16_t CONN_DST_EG1_DELAYTIME = 0x020B;    // EG1 Delay Time
constexpr uint16_t CONN_DST_EG1_HOLDTIME = 0x020C;     // EG1 Hold Time
constexpr uint16_t CONN_DST_EG1_SHUTDOWNTIME = 0x020D; // EG1 Shutdown Time

constexpr uint16_t CONN_DST_EG2_ATTACKTIME = 0x030A;   // EG2 Attack Time
constexpr uint16_t CONN_DST_EG2_DECAYTIME = 0x030B;    // EG2 Decay Time
constexpr uint16_t CONN_DST_EG2_RESERVED = 0x030C;     // EG2 Reserved
constexpr uint16_t CONN_DST_EG2_RELEASETIME = 0x030D;  // EG2 Release Time
constexpr uint16_t CONN_DST_EG2_SUSTAINLEVEL = 0x030E; // EG2 Sustain Level
constexpr uint16_t CONN_DST_EG2_DELAYTIME = 0x030F;    // EG2 Delay Time
constexpr uint16_t CONN_DST_EG2_HOLDTIME = 0x0310;     // EG2 Hold Time

// Filter Destinations
constexpr uint16_t CONN_DST_FILTER_CUTOFF = 0x0500; // Filter Cutoff Frequency
constexpr uint16_t CONN_DST_FILTER_Q = 0x0501;      // Filter Resonance

// Transforms
constexpr uint16_t CONN_TRN_NONE = 0x0000;    // No Transform
constexpr uint16_t CONN_TRN_CONCAVE = 0x0001; // Concave Transform
constexpr uint16_t CONN_TRN_CONVEX = 0x0002;  // Convex Transform
constexpr uint16_t CONN_TRN_SWITCH = 0x0003;  // Switch Transform

// transform mask
constexpr uint16_t TRN_SRC_INV_MASK = 0b100000'000000'0000;
constexpr uint16_t TRN_SRC_BIP_MASK = 0b010000'000000'0000;
constexpr uint16_t TRN_SRC_TRN_MASK = 0b001111'000000'0000;
constexpr uint16_t TRN_CTL_INV_MASK = 0b000000'100000'0000;
constexpr uint16_t TRN_CTL_BIP_MASK = 0b000000'010000'0000;
constexpr uint16_t TRN_CTL_TRN_MASK = 0b000000'001111'0000;
constexpr uint16_t TRN_OUT_TRN_MASK = 0b000000'000000'1111;

struct DLSTransform
{
    uint16_t out_trans : 4;
    uint16_t ctl_trans : 4;
    uint16_t src_trans : 4;
    bool ctl_inv : 1;
    bool src_inv : 1;
    bool ctl_bip : 1;
    bool src_bip : 1;

    explicit DLSTransform(uint16_t transform) noexcept
    : out_trans(transform & TRN_OUT_TRN_MASK), ctl_trans((transform & TRN_CTL_TRN_MASK) >> 4),
      src_trans((transform & TRN_SRC_TRN_MASK) >> 10), ctl_inv((transform & TRN_CTL_INV_MASK) != 0),
      src_inv((transform & TRN_SRC_INV_MASK) != 0), ctl_bip((transform & TRN_CTL_BIP_MASK) != 0),
      src_bip((transform & TRN_SRC_BIP_MASK) != 0)
    {
    }

    explicit operator uint16_t() const noexcept
    {
        return (src_inv ? TRN_SRC_INV_MASK : 0) | (src_bip ? TRN_SRC_BIP_MASK : 0) |
               (src_trans << 10) | (ctl_inv ? TRN_CTL_INV_MASK : 0) |
               (ctl_bip ? TRN_CTL_BIP_MASK : 0) | (ctl_trans << 4) | out_trans;
    }
};

// move usControl transform to usSource transform
constexpr static uint16_t dls_transform_ctl_to_src(uint16_t transform)
{
    return ((transform << 6) & 0b111111'000000'0000) | (transform & 0b1111);
}

constexpr static std::optional<unsigned char> convert_dls_transform_to_fluid(uint16_t transform_mode)
{
    switch (transform_mode)
    {
        case CONN_TRN_NONE:
            return FLUID_MOD_LINEAR;
        case CONN_TRN_CONCAVE:
            return FLUID_MOD_CONCAVE;
        case CONN_TRN_CONVEX:
            return FLUID_MOD_CONVEX;
        case CONN_TRN_SWITCH:
            return FLUID_MOD_SWITCH;
        default:
            return std::nullopt;
    }
}

constexpr static std::optional<uint16_t> convert_dls_mod_dest_to_gen(uint16_t dest)
{
    if (dest == CONN_DST_NONE || dest == CONN_DST_RESERVED || dest == CONN_DST_EG1_RESERVED ||
        dest == CONN_DST_EG2_RESERVED || (dest >= CONN_DST_LEFT && dest <= CONN_DST_RIGHTREAR))
    {
        return std::nullopt;
    }

    switch (dest)
    {
        case CONN_DST_GAIN:
            return GEN_ATTENUATION; // NOTE: attenuation = -gain
        case CONN_DST_EG1_SUSTAINLEVEL:
            return GEN_VOLENVSUSTAIN; // NOTE: SF2 sustain (atten) in dB = (100% - sustainlevel) * 96 dB (or 144 dB, whatever)
        case CONN_DST_EG2_SUSTAINLEVEL:
            return GEN_MODENVSUSTAIN; // NOTE: SF2 sustain (atten) = 100% - sustainlevel
        case CONN_DST_PITCH:
            return GEN_FINETUNE;
        case CONN_DST_PAN:
            return GEN_PAN;
        case CONN_DST_CHORUS:
            return GEN_CHORUSSEND;
        case CONN_DST_REVERB:
            return GEN_REVERBSEND;
        case CONN_DST_LFO_FREQUENCY:
            return GEN_MODLFOFREQ;
        case CONN_DST_LFO_STARTDELAY:
            return GEN_MODLFODELAY;
        case CONN_DST_VIB_FREQUENCY:
            return GEN_VIBLFOFREQ;
        case CONN_DST_VIB_STARTDELAY:
            return GEN_VIBLFODELAY;
        case CONN_DST_EG1_ATTACKTIME:
            return GEN_VOLENVATTACK;
        case CONN_DST_EG1_DECAYTIME:
            return GEN_VOLENVDECAY;
        case CONN_DST_EG1_RELEASETIME:
            return GEN_VOLENVRELEASE;
        case CONN_DST_EG1_DELAYTIME:
            return GEN_VOLENVDELAY;
        case CONN_DST_EG1_HOLDTIME:
            return GEN_VOLENVHOLD;
        case CONN_DST_EG1_SHUTDOWNTIME:
            return std::nullopt;
        case CONN_DST_EG2_ATTACKTIME:
            return GEN_MODENVATTACK;
        case CONN_DST_EG2_DECAYTIME:
            return GEN_MODENVDECAY;
        case CONN_DST_EG2_RELEASETIME:
            return GEN_MODENVRELEASE;
        case CONN_DST_EG2_DELAYTIME:
            return GEN_MODENVDELAY;
        case CONN_DST_EG2_HOLDTIME:
            return GEN_MODENVHOLD;
        case CONN_DST_FILTER_CUTOFF:
            return GEN_FILTERFC;
        case CONN_DST_FILTER_Q:
            return GEN_FILTERQ;
        default:
            return std::nullopt;
    }
}

// gsrc: general source
constexpr std::optional<uint16_t> convert_dls_mod_gsrc(uint16_t source)
{
    switch (source)
    {
        case CONN_SRC_NONE:
            return FLUID_MOD_NONE;
        case CONN_SRC_KEYONVELOCITY:
            return FLUID_MOD_VELOCITY;
        case CONN_SRC_KEYNUMBER:
            return FLUID_MOD_KEY;
        case CONN_SRC_POLYPRESSURE:
            return FLUID_MOD_KEYPRESSURE;
        case CONN_SRC_CHANNELPRESSURE:
            return FLUID_MOD_CHANNELPRESSURE;
        case CONN_SRC_PITCHWHEEL:
            return FLUID_MOD_PITCHWHEEL;
        case CONN_SRC_RPN0:
            return FLUID_MOD_PITCHWHEELSENS;
        default:
            return std::nullopt;
    }
}

// csrc: CC source
constexpr std::optional<uint16_t> convert_dls_mod_csrc(uint16_t source)
{
    switch (source)
    {
        case CONN_SRC_CC1:
            return 1;
        case CONN_SRC_CC7:
            return 7;
        case CONN_SRC_CC10:
            return 10;
        case CONN_SRC_CC11:
            return 11;
        case CONN_SRC_CC91:
            return 91;
        case CONN_SRC_CC93:
            return 93;
        default:
            return std::nullopt;
    }
}

void add_dls_connectionblock_to_art(fluid_dls_articulation &art,
                                    uint16_t dest_gen,
                                    uint16_t src_dls,
                                    fluid_real_t scale,
                                    DLSTransform transform,
                                    uint16_t control_dls)
{
    if (src_dls == CONN_SRC_NONE && control_dls == CONN_SRC_NONE)
    {
        art.gens[dest_gen] = art.gens[dest_gen].value_or(0) + scale;
        return;
    }

    // DLS-2 1.6.5.5 Output Transforms
    // There are no output transforms defined in the DLS Level 2. The bits in the output transform
    // field must all be set to zero for compatibility with future versions of DLS.

    // if output transform is set to something else, but input transform is none, and the connection
    // block does not have a control source, apply the output transform to the input instead.
    if (transform.out_trans != CONN_TRN_NONE && transform.src_trans == CONN_TRN_NONE && control_dls == CONN_SRC_NONE)
    {
        transform.src_trans = transform.out_trans;
        transform.out_trans = CONN_TRN_NONE;
    }

    if (transform.out_trans != CONN_TRN_NONE)
    {
        FLUID_LOG(FLUID_WARN, "Output transform in connection block is not supported, set to to linear");
    }

    fluid_mod_t mod{};
    auto src = convert_dls_mod_gsrc(src_dls);
    if (src.has_value())
    {
        mod.flags1 = FLUID_MOD_GC;
    }
    else
    {
        src = convert_dls_mod_csrc(src_dls);
        if (src.has_value())
        {
            mod.flags1 = FLUID_MOD_CC;
        }
        else
        {
            FLUID_LOG(FLUID_WARN,
                      "Ignoring connection block with unsupported source for srcOper: %u",
                      static_cast<unsigned>(src_dls));
            return;
        }
    }
    mod.src1 = src.value();

    auto ctl = convert_dls_mod_gsrc(control_dls);
    if (ctl.has_value())
    {
        mod.flags2 = FLUID_MOD_GC;
    }
    else
    {
        ctl = convert_dls_mod_csrc(control_dls);
        if (ctl.has_value())
        {
            mod.flags2 = FLUID_MOD_CC;
        }
        else
        {
            FLUID_LOG(FLUID_WARN,
                      "Ignoring connection block with unsupported source for amtSrcOper: %u",
                      static_cast<unsigned>(control_dls));
            return;
        }
    }
    mod.src2 = ctl.value();

    if (mod.src1 != FLUID_MOD_NONE)
    {
        mod.flags1 |= transform.src_bip ? FLUID_MOD_BIPOLAR : FLUID_MOD_UNIPOLAR;
        mod.flags1 |= transform.src_inv ? FLUID_MOD_NEGATIVE : FLUID_MOD_POSITIVE;
        auto transform_flag = convert_dls_transform_to_fluid(transform.src_trans);
        if (!transform_flag.has_value())
        {
            FLUID_LOG(FLUID_WARN, "Invalid DLS transform enum, ignoring connection block");
            return;
        }
        mod.flags1 |= transform_flag.value();
    }

    if (mod.src2 != FLUID_MOD_NONE)
    {
        mod.flags2 |= transform.ctl_bip ? FLUID_MOD_BIPOLAR : FLUID_MOD_UNIPOLAR;
        mod.flags2 |= transform.ctl_inv ? FLUID_MOD_NEGATIVE : FLUID_MOD_POSITIVE;
        auto transform_flag = convert_dls_transform_to_fluid(transform.ctl_trans);
        if (!transform_flag.has_value())
        {
            FLUID_LOG(FLUID_WARN, "Invalid DLS transform enum, ignoring connection block");
            return;
        }
        mod.flags2 |= transform_flag.value();
    }

    mod.amount = scale;
    mod.dest = dest_gen;

    // transform.out_trans is ignored because unsupported
    mod.trans = FLUID_MOD_TRANSFORM_LINEAR;

    for (auto &existing_mod : art.mods)
    {
        if (fluid_mod_test_identity(&existing_mod, &mod))
        {
            existing_mod.amount += mod.amount;
            return;
        }
    }

    art.mods.push_back(mod);
}

void add_dls_connectionblock_to_art(fluid_dls_articulation &art, uint16_t dest_gen, uint16_t src_dls, fluid_real_t scale, DLSTransform transform)
{
    add_dls_connectionblock_to_art(art, dest_gen, src_dls, scale, transform, CONN_SRC_NONE);
}

void convert_dls_connectionblock_to_art(fluid_dls_articulation &art,
                                        uint16_t source,
                                        uint16_t control,
                                        uint16_t destination,
                                        uint16_t transform,
                                        int32_t scale_16)
{
    switch (destination)
    {
        case CONN_DST_NONE:
        // these reserved dest are used in gm.dls, interesting
        case CONN_DST_RESERVED:
        case CONN_DST_EG1_RESERVED:
        case CONN_DST_EG2_RESERVED:
            return;
        default:
            break;
    }

    if (source == CONN_SRC_NONE && control != CONN_SRC_NONE)
    {
        source = std::exchange(control, CONN_SRC_NONE);
        transform = dls_transform_ctl_to_src(transform);
    }

    if (source == CONN_SRC_RPN1)
    {
        FLUID_LOG(FLUID_WARN, "Ignoring connection block with RPN 1 source");
        return;
    }
    if (source == CONN_SRC_RPN2)
    {
        FLUID_LOG(FLUID_WARN, "Ignoring connection block with RPN 2 source");
        return;
    }
    if (source == CONN_SRC_EG1)
    {
        FLUID_LOG(FLUID_WARN, "Ignoring connection block with EG1 source");
        return;
    }

    auto trans = DLSTransform{ transform };
    fluid_real_t scale = static_cast<fluid_real_t>(scale_16) / 65536;

    // keynum * scale -> keynum, linear
    if (source == CONN_SRC_KEYNUMBER && control == CONN_SRC_NONE &&
        destination == CONN_DST_KEYNUMBER && transform == 0)
    {
        art.keynum_scale = scale / 12800;
        return;
    }

    // LFO or EG -> something
    // they are not available as modulator source in SF2
    // note: keynumToVol/ModEnvHold/Decay is implemented through modulator. why they exist?
    if (source == CONN_SRC_LFO)
    {
        if (!trans.src_bip)
        {
            FLUID_LOG(FLUID_DBG, "Non bipolar LFO source is not supported, set to bipolar");
        }
        if (trans.src_inv || trans.src_trans != CONN_TRN_NONE)
        {
            FLUID_LOG(FLUID_WARN, "LFO source is inverted or transformed, ignoring trnasform");
        }
        switch (destination)
        {
            case CONN_DST_PITCH:
                add_dls_connectionblock_to_art(art,
                                               GEN_MODLFOTOPITCH,
                                               control,
                                               scale,
                                               DLSTransform{ dls_transform_ctl_to_src(transform) });
                return;
            case CONN_DST_FILTER_CUTOFF:
                add_dls_connectionblock_to_art(art,
                                               GEN_MODLFOTOFILTERFC,
                                               control,
                                               scale,
                                               DLSTransform{ dls_transform_ctl_to_src(transform) });
                return;
            case CONN_DST_GAIN:
                add_dls_connectionblock_to_art(
                art, GEN_MODLFOTOVOL, control, scale, DLSTransform{ dls_transform_ctl_to_src(transform) });
                return;
            default:
                break;
        }
    }
    else if (source == CONN_SRC_VIBRATO)
    {
        if (!trans.src_bip)
        {
            FLUID_LOG(FLUID_DBG, "Non bipolar vibrato LFO source is not supported, set to bipolar");
        }
        if (trans.src_inv || trans.src_trans != CONN_TRN_NONE)
        {
            FLUID_LOG(FLUID_WARN, "Inverted or transformed vibrato LFO source is not supported, ignoring trnasform");
        }
        switch (destination)
        {
            case CONN_DST_PITCH:
                add_dls_connectionblock_to_art(art,
                                               GEN_VIBLFOTOPITCH,
                                               control,
                                               scale,
                                               DLSTransform{ dls_transform_ctl_to_src(transform) });
                return;
            default:
                break;
        }
    }
    else if (source == CONN_SRC_EG2)
    {
        if (trans.src_inv || trans.src_bip || trans.src_trans != CONN_TRN_NONE)
        {
            FLUID_LOG(FLUID_WARN, "EG2 source is transformed, ignoring trnasform");
        }
        switch (destination)
        {
            case CONN_DST_PITCH:
                add_dls_connectionblock_to_art(art,
                                               GEN_MODENVTOPITCH,
                                               control,
                                               scale,
                                               DLSTransform{ dls_transform_ctl_to_src(transform) });
                return;
            case CONN_DST_FILTER_CUTOFF:
                add_dls_connectionblock_to_art(art,
                                               GEN_MODENVTOFILTERFC,
                                               control,
                                               scale,
                                               DLSTransform{ dls_transform_ctl_to_src(transform) });
                return;
            default:
                break;
        }
    }

    if (destination == CONN_DST_GAIN)
    {
        scale = -scale;
    }

    if (destination == CONN_DST_EG1_SUSTAINLEVEL)
    {
        art.gens[GEN_VOLENVSUSTAIN] = art.gens[GEN_VOLENVSUSTAIN].value_or(0) + 960;
        scale = -scale * 960 / 1000;
    }

    if (destination == CONN_DST_EG2_SUSTAINLEVEL)
    {
        art.gens[GEN_MODENVSUSTAIN] = art.gens[GEN_MODENVSUSTAIN].value_or(0) + 1000;
        scale = -scale;
    }

    auto dest_gen = convert_dls_mod_dest_to_gen(destination);
    if (!dest_gen.has_value())
    {
        FLUID_LOG(FLUID_DBG,
                  "Ignoring connection block with unsupported destination: %u",
                  static_cast<unsigned>(destination));
        return;
    }

    add_dls_connectionblock_to_art(art, dest_gen.value(), source, scale, trans, control);
}

// Parse the DLS structure
fluid_dls_font::fluid_dls_font(fluid_synth_t *synth,
                               fluid_sfont_t *sfont,
                               const fluid_file_callbacks_t *fcbs,
                               const char *filename,
                               uint32_t output_sample_rate,
                               bool try_mlock)
: synth(synth), sfont(sfont), fcbs(fcbs), output_sample_rate(output_sample_rate), filename(filename)
{
    // Get basic file information

    file = fcbs->fopen(filename); // NOLINT(cppcoreguidelines-prefer-member-initializer)
    if (file == nullptr)
    {
        FLUID_LOG(FLUID_ERR, "Unable to open file '%s'", filename);
        throw std::exception{};
    }

    if (fcbs->fseek(file, 0L, SEEK_END) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Seek to end of file failed");
        throw std::exception{};
    }
    filesize = fcbs->ftell(file);
    if (filesize == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Get end of file position failed");
        throw std::exception{};
    }
    if (fcbs->fseek(file, 0, SEEK_SET) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Rewind to start of file failed");
        throw std::exception{};
    }

    // Parse DLS
    // chunk: RIFF[DLS ]
    // subchunk: ...

    RIFFChunk chunk{};

    READCHUNK(this, chunk); // load RIFF chunk
    if (chunk.id != RIFF_FCC)
    {
        FLUID_LOG(FLUID_ERR, "Not a RIFF file");
        throw std::exception{};
    }

    READID(this, &chunk.id); // load file ID
    if (chunk.id != DLS_FCC)
    {
        FLUID_LOG(FLUID_ERR, "Not a DLS file");
        throw std::exception{};
    }

    if (chunk.size + 8 > filesize)
    {
        FLUID_LOG(FLUID_ERR, "DLS file early EOF");
        throw std::exception{};
    }
    if (chunk.size + 8 < filesize)
    {
        FLUID_LOG(FLUID_WARN, "DLS file has extra data after RIFF chunk");
    }

    // we don't care about real file size after this point
    filesize = chunk.size + 8;

    // iterate over chunks in the RIFF form
    RIFFChunk subchunk{};
    fluid_long_long_t pos = 12;  // RIFF u32 DLS_ 12 bytes
    while (pos < chunk.size + 8) // absolute pos
    {
        fseek(pos, SEEK_SET);
        READCHUNK_RAW(this, &subchunk);
        pos += 8; // <ID> u32

        switch (subchunk.id) // toplevel chunk
        {
            case DLID_FCC:
            case VERS_FCC:
                break;
            case CDL_FCC:
                if (!execute_cdls(pos, subchunk.size))
                {
                    FLUID_LOG(FLUID_ERR, "DLS toplevel CDL bypasses the sound library");
                    throw std::exception{};
                }
                break;
            case COLH_FCC: {
                // read it now to preserve the instrument vector
                if (subchunk.size != 4)
                {
                    FLUID_LOG(FLUID_ERR, "DLS colh chunk size is not 4 bytes");
                    throw std::exception{};
                }
                uint32_t colh;
                READ32(this, colh);
                instruments.reserve(colh);
                break;
            }
            case PTBL_FCC: {
                // read ptbl now
                uint32_t cbsize;
                READ32(this, cbsize);
                if (cbsize < 8)
                {
                    FLUID_LOG(FLUID_ERR, "DLS ptbl chunk has invalid cbSize");
                    throw std::exception{};
                }

                uint32_t cues; // sample count
                READ32(this, cues);
                if (cues * 4 + cbsize != subchunk.size)
                {
                    FLUID_LOG(FLUID_ERR, "DLS ptbl chunk has corrupted size");
                    throw std::exception{};
                }

                fskip(cbsize - 8); // usually cbsize == 8

                poolcues.resize(cues);
                for (uint32_t i = 0; i < cues; i++)
                {
                    READ32(this, poolcues[i]);
                }
                samples.reserve(cues);
                break;
            }
            case LIST_FCC:
                READID(this, &subchunk.id); // read list type
                switch (subchunk.id)
                {
                    case INFO_FCC:
                        read_name_from_info_entries(pos + 4, subchunk.size - 4);
                        break;
                    case LINS_FCC:
                        linsoffset = pos - 8;
                        break;
                    case WVPL_FCC:
                        wvploffset = pos - 8;
                        break;
                    default:
                        FLUID_LOG(FLUID_WARN,
                                  "Ignoring unknown LIST chunk '" FMT_4CC_SPEC "'",
                                  FMT_4CC_ARG(subchunk.id));
                }
                break;
            default:
                FLUID_LOG(FLUID_WARN,
                          "Ignoring unknown top-level DLS chunk '" FMT_4CC_SPEC "'",
                          FMT_4CC_ARG(subchunk.id));
        }

        pos += subchunk.size;
        if (subchunk.size % 2 != 0)
        {
            pos++;
        }
    }

    // Parse samples (LIST[wvpl])
    if (wvploffset == 0)
    {
        FLUID_LOG(FLUID_ERR, "DLS does not contain a LIST[wvpl] chunk");
        throw std::exception{};
    }
    parse_wvpl(wvploffset);
    // reading sample data is now completed

    FLUID_LOG(FLUID_DBG, "DLS %zu samples read", samples.size());

    sampledata_mlock = mlock_guard{ sampledata.data(), static_cast<fluid_long_long_t>(sampledata.size()) };
    if (try_mlock && !sampledata.empty())
    {
        if (sampledata_mlock.lock() != 0)
        {
            FLUID_LOG(FLUID_WARN, "Failed to pin the sample data to RAM; swapping is possible.");
        }
        else
        {
            FLUID_LOG(FLUID_DBG, "Sample data pinned to RAM, %zu bytes", sampledata.size());
        }
    }

    // Parse LIST[lins]

    if (linsoffset == 0)
    {
        FLUID_LOG(FLUID_ERR, "DLS does not contain a LIST[lins] chunk");
        throw std::exception{};
    }
    parse_lins(linsoffset);

    FLUID_LOG(FLUID_DBG, "DLS %zu instruments read", instruments.size());

    // convert dls samples to fluid samples

    samples_fluid.reserve(samples.size());

    for (auto &sample : samples)
    {
        auto &fluid = samples_fluid.emplace_back();
        fluid.start = sample.start;
        fluid.end = sample.end - 1;
        fluid.samplerate = sample.samplerate;
        std::strncpy(fluid.name, sample.name.c_str(), sizeof(fluid.name) - 1);
        fluid.name[sizeof(fluid.name) - 1] = '\0';
        if (sample.wsmp.has_value())
        {
            const auto &wsmp = sample.wsmp.value();
            fluid.loopstart = sample.start + wsmp.loop_start;
            fluid.loopend = sample.start + wsmp.loop_start + wsmp.loop_length;
            fluid.origpitch = wsmp.unity_note;
            fluid.pitchadj = wsmp.fine_tune;
        }
        else
        {
            fluid.loopstart = 0;
            fluid.loopend = 0;
            fluid.origpitch = 0;
            fluid.pitchadj = 0;
        }
        fluid.data = sampledata.data();
        fluid.sampletype = FLUID_SAMPLETYPE_MONO;
    }

    for (auto &instrument : instruments)
    {
        for (auto &region : instrument.regions)
        {
            auto &sample = samples.at(region.sampleindex);
            if (sample.wsmp.has_value())
            {
                const auto &wsmp = sample.wsmp.value();
                if (wsmp.loop_length == 0)
                {
                    region.samplemode_inherited = 0;
                }
                else
                {
                    if (wsmp.loop_type == 0)
                    {
                        region.samplemode_inherited = 1;
                    }
                    else if (wsmp.loop_type == 1)
                    {
                        region.samplemode_inherited = 3;
                    }
                    else
                    {
                        FLUID_LOG(FLUID_WARN,
                                  "Invalid loop type %u, defaulting to loop and release",
                                  static_cast<unsigned int>(wsmp.loop_type));
                        region.samplemode_inherited = 3;
                    }
                }
                region.gain_inherited = static_cast<fluid_real_t>(wsmp.gain) / 65536;
            }
            else
            {
                region.samplemode_inherited = 0;
                region.gain_inherited = 0;
            }
        }
    }

    // now samples list can be cleared
    samples.clear();

    std::sort(instruments.begin(),
              instruments.end(),
              [](const fluid_dls_instrument &lhs, const fluid_dls_instrument &rhs) {
                  if (lhs.bankmsb * 128 + lhs.banklsb == rhs.bankmsb * 128 + rhs.banklsb)
                  {
                      return lhs.pcnum < rhs.pcnum;
                  }
                  return lhs.bankmsb * 128 + lhs.banklsb < rhs.bankmsb * 128 + rhs.banklsb;
              });

    // convert instruments to fluid presets
    for (auto &instrument : instruments)
    {
        instrument.samples_fluid = samples_fluid.data();
        instrument.articulations = articulations.data();

        instrument.fluid.sfont = sfont;
        instrument.synth = synth;
        instrument.fluid.get_name = fluid_dls_preset_get_name;
        instrument.fluid.get_banknum = fluid_dls_preset_get_banknum;
        instrument.fluid.get_num = fluid_dls_preset_get_num;
        instrument.fluid.noteon = fluid_dls_preset_noteon;
        instrument.fluid.free = fluid_dls_preset_free;
        instrument.fluid.data = &instrument;
    }
}

// cdl
inline std::optional<uint32_t> fluid_dls_font::eval_dlsid_query(const DLSID &dlsid)
{
    if (dlsid == DLSID_GMInHardware)
    {
        return 1;
    }
    if (dlsid == DLSID_GSInHardware)
    {
        return 1;
    }
    if (dlsid == DLSID_XGInHardware)
    {
        return 1;
    }
    if (dlsid == DLSID_SupportsDLS1)
    {
        return 1;
    }
    if (dlsid == DLSID_SupportsDLS2)
    {
        return 1;
    }
    if (dlsid == DLSID_SampleMemorySize)
    {
        return std::numeric_limits<uint32_t>::max() / 4 * 3; // ~1.5GiB
    }
    if (dlsid == DLSID_ManufacturersID)
    {
        // See https://www.amei.or.jp/report/System_ID_e.html
        // Who are we?
        return 0x00004000; // a random number, no manufacturer have owned it
    }
    if (dlsid == DLSID_ProductID)
    {
        return 0x0d000721; // a random number
    }
    if (dlsid == DLSID_SamplePlaybackRate)
    {
        return output_sample_rate;
    }
    FLUID_LOG(FLUID_WARN,
              "Unknown CDL query {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
              dlsid.Data1,
              dlsid.Data2,
              dlsid.Data3,
              dlsid.Data4[0],
              dlsid.Data4[1],
              dlsid.Data4[2],
              dlsid.Data4[3],
              dlsid.Data4[4],
              dlsid.Data4[5],
              dlsid.Data4[6],
              dlsid.Data4[7]);
    return std::nullopt;
}

inline bool fluid_dls_font::execute_cdls(fluid_long_long_t offset, int size)
{
    uint32_t stack[64];
    unsigned sp = 0;

    auto push = [&](uint32_t value) {
        if (sp >= sizeof(stack) / sizeof(stack[0]))
        {
            FLUID_LOG(FLUID_ERR, "CDL stack overflow");
            throw std::exception{};
        }
        stack[sp++] = value;
    };

    auto pop = [&]() -> uint32_t {
        if (sp == 0)
        {
            FLUID_LOG(FLUID_ERR, "CDL stack underflow");
            throw std::exception{};
        }
        return stack[--sp];
    };

    if (fcbs->fseek(file, offset, SEEK_SET) != FLUID_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to seek to CDL operations");
        throw std::exception{};
    }

    int pc = 0;
    uint16_t opcode;
    uint32_t rax; // these two are just temp vars
    uint32_t rbx;
    while (pc < size)
    {
        READ16(this, opcode);
        pc += 2;
        switch (opcode)
        {
            // Assign
            case 0x0010: // DLS_CDL_CONST
                READ32(this, rax);
                pc += 4;
                push(rax);
                break;
            // Unary operators
            case 0x000F: // DLS_CDL_NOT (logical)
                push(!pop());
                break;
            // Binary operators
            case 0x0001: // DLS_CDL_AND
                rax = pop();
                rbx = pop();
                push(rax & rbx);
                break;
            case 0x0002: // DLS_CDL_OR
                rax = pop();
                rbx = pop();
                push(rax | rbx);
                break;
            case 0x0003: // DLS_CDL_XOR
                rax = pop();
                rbx = pop();
                push(rax ^ rbx);
                break;
            case 0x0004: // DLS_CDL_ADD
                rax = pop();
                rbx = pop();
                push(rax + rbx);
                break;
            case 0x0005:     // DLS_CDL_SUBTRACT
                rax = pop(); // X, top
                rbx = pop(); // Y, prev
                push(rax - rbx);
                break;
            case 0x0006: // DLS_CDL_MULTIPLY
                rax = pop();
                rbx = pop();
                push(rax * rbx);
                break;
            case 0x0007: // DLS_CDL_DIVIDE
                rax = pop();
                rbx = pop();
                if (rbx == 0)
                {
                    FLUID_LOG(FLUID_ERR, "CDL division by zero");
                    throw std::exception{};
                }
                push(rax / rbx);
                break;
            case 0x0008: // DLS_CDL_LOGICAL_AND
                rax = pop();
                rbx = pop();
                push(rax && rbx);
                break;
            case 0x0009: // DLS_CDL_LOGICAL_OR
                rax = pop();
                rbx = pop();
                push(rax || rbx);
                break;
            case 0x000A: // DLS_CDL_LT
                rax = pop();
                rbx = pop();
                push(rax < rbx);
                break;
            case 0x000B: // DLS_CDL_LE
                rax = pop();
                rbx = pop();
                push(rax <= rbx);
                break;
            case 0x000C: // DLS_CDL_GT
                rax = pop();
                rbx = pop();
                push(rax > rbx);
                break;
            case 0x000D: // DLS_CDL_GE
                rax = pop();
                rbx = pop();
                push(rax >= rbx);
                break;
            case 0x000E: // DLS_CDL_EQ
                rax = pop();
                rbx = pop();
                push(rax == rbx);
                break;
            // Query
            case 0x0011: // DLS_CDL_QUERY
            case 0x0012: // DLS_CDL_QUERY_SUPPORTED
            {
                DLSID dlsid{};
                READGUID(this, dlsid);
                pc += sizeof(DLSID);
                auto result = eval_dlsid_query(dlsid);
                if (opcode == 0x0011) // Query
                {
                    push(result.value()); // or std::bad_optional_access
                }
                push(result.has_value() ? 1 : 0); // Query_Supported
                break;
            }
            default:
                FLUID_LOG(FLUID_ERR, "Unknown CDL opcode 0x%04x", opcode);
                throw std::exception{};
        } // switch(opcode) end
    } // while pc end
    if (pc > size)
    {
        FLUID_LOG(FLUID_ERR, "CDL chunk too early end of chunk");
        throw std::exception{};
    }

    return pop() != 0;
}

// wave
inline void fluid_dls_font::parse_wvpl(fluid_long_long_t offset)
{
    // get basic information of wvpl chunk
    fseek(offset, SEEK_SET);
    RIFFChunk chunk;
    const int headersize = READCHUNK(this, chunk);
    if (offset + headersize + chunk.size > filesize)
    {
        FLUID_LOG(FLUID_ERR, "DLS wvpl chunk exceeds file size");
        throw std::exception{};
    }
    if (headersize != 12)
    {
        FLUID_LOG(FLUID_WARN, "Nonstandard 'wvpl' chunk; 'LIST[wvpl]' is legal");
    }

    // iterate ptbl
    for (auto pos : poolcues)
    {
        auto &sample = samples.emplace_back();
        parse_wave(offset + headersize + pos, sample);
    }
}

inline void fluid_dls_font::parse_wave(fluid_long_long_t offset, fluid_dls_sample &sample)
{
    fseek(offset, SEEK_SET);

    RIFFChunk chunk;
    const int headersize = READCHUNK(this, chunk);
    if (chunk.id != WAVE_FCC)
    {
        FLUID_LOG(FLUID_WARN,
                  "Ignoring unexcepted subchunk in DLS wvpl chunk pointed by ptbl: '" FMT_4CC_SPEC "'",
                  FMT_4CC_ARG(chunk.id));
        return;
    }
    if (headersize != 12)
    {
        fluid_log(FLUID_WARN, "Nonstandard 'wave' chunk; 'LIST[wave]' is legal");
    }

    bool contains_data = false;
    uint16_t fmtTag{};
    uint16_t bitsPerSample{};

    uint32_t pos = offset + headersize;
    RIFFChunk subchunk{};
    while (pos < offset + headersize + chunk.size) // pos is absolute
    {
        fseek(pos, SEEK_SET);
        pos += READCHUNK(this, subchunk);

        switch (subchunk.id)
        {
            case INFO_FCC:
                sample.name = read_name_from_info_entries(pos, subchunk.size);
                break;
            case DLID_FCC:
            case GUID_FCC:
            case FACT_FCC:
            case CUE_FCC:
                break;
            case FMT_FCC: { // See WAVEFORMAT struct (mmreg.h)
                uint16_t temp16;
                uint32_t temp32;
                READ16(this, temp16); // read formatTag
                if (temp16 != WAVE_FORMAT_PCM && temp16 != WAVE_FORMAT_ALAW && temp16 != WAVE_FORMAT_MULAW)
                {
                    FLUID_LOG(FLUID_ERR, "Unsupported wave format %04x", temp16);
                    throw std::exception{};
                }
                fmtTag = temp16;
                READ16(this, temp16); // read channels
                if (temp16 != 1)
                {
                    FLUID_LOG(FLUID_ERR, "Unsupported wave channel count %u", temp16);
                    throw std::exception{};
                }
                READ32(this, temp32); // read sampleRate
                sample.samplerate = temp32;
                READ32(this, temp32); // read avgBytesPerSec
                READ16(this, temp32); // read blockAlign
                READ16(this, temp16); // read bitsPerSample
                if (temp16 != 8 && temp16 != 16)
                {
                    FLUID_LOG(FLUID_ERR, "Unsupported wave bits per sample %u", temp16);
                    throw std::exception{};
                }
                bitsPerSample = temp16;
                if ((fmtTag == WAVE_FORMAT_ALAW || fmtTag == WAVE_FORMAT_MULAW) && bitsPerSample != 8)
                {
                    FLUID_LOG(FLUID_ERR, "A-law/u-law DLS wave must be 8-bit");
                    throw std::exception{};
                }
                // probably a cbSize field for WAVEFORMATEX, but it is zero for alaw/mulaw
                break;
            }
            case WSMP_FCC:
                if (parse_wsmp(pos, sample.wsmp.emplace()) != subchunk.size)
                {
                    FLUID_LOG(FLUID_ERR, "DLS wsmp chunk in wave chunk has corrupted size");
                    throw std::exception{};
                }
                break;
            case DATA_FCC: {
                contains_data = true;
                if (fmtTag == 0 || bitsPerSample == 0)
                {
                    FLUID_LOG(FLUID_ERR, "DLS fmt chunk must exist in wave chunk and be prior to data chunk");
                    throw std::exception{};
                }
                if (subchunk.size % (bitsPerSample / 8) != 0)
                {
                    FLUID_LOG(FLUID_ERR, "DLS data chunk not align to bitsPerSample");
                    throw std::exception{};
                }
                auto samplelen = subchunk.size / (bitsPerSample / 8);
                sample.start = sampledata.size();
                sample.end = sample.start + samplelen;
                sampledata.resize(sampledata.size() + samplelen);

                char buffer[4096];
                uint32_t remaining = subchunk.size;
                int16_t *destination = sampledata.data() + sample.start;
                while (remaining > 0)
                {
                    uint32_t c = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
                    if (fcbs->fread(buffer, c, file) != FLUID_OK)
                    {
                        FLUID_LOG(FLUID_ERR, "fcbs->fread failed when reading DLS data chunk");
                        throw std::exception{};
                    }

                    if (fmtTag == WAVE_FORMAT_PCM)
                    {
                        read_data_lpcm(destination, buffer, c, bitsPerSample);
                    }
                    else
                    {
                        read_data_xlaw(destination, buffer, c, fmtTag);
                    }

                    destination += c / (bitsPerSample / 8);
                    remaining -= c;
                }
                break;
            }
            default:
                FLUID_LOG(FLUID_WARN,
                          "Ignoring unexcepted DLS chunk in LIST[wave] '" FMT_4CC_SPEC "'",
                          FMT_4CC_ARG(subchunk.id));
        }

        pos += subchunk.size;
        if (subchunk.size % 2 != 0)
        {
            pos++;
        }
    } // end for each subchunk in LIST[wave]

    if (!contains_data)
    {
        FLUID_LOG(FLUID_ERR, "DLS data chunk must exist in wave chunk");
        throw std::exception{};
    }
}

inline uint32_t fluid_dls_font::parse_wsmp(fluid_long_long_t offset, fluid_dls_wsmp &wsmp)
{
    fseek(offset, SEEK_SET);

    uint32_t cbsize;
    READ32(this, cbsize);
    if (cbsize < 20)
    {
        // This is seen in Crystal's DLS. See CRS1_FCC comment.
        FLUID_LOG(FLUID_WARN, "DLS wsmp chunk cbSize < 20. The file is probably corrupted.");
        cbsize = 20;
    }
    READ16(this, wsmp.unity_note);
    READ16(this, wsmp.fine_tune);
    READ32(this, wsmp.gain);
    fskip(4); // fulOptions
    uint32_t loops;
    READ32(this, loops);
    if (loops > 1)
    {
        FLUID_LOG(FLUID_ERR, "DLS wsmp chunk has more than one loop");
        throw std::exception{};
    }
    if (loops == 0)
    {
        return cbsize;
    }

    uint32_t loopcbsize;
    READ32(this, loopcbsize);
    if (loopcbsize < 16)
    {
        // This is also seen in Crystal's DLS. See CRS1_FCC comment.
        FLUID_LOG(FLUID_ERR, "DLS wsmp chunk loop cbSize < 16");
        loopcbsize = 16;
    }
    READ32(this, wsmp.loop_type);
    READ32(this, wsmp.loop_start);
    READ32(this, wsmp.loop_length);
    return cbsize + loopcbsize;
}

// lins

inline void fluid_dls_font::parse_lins(fluid_long_long_t offset)
{
    fseek(offset, SEEK_SET);
    RIFFChunk chunk;
    auto headersize = READCHUNK(this, chunk);
    if (chunk.id != LINS_FCC)
    {
        // this should never happen!
        FLUID_LOG(FLUID_ERR, "Expected 'lins' chunk");
        throw std::exception{};
    }
    if (headersize != 12)
    {
        FLUID_LOG(FLUID_WARN, "Nonstandard 'lins' chunk; LIST[lins] is legal");
    }

    fluid_long_long_t pos = offset + headersize; // absolute pos
    while (pos < offset + headersize + chunk.size)
    {
        RIFFChunk subchunk;
        fseek(pos, SEEK_SET);
        auto subchunkpos = pos;
        pos += READCHUNK(this, subchunk);

        if (subchunk.id != INS_FCC)
        {
            FLUID_LOG(FLUID_WARN,
                      "Ignoring unexcepted DLS chunk '" FMT_4CC_SPEC "' in LIST[lins]",
                      FMT_4CC_ARG(subchunk.id));
            goto skip_lins_subchunk;
        }

        {
            auto &instrument = instruments.emplace_back();
            parse_ins(subchunkpos, instrument);
        }

    skip_lins_subchunk:
        pos += subchunk.size;
        if (subchunk.size % 2 != 0)
        {
            pos++;
        }
    }
}

inline void fluid_dls_font::parse_ins(fluid_long_long_t offset, fluid_dls_instrument &instrument)
{
    fseek(offset, SEEK_SET);
    RIFFChunk chunk;
    auto headersize = READCHUNK(this, chunk);
    if (chunk.id != INS_FCC)
    {
        // this should never happen!
        FLUID_LOG(FLUID_ERR, "Expected 'ins' chunk");
        throw std::exception{};
    }
    if (headersize != 12)
    {
        FLUID_LOG(FLUID_WARN, "Nonstandard 'ins' chunk; LIST[ins] is legal");
    }

    size_t articulation_index = -1; // in C++26 we can do std::optional<T&>

    fluid_long_long_t pos = offset + headersize; // absolute pos
    while (pos < offset + headersize + chunk.size)
    {
        RIFFChunk subchunk;
        fseek(pos, SEEK_SET);
        auto subchunkpos = pos;
        pos += READCHUNK(this, subchunk);

        switch (subchunk.id)
        {
            case DLID_FCC:
                break;
            case INFO_FCC:
                instrument.name = read_name_from_info_entries(pos, subchunk.size);
                break;
            case INSH_FCC: {
                if (subchunk.size != 12)
                {
                    FLUID_LOG(FLUID_ERR, "DLS insh chunk size != 12");
                    throw std::exception{};
                }
                uint32_t temp;
                READ32(this, temp); // cRegions
                instrument.regions.reserve(temp);
                READ32(this, temp); // bank
                instrument.is_drums = (temp & 0x80000000) != 0;
                instrument.bankmsb = (temp >> 8) & 0x7F;
                instrument.banklsb = temp & 0x7F;
                READ32(this, temp);
                instrument.pcnum = temp & 0x7F;
                break;
            }
            case LART_FCC:
            case LAR2_FCC:
                if (articulation_index == static_cast<size_t>(-1))
                {
                    articulation_index = articulations.size();
                    if (!parse_lart(subchunkpos, articulations.emplace_back())) // bypassed by cdl
                    {
                        FLUID_LOG(FLUID_DBG, "A instrument lart chunk is bypassed by cdl");
                        articulations.pop_back();
                        articulation_index = static_cast<size_t>(-1);
                    }
                    break;
                }
                parse_lart(subchunkpos, articulations[articulation_index]);
                break;
            case LRGN_FCC:
                parse_lrgn(subchunkpos, instrument);
                break;
            default:
                FLUID_LOG(FLUID_WARN,
                          "Ignoring unexcepted DLS chunk '" FMT_4CC_SPEC "' in LIST[ins]",
                          FMT_4CC_ARG(subchunk.id));
        }

        pos += subchunk.size;
        if (subchunk.size % 2 != 0)
        {
            pos++;
        }
    }

    for (auto &region : instrument.regions)
    {
        if (region.artindex == static_cast<uint32_t>(-1))
        {
            region.artindex = articulation_index;
        }
    }
}

inline bool fluid_dls_font::parse_lart(fluid_long_long_t offset, fluid_dls_articulation &articulation)
{
    fseek(offset, SEEK_SET);
    RIFFChunk chunk;
    auto headersize = READCHUNK(this, chunk);
    if (chunk.id != LART_FCC && chunk.id != LAR2_FCC)
    {
        // this should never happen!
        FLUID_LOG(FLUID_ERR, "Expected 'lart' or 'lar2' chunk");
        throw std::exception{};
    }
    if (headersize != 12)
    {
        FLUID_LOG(FLUID_WARN, "Nonstandard 'lart' chunk; LIST[lart] is legal");
    }

    fluid_long_long_t pos = offset + headersize; // absolute pos
    while (pos < offset + headersize + chunk.size)
    {
        RIFFChunk subchunk;
        fseek(pos, SEEK_SET);
        auto subchunkpos = pos;
        pos += READCHUNK(this, subchunk);

        if (subchunk.id == CDL_FCC)
        {
            if (!execute_cdls(pos, subchunk.size))
            {
                return false;
            }
        }

        if (subchunk.id != ART1_FCC && subchunk.id != ART2_FCC)
        {
            FLUID_LOG(FLUID_WARN,
                      "Ignoring unexcepted DLS chunk '" FMT_4CC_SPEC "' in LIST[lart]",
                      FMT_4CC_ARG(subchunk.id));
            goto skip_lart_subchunk;
        }

        parse_art(subchunkpos, articulation);

    skip_lart_subchunk:
        pos += subchunk.size;
        if (subchunk.size % 2 != 0)
        {
            pos++;
        }
    }

    return true;
}

inline void fluid_dls_font::parse_art(fluid_long_long_t offset, fluid_dls_articulation &articulation)
{
    fseek(offset, SEEK_SET);
    RIFFChunk chunk;
    auto headersize = READCHUNK(this, chunk);
    if (chunk.id != ART1_FCC && chunk.id != ART2_FCC)
    {
        // this should never happen!
        FLUID_LOG(FLUID_ERR, "Expected 'art1' or 'art2' chunk");
        throw std::exception{};
    }
    if (headersize != 8)
    {
        FLUID_LOG(FLUID_WARN, "Nonstandard LIST[art1/2] chunk; 'art1/2' is legal");
    }

    bool isArt1 = chunk.id == ART1_FCC;

    uint32_t cbsize;
    READ32(this, cbsize);
    if (cbsize < 8)
    {
        FLUID_LOG(FLUID_ERR, "art chunk cbSize < 8");
        throw std::exception{};
    }
    uint32_t connblocks;
    READ32(this, connblocks);
    if (cbsize + connblocks * 12 != chunk.size)
    {
        FLUID_LOG(FLUID_ERR, "art chunk has corrupted size");
        throw std::exception{};
    }
    fskip(cbsize - 8);

    for (uint32_t i = 0; i < connblocks; i++)
    {
        uint16_t source, control, dest, trans;
        int32_t scale;
        READ16(this, source);
        READ16(this, control);
        READ16(this, dest);
        READ16(this, trans);
        if (isArt1)
        {
            // DLS-1 uses concave transform for CC 7 and 11
            // in DLS-2 the transform is applied to source

            // DLS-2 transform: 0b00nnnn'00nnnn'0000
            trans = (trans << 10) | (trans << 4);
            auto dlsv2trans = DLSTransform{ trans };
            if (source == CONN_SRC_LFO || source == CONN_SRC_VIBRATO)
            {
                dlsv2trans.src_bip = true;
            }
            if (control == CONN_SRC_LFO || control == CONN_SRC_VIBRATO)
            {
                dlsv2trans.ctl_bip = true;
            }
            trans = static_cast<uint16_t>(dlsv2trans);
        }
        READ32(this, scale);
        convert_dls_connectionblock_to_art(articulation, source, control, dest, trans, scale);
    }
}

inline void fluid_dls_font::parse_lrgn(fluid_long_long_t offset, fluid_dls_instrument &instrument)
{
    fseek(offset, SEEK_SET);
    RIFFChunk chunk;
    auto headersize = READCHUNK(this, chunk);
    if (chunk.id != LRGN_FCC)
    {
        // this should never happen!
        FLUID_LOG(FLUID_ERR, "Expected 'lrgn' chunk");
        throw std::exception{};
    }
    if (headersize != 12)
    {
        FLUID_LOG(FLUID_WARN, "Nonstandard 'lrgn' chunk; LIST[lrgn] is legal");
    }

    fluid_long_long_t pos = offset + headersize; // absolute pos
    while (pos < offset + headersize + chunk.size)
    {
        RIFFChunk subchunk;
        fseek(pos, SEEK_SET);
        auto subchunkpos = pos;
        pos += READCHUNK(this, subchunk);

        if (subchunk.id != RGN_FCC && subchunk.id != RGN2_FCC)
        {
            FLUID_LOG(FLUID_WARN,
                      "Ignoring unexcepted DLS chunk '" FMT_4CC_SPEC "' in LIST[lrgn]",
                      FMT_4CC_ARG(subchunk.id));
            goto skip_lrgn_subchunk;
        }

        {
            auto &region = instrument.regions.emplace_back();
            if (!parse_rgn(subchunkpos, region)) // bypassed by cdl
            {
                FLUID_LOG(FLUID_DBG, "A region is bypassed by cdl");
                instrument.regions.pop_back();
            }
        }

    skip_lrgn_subchunk:
        pos += subchunk.size;
        if (subchunk.size % 2 != 0)
        {
            pos++;
        }
    }
}

inline bool fluid_dls_font::parse_rgn(fluid_long_long_t offset, fluid_dls_region &region)
{
    static int self_exclusive_class [[maybe_unused]] = 65536; // exclusive number for self-exclusive regions

    fseek(offset, SEEK_SET);
    RIFFChunk chunk;
    auto headersize = READCHUNK(this, chunk);
    if (chunk.id != RGN_FCC && chunk.id != RGN2_FCC)
    {
        // this should never happen!
        FLUID_LOG(FLUID_ERR, "Expected 'rgn' chunk");
        return false;
    }
    if (headersize != 12)
    {
        FLUID_LOG(FLUID_WARN, "Nonstandard 'rgn' chunk; LIST[rgn] is legal");
    }

    size_t articulation_index = -1;

    fluid_long_long_t pos = offset + headersize; // absolute pos
    while (pos < offset + headersize + chunk.size)
    {
        RIFFChunk subchunk;
        fseek(pos, SEEK_SET);
        auto subchunkpos = pos;
        pos += READCHUNK(this, subchunk);

        switch (subchunk.id)
        {
            case INFO_FCC:
                break;
            case WLNK_FCC:
                fskip(8); // fluidsynth does not implement phase-locking and multichannel output
                READ32(this, region.sampleindex);
                break;
            case CDL_FCC:
                if (!execute_cdls(pos, subchunk.size))
                {
                    return false;
                }
                break;
            case LART_FCC:
            case LAR2_FCC:
                if (articulation_index == static_cast<size_t>(-1))
                {
                    articulation_index = articulations.size();
                    if (!parse_lart(subchunkpos, articulations.emplace_back()))
                    {
                        FLUID_LOG(FLUID_DBG, "A region lart chunk is bypassed by cdl");
                        articulations.pop_back();
                        articulation_index = static_cast<size_t>(-1);
                    }
                    break;
                }
                parse_lart(subchunkpos, articulations[articulation_index]);
                break;
            case RGNH_FCC: {
                uint16_t temp;
                READ16(this, temp); // key low
                region.range.keylo = temp;
                READ16(this, temp); // key high
                region.range.keyhi = temp;
                READ16(this, temp); // vel low
                region.range.vello = temp;
                READ16(this, temp); // vel high
                region.range.velhi = temp;
                READ16(this, temp);                              // fusOptions
                if ((temp & F_RGN_OPTION_SELFNONEXCLUSIVE) == 0) // self-exclusive
                {
                    // implement this flag doesn't make sense
                    // region.exclusive_class = self_exclusive_class++;
                }
                READ16(this, temp); // keyGroup
                if (temp != 0)
                {
                    region.exclusive_class = temp;
                }
                // usLayer is useless
                break;
            }
            // DLS-2 1.14.6 Each region contains at minimum a <rgnh-ck> region header chunk and a <wlnk-ck> wave link chunk.
            // It may also **optionally** contain a <wsmp-ck> wave sample chunk. ...
            // DLS-2 2.2 <rgn-list> -> ... <wsmp-ck> ...
            // DLS-2 2.2 "the structure tree" ... rgn -> ... wsmp (optional) ...
            // DLS-2 2.8 Other chunks at the same nesting level include a <wsmp-ck> wave sample chunk.
            case WSMP_FCC:
                parse_wsmp(pos, region.wsmp.emplace());
                break;
            default:
                FLUID_LOG(FLUID_WARN,
                          "Unknown DLS chunk '" FMT_4CC_SPEC "' in LIST[rgn]",
                          FMT_4CC_ARG(subchunk.id));
                break;
        }

        pos += subchunk.size;
        if (subchunk.size % 2 != 0)
        {
            pos++;
        }
    }

    region.artindex = articulation_index;

    return true;
}

// fluid sfloader interfaces implementation

struct fluid_dls_loader_data
{
    fluid_settings_t *settings;
    fluid_synth_t *synth;
};

fluid_sfloader_t *new_fluid_dls_loader(fluid_synth_t *synth, fluid_settings_t *settings)
{
    fluid_return_val_if_fail(settings != nullptr, nullptr);

    fluid_sfloader_t *loader = new_fluid_sfloader(fluid_dls_loader_load, fluid_dls_loader_delete);

    if (loader == nullptr)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return nullptr;
    }

    auto *data = new fluid_dls_loader_data;
    data->settings = settings;
    data->synth = synth;
    fluid_sfloader_set_data(loader, data);

    return loader;
}

void fluid_dls_loader_delete(fluid_sfloader_t *loader) noexcept
{
    delete static_cast<fluid_dls_loader_data *>(fluid_sfloader_get_data(loader));
}

static fluid_sfont_t *fluid_dls_loader_load(fluid_sfloader_t *loader, const char *filename) noexcept
{
    auto *sfont = new_fluid_sfont(fluid_dls_sfont_get_name,
                                  fluid_dls_sfont_get_preset,
                                  fluid_dls_iteration_start,
                                  fluid_dls_iteration_next,
                                  fluid_dls_sfont_delete);

    if (sfont == nullptr)
    {
        return nullptr;
    }

    uint32_t sample_rate = 44100;
    bool try_mlock = false;
    auto *sfloader_data = static_cast<fluid_dls_loader_data *>(fluid_sfloader_get_data(loader));
    auto *settings = sfloader_data->settings;
    if (settings != nullptr)
    {
        double rate{};
        if (fluid_settings_getnum(settings, "synth.sample-rate", &rate) == FLUID_OK)
        {
            sample_rate = static_cast<uint32_t>(rate);
        }
        int mlock{};
        if (fluid_settings_getint(settings, "synth.lock-memory", &mlock) == FLUID_OK)
        {
            try_mlock = mlock != 0;
        }
    }

    auto *dlsfont =
    new_fluid_dls_font(sfloader_data->synth, sfont, &loader->file_callbacks, filename, sample_rate, try_mlock);
    if (dlsfont == nullptr)
    {
        delete_fluid_sfont(sfont);
        return nullptr;
    }

    fluid_sfont_set_data(sfont, dlsfont);
    return sfont;
}

// fluid_sfont_t interface
static const char *fluid_dls_sfont_get_name(fluid_sfont_t *sfont) noexcept
{
    return static_cast<const fluid_dls_font *>(fluid_sfont_get_data(sfont))->filename.c_str();
}

static fluid_preset_t *fluid_dls_sfont_get_preset(fluid_sfont_t *sfont, int bank, int prenum) noexcept
{
    auto *dlsfont = static_cast<fluid_dls_font *>(fluid_sfont_get_data(sfont));

    for (auto &inst : dlsfont->instruments)
    {
        if (fluid_dls_preset_get_banknum(&inst.fluid) == bank && inst.pcnum == prenum)
        {
            return &inst.fluid;
        }

        // Drum bank fallback for MMA bank style
        if (inst.synth->bank_select == FLUID_BANK_STYLE_MMA && bank == DRUM_INST_BANK &&
            inst.is_drums && inst.pcnum == prenum)
        {
            return &inst.fluid;
        }
    }

    return nullptr;
}

static void fluid_dls_iteration_start(fluid_sfont_t *sfont) noexcept
{
    auto *dlsfont = static_cast<fluid_dls_font *>(fluid_sfont_get_data(sfont));
    dlsfont->fluid_preset_iterator = dlsfont->instruments.begin();
}

static fluid_preset_t *fluid_dls_iteration_next(fluid_sfont_t *sfont) noexcept
{
    auto *dlsfont = static_cast<fluid_dls_font *>(fluid_sfont_get_data(sfont));
    if (dlsfont->fluid_preset_iterator == dlsfont->instruments.end())
    {
        return nullptr;
    }
    return &(*(dlsfont->fluid_preset_iterator++)).fluid;
}

static int fluid_dls_sfont_delete(fluid_sfont_t *sfont) noexcept
{
    auto *dlsfont = static_cast<fluid_dls_font *>(fluid_sfont_get_data(sfont));

    delete_fluid_dls_font(dlsfont);
    delete_fluid_sfont(sfont);

    return 0;
}

static const char *fluid_dls_preset_get_name(fluid_preset_t *preset) noexcept
{
    return static_cast<const fluid_dls_instrument *>(fluid_preset_get_data(preset))->name.c_str();
}

static int fluid_dls_preset_get_banknum(fluid_preset_t *preset) noexcept
{
    const auto *inst = static_cast<const fluid_dls_instrument *>(fluid_preset_get_data(preset));
    const auto *synth = inst->synth;

    // see fluid_synth.h enum fluid_midi_bank_select
    // see fluid_chan.c fluid_channel_set_bank_msb()
    if (synth->bank_select == FLUID_BANK_STYLE_GM)
    {
        if (inst->is_drums)
        {
            return DRUM_INST_BANK;
        }
        return inst->bankmsb; // to always match bank 0 for GM
    }
    if (synth->bank_select == FLUID_BANK_STYLE_GS)
    {
        return inst->bankmsb + (inst->is_drums ? DRUM_INST_BANK : 0);
    }
    if (synth->bank_select == FLUID_BANK_STYLE_XG)
    {
        if (inst->is_drums)
        {
            // see https://github.com/FluidSynth/fluidsynth/issues/1524
            // see fluid_chan.c fluid_channel_set_bank_msb()
            return 128;
        }
        return inst->banklsb;
    }
    // if (synth->bank_select == FLUID_BANK_STYLE_MMA)
    if (inst->is_drums)
    {
        return DRUM_INST_BANK * 128;
    }
    return inst->banklsb + (128 * inst->bankmsb);
}

static int fluid_dls_preset_get_num(fluid_preset_t *preset) noexcept
{
    return static_cast<const fluid_dls_instrument *>(fluid_preset_get_data(preset))->pcnum;
}

static int fluid_dls_preset_noteon(fluid_preset_t *preset, fluid_synth_t *synth, int chan, int key, int vel) noexcept
{
    auto *dlspreset = static_cast<fluid_dls_instrument *>(fluid_preset_get_data(preset));

    int tuned_key;

    // this is copied from fluid_defsfont
    if (synth->channel[chan]->channel_type == CHANNEL_TYPE_MELODIC)
    {
        tuned_key = (int)(fluid_channel_get_key_pitch(synth->channel[chan], key) / 100.0f + 0.5f);
    }
    else
    {
        tuned_key = key;
    }

    for (auto &region : dlspreset->regions)
    {
        if (!fluid_zone_inside_range(&region.range, tuned_key, vel))
        {
            continue;
        }

        auto *voice = fluid_synth_alloc_voice_LOCAL(
        synth, dlspreset->samples_fluid + region.sampleindex, chan, key, vel, &region.range);

        if (voice == nullptr)
        {
            return FLUID_FAILED;
        }

        if (region.artindex != static_cast<uint32_t>(-1))
        {
            auto &art = dlspreset->articulations[region.artindex];
            for (int i = 0; i < GEN_LAST; i++)
            {
                if (art.gens[i].has_value())
                {
                    fluid_voice_gen_set(voice, i, art.gens[i].value());
                }
            }
            for (auto &mod : art.mods)
            {
                // there is only 10 standard default modulators
                fluid_voice_add_mod_local(voice, &mod, FLUID_VOICE_OVERWRITE, 10);
            }
        }

        if (region.wsmp.has_value())
        {
            const auto &wsmp = region.wsmp.value();
            const auto &sample = dlspreset->samples_fluid[region.sampleindex];

            fluid_voice_gen_set(voice, GEN_OVERRIDEROOTKEY, wsmp.unity_note);
            fluid_voice_gen_incr(voice, GEN_FINETUNE, wsmp.fine_tune - sample.pitchadj);
            fluid_voice_gen_incr(voice, GEN_ATTENUATION, -wsmp.gain / 65536.0f);

            if (wsmp.loop_length != 0)
            {
                if (wsmp.loop_type == 0)
                {
                    fluid_voice_gen_set(voice, GEN_SAMPLEMODE, 1);
                }
                else if (wsmp.loop_type == 1)
                {
                    fluid_voice_gen_set(voice, GEN_SAMPLEMODE, 3);
                }
                else
                {
                    FLUID_LOG(FLUID_WARN, "invalid loop type of region wsmp, set to loop and release");
                    fluid_voice_gen_set(voice, GEN_SAMPLEMODE, 3);
                }
            }
            else
            {
                fluid_voice_gen_set(voice, GEN_SAMPLEMODE, 0);
            }

            fluid_voice_gen_set(voice, GEN_STARTLOOPADDROFS, static_cast<int>(sample.start + wsmp.loop_start) - static_cast<int>(sample.loopstart));
            fluid_voice_gen_set(voice,
                                GEN_ENDLOOPADDROFS,
                                static_cast<int>(sample.start + wsmp.loop_start + wsmp.loop_length) - static_cast<int>(sample.loopend));
        }
        else
        {
            fluid_voice_gen_incr(voice, GEN_ATTENUATION, -region.gain_inherited);
            fluid_voice_gen_set(voice, GEN_SAMPLEMODE, region.samplemode_inherited);
        }

        fluid_voice_gen_set(voice, GEN_EXCLUSIVECLASS, region.exclusive_class);

        fluid_synth_start_voice(synth, voice);
    }

    return FLUID_OK;
}

static void fluid_dls_preset_free(fluid_preset_t *preset [[maybe_unused]]) noexcept
{
    // do nothing. preset (part of fluid_dls_instrument) is under RAII of fluid_dls_font
}
