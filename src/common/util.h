#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))
#define STRUCT_ARRAY_LEN(t, m) (sizeof((((t*)0)->m)) / sizeof((((t*)0)->m)[0]))

#define STATIC_ASSERT(...) static_assert(__VA_ARGS__)

#if defined( __GNUC__ )
#define UNREACHABLE() (__builtin_unreachable())
#elif defined( _MSC_VER )
#define UNREACHABLE() (__assume(0))
#endif

#define XZ(v) v.x, v.y
#define XYZ(v) v.x, v.y, v.z
#define XYZW(v) v.x, v.y, v.z, v.w

typedef union
{
    float    val;
    uint32_t bits;
} float_bits;

typedef union
{
    double   val;
    uint64_t bits;
} double_bits_t;

inline uint32_t ssub_uint32(uint32_t a, uint32_t b)
{
    if (b > a)
        return 0;
    return a - b;
}

inline int16_t bswap_int16(int16_t val)
{
    return (val << 8) | ((val >> 8) & 0xFF);
}

inline uint16_t bswap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

inline int32_t bswap_int32(int32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

inline uint32_t bswap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

inline float bswap_float(float val)
{
    union
    {
        float f;
        uint32_t u;
    } conv = { .f = val };
    conv.u = bswap_uint32(conv.u);
    return conv.f;
}

inline vec2_t bswap_vec2(vec2_t* val)
{
    vec2_t vec = *val;
    for (int i = 0; i < 2; i++)
        vec.v[i] = bswap_float(vec.v[i]);
    return vec;
}

inline vec3_t bswap_vec3(vec3_t* val)
{
    vec3_t vec = *val;
    for (int i = 0; i < 3; i++)
        vec.v[i] = bswap_float(vec.v[i]);
    return vec;
}

inline quat_t bswap_quat(quat_t* val)
{
    quat_t quat = *val;
    for (int i = 0; i < 4; i++)
        quat.v[i] = bswap_float(quat.v[i]);
    return quat;
}

inline char* strcat2(char* dest, char* src)
{
    while (*dest) dest++;
    while (*dest++ = *src++);
    return --dest;
}

inline char* strext(char* dest, const char* src, const char* ext)
{
	strcpy(dest, src);
	strcpy(strrchr(dest, '.') + 1, ext);
    return dest;
}