/* General compiler and language abstraction layer
 * isolate application from bloody differences among compilers
 * RULES: keep it thin, simple and maintainable
 * Creator & Maintainer: Zhang Fan (zhang.h.fan@gmail.com)
 */
#if !defined(__UNI_TYPES_H__)
#define __UNI_TYPES_H__

#include <stdio.h>
#include <stdlib.h>
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
//#ifndef __STDC_FORMAT_MACROS
//#define __STDC_FORMAT_MACROS
//#endif
//#include <inttypes.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <wchar.h>
#include <ctype.h>
#include <wctype.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

/* IMPORTANT Notes: subtle structure alignment difference between GCC and MSVC.
 * let's say we have:
	struct padding_t
	{
		uint64_t val:40;
		//uint64_t pad:24;
		uint8_ dat[8];
	};
	struct padding_t pad;
	printf("dat offset = %d\n", pad.dat - (uint8_t*)&pad);
 * under MSVC, we get 'dat offset = 8', but under GCC, the output is 'dat offset = 5'.
 * so, make sure add padding bit-field in any time.
 */

#ifdef _WIN32
	/*! \def GRCALL
	 *  the calling convention of functions exported by modules.
	 */
#	define GRCALL __stdcall
#	define GRINLINE __inline
#else
#	define GRINLINE inline

	/*! \def GRCALL
	 *  the calling convention of functions exported by modules.
	 */
#	if defined(__CYGWIN__)
#		define GRCALL __stdcall
#	else
#		define __stdcall
#		define GRCALL
#	endif
#endif

#if defined(_MSC_VER)
#	pragma warning(disable: 4200)
#	pragma warning(disable: 4996)
#endif

#if defined(__cplusplus)
#	define GREXTERN extern "C"
extern "C" {
#else
#	define GREXTERN extern
#endif

#ifdef _WIN32
#	define GR_BIG_ENDIAN 0 //XXX
#	define GR_LITTLE_ENDIAN 1 //XXX
#else
#	include <endian.h>
#	define GR_BIG_ENDIAN (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#	define GR_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#endif

#if UINTPTR_MAX == UINT64_MAX
#	define GRI64    "l"
#	define GRIi64   "ld"
#	define GRIu64   "lu"
#	define GRIo64   "lo"
#	define GRIx64   "lx"
#	define GRIX64   "lX"
#else
#	define GRI64    "ll"
#	define GRIi64	"lld"
#	define GRIu64	"llu"
#	define GRIo64	"llo"
#	define GRIx64	"llx"
#	define GRIX64	"llX"
#endif

#define streq(a,b) ( (void*)(a) == (void*)(b) || ((a) && (b) && (strcmp((a),(b)) == 0)) )
#define streqn(a,b,n) ( (void*)(a) == (void*)(b) || ((a) && (b) && (strncmp((a),(b),(n)) == 0)) )
#define strceq(a,b) ( (void*)(a) == (void*)(b) || ((a) && (b) && (strcasecmp((a),(b)) == 0)) )
#define strceqn(a,b,n) ( (void*)(a) == (void*)(b) || ((a) && (b) && (strncasecmp((a),(b),(n)) == 0)) )
#define objeq(a,b) (memcmp(&a, &b, sizeof(a)) == 0)
#define objneq(a,b) (memcmp(&a, &b, sizeof(a)) != 0)
#define strtrue(a) ((a) && ((atoi(a) != 0) || strceq(a, "true")))

/*! \def grmin(a,b)
 *  Return the smaller one of 'a' and 'b'
 */
#ifndef grmin
#	define grmin(a,b) ((a)>(b)?(b):(a))
#endif
/*! \def grmax(a,b)
 *  Return the larger one of 'a' and 'b'
 */
#ifndef grmax
#	define grmax(a,b) ((a)>(b)?(a):(b))
#endif

/*! \def check_range(v,l,h)
 *  To judge if 'v' is located between 'l' and 'h'. 'v'; 'l' and 'h' must be comparable.
 */
#define check_range(v,l,h) ((v)<(l)?(l):(v)>(h)?(h):(v));

#define grcalloc(type) ((type*)calloc(1, sizeof(type)))

#ifdef GR_UTF8
typedef char grchr_t;
typedef char* grstr_t;
#	define GS(a) a
#	define grscpy(a,b) strcpy(a,b)
#	define grsncpy(a,b,n) strncpy(a,b,n)
#	define grslen(a) strlen(a)
#	define grscmp(a,b) strcmp(a,b)
#	define grsncmp(a,b,n) strncmp(a,b,n)
#	define grscasecmp(a,b) strcasecmp(a,b)
#	define grsncasecmp(a,b,n) strncasecmp(a,b,n)
#	define grsdup(a) strdup(a)
#	define grscat(a,b) strcat(a,b)
#	define grprintf printf
#	define grsprintf sprintf
#	define grsnprintf snprintf
#	define grfprintf fprintf
#	define grvprintf vprintf
#	define grvfprintf vfprintf
#	define grvsprintf vsprintf
#	define grsfget fgets
#	define grscanf scanf
#	define grfscanf fscanf
#	define grstok(a,b,c) strtok_r(a,b,c)
#	define grstol(a,b,c) strtol(a,b,c)
#	define grstoll(a,b,c) strtoll(a,b,c)
#	define grstoul(a,b,c) strtoul(a,b,c)
#	define grsstr(a,b) strstr(a,b)
#	ifdef _WIN32
#		define GRS "s"               /* sprintf char* for Windows */
#		define GRW "S"               /* sprintf wchar_t* for Windows */
#		define GFS "%s"              /* sprintf char* for Windows */
#		define GFW "%S"              /* sprintf wchar_t* for Windows */
#		define GOS "%s\n"            /* sprintf char* for Windows */
#		define GOW "%S\n"            /* sprintf wchar_t* for Windows */
#		define GFSN(n) "%" n "s"     /* sprintf char* for Windows */
#		define GFWN(n) "%" n "S"     /* sprintf wchar_t* for Windows */
#	else
#		define GRS "s"               /* sprintf char* for POSIX */
#		define GRW "S"               /* sprintf wchar_t* for POSIX */
#		define GFS "%s"              /* sprintf char* for POSIX */
#		define GFW "%S"              /* sprintf wchar_t* for POSIX */
#		define GOS "%s\n"            /* sprintf char* for POSIX */
#		define GOW "%S\n"            /* sprintf wchar_t* for POSIX */
#		define GFSN(n) "%" n "s"     /* sprintf char* for Windows */
#		define GFWN(n) "%" n "S"     /* sprintf wchar_t* for Windows */
#	endif
#	define GRT GRS
#	define GFT GFS
#	define GOT GOS
#	define GFTN(n) GFSN(n)
#	define grstring std::string
#else
typedef wchar_t grchr_t;
typedef wchar_t* grstr_t;
#	define GS(a) L##a
#	define grscpy(a,b) wcscpy(a,b)
#	define grsncpy(a,b,n) wcsncpy(a,b,n)
#	define grslen(a) wcslen(a)
#	define grscmp(a,b) wcscmp(a,b)
#	define grsncmp(a,b,n) wcsncmp(a,b,n)
#	define grscasecmp(a,b) wcscasecmp(a,b)
#	define grsncasecmp(a,b,n) wcsncasecmp(a,b,n)
#	define grsdup(a) wcsdup(a)
#	define grscat(a,b) wcscat(a,b)
#	define grprintf wprintf
#	define grsprintf swprintf
#	define grsnprintf swprintf
#	define grfprintf fwprintf
#	define grvprintf vwprintf
#	define grvfprintf vwfprintf
#	define grvsprintf vswprintf
#	define grsfget fgetws
#	define grscanf wscanf
#	define grfscanf fwscanf
#	define grstok(a,b,c) wcstok(a,b,c)
#	define grstol(a,b,c) wcstol(a,b,c)
#	define grstoll(a,b,c) wcstoll(a,b,c)
#	define grstoul(a,b,c) wcstoul(a,b,c)
#	define grsstr(a,b) wcswcs(a,b)
#	ifdef _WIN32
#		define GRS L"S"                  /* wprintf char* for Windows */
#		define GRW L"s"                  /* wprintf wchar_t* for Windows */
#		define GFS L"%S"                 /* wprintf char* for Windows */
#		define GFW L"%s"                 /* wprintf wchar_t* for Windows */
#		define GOS L"%S\n"               /* wprintf char* for Windows */
#		define GOW L"%s\n"               /* wprintf wchar_t* for Windows */
#		define GFSN(n) L"%" L##n L"S"    /* wprintf char* for Windows */
#		define GFWN(n) L"%" L##n L"s"    /* wprintf wchar_t* for Windows */
#	else
#		define GRS L"s"                  /* wprintf char* for POSIX */
#		define GRW L"S"                  /* wprintf wchar_t* for POSIX */
#		define GFS L"%s"                 /* wprintf char* for POSIX */
#		define GFW L"%S"                 /* wprintf wchar_t* for POSIX */
#		define GOS L"%s\n"               /* wprintf char* for POSIX */
#		define GOW L"%S\n"               /* wprintf wchar_t* for POSIX */
#		define GFSN(n) L"%" L##n "s"     /* wprintf char* for Windows */
#		define GFWN(n) L"%" L##n "S"     /* wprintf wchar_t* for Windows */
#	endif
#	define GRT GRW
#	define GFT GFW
#	define GOT GOW
#	define GFTN(n) GFWN(n)
#	define grstring std::wstring
#endif
#define GRCSIZE sizeof(grchr_t)
static GRINLINE grstr_t grscasestr(const grstr_t Astr, const grstr_t Atok)
{
	grstr_t base = (grstr_t)Astr;
	for (; *base; base++) {
		grstr_t str = (grstr_t)base;
		grstr_t tok = (grstr_t)Atok;
#ifdef GR_UTF8
		/* XXX: tolower works for utf-8? */
		while (*str && (!*tok || tolower(*tok++) == tolower(*str++)))
#else
		while (*str && (!*tok || towlower(*tok++) == towlower(*str++)))
#endif
			if (!*tok)
				return base;
	}
	return NULL;
}

/*! \def ZNAME_LEN
 *  The general name string storage space limitation.
 */
#define ZNAME_LEN 64
/*! \def ZNAME_CAP
 *  The actual characters can be used for naming. it is ZNAME_LEN minus 1.
 */
#define ZNAME_CAP (ZNAME_LEN - 1)

/*! \typedef token_t
 *  \brief The fixed size string type as common token name.
 */
typedef grchr_t token_t[ZNAME_LEN];

#ifdef GR_UTF8
#	define GRTOK "=s64"
#else
#	define GRTOK "=t64"
#endif

typedef char atoken_t[ZNAME_LEN];
typedef wchar_t wtoken_t[ZNAME_LEN];

/*! \struct guid_t
 *  \brief The GUID type represent to be used around goldenrod project.
 */
typedef union
{
	uint8_t bytes[16];           /*!< the byte array form of guid */
	uint32_t dwords[4];          /*!< the double-word array form of guid */
	struct {
		uint32_t data1;        /*!< the first field of uuid, MSB */
		uint16_t data2;        /*!< the second field of uuid, MSB */
		uint16_t data3;        /*!< the third field of uuid, MSB */
		uint8_t data4[2];      /*!< the fourth field of uuid */
		uint8_t data5[6];      /*!< the fifth field of uuid */
	} uuid;                      /*!< the uuid structure form of guid */
} guid_t;

#define EMPTY_UUID "00000000-0000-0000-0000-000000000000"

/*! \struct timetable_t
 *  \brief time table definition, tipically to be used for scheduling.
 */
typedef struct
{
	uint16_t month;           //!< bit0-11: Jan-Dec, bit15: every month
	uint16_t weekday:7;       //!< Sunday-Saturday
	uint16_t everyWeekday:1;  //!< every day of week or not
	uint16_t useMWeek:1;      //!< the mweek takes effect or not
	uint16_t mweek:6;         //!< the week of month
	uint16_t everyWeek:1;     //!< every week of month or not
	uint32_t day;             //!< bit0-30: 1st-31st, bit31: every day
	uint32_t hour;            //!< bit0-23: 0-23 O'clock; bit31: every hour
	uint32_t activatedDEP:1;  //!< the item is activated(deprecated, running status should not be here, get unexpected if serialize/unserialize
	uint32_t dayAndWeek:1;    //!< day and weekday both should be matched
	uint32_t isDuration:1;    //!< it's a duration, otherwise a time point
	uint32_t pad2:29;         //!< pad for alignment
	uint64_t minute;          //!< bit0-59: 0-59 minute; bit63: every minutes
	uint64_t second;          //!< reserved
} timetable_t;
#define FMT_TIMETABLE  "cciiill"

/*! \struct vector8_t
 *  \brief A general purpose dual tuple definition.
 */
typedef struct {
	int8_t x;     /*!< the x part of 8-bit vector */
	int8_t y;     /*!< the y part of 8-bit vector */
} vector8_t;

/*! \struct vector16_t
 *  \brief A general purpose dual tuple definition.
 */
typedef struct
{
	int16_t x;     /*!< the x part of 16-bit vector */
	int16_t y;     /*!< the y part of 16-bit vector */
} vector16_t;

/*! \struct vector32_t
 *  \brief A general purpose dual tuple definition.
 */
typedef struct
{
	int32_t x;     /*!< the x part of 32-bit vector */
	int32_t y;     /*!< the y part of 32-bit vector */
} vector32_t;

/*! \struct vector64_t
 *  \brief A general purpose dual tuple definition.
 */
typedef struct
{
	int64_t x;     /*!< the x part of 64-bit vector */
	int64_t y;     /*!< the y part of 64-bit vector */
} vector64_t;

/*! \struct quarter8_t
 *  \brief A general purpose integer tetrad definition.
 */
typedef struct
{
	int8_t a;      /*!< the first int16_t part of quarter8_t */
	int8_t b;      /*!< the second int16_t part of quarter8_t */
	int8_t c;      /*!< the third int16_t part of quarter8_t */
	int8_t d;      /*!< the forth int16_t part of quarter8_t */
} quarter8_t;

/*! \struct quarter16_t
 *  \brief A general purpose integer tetrad definition.
 */
typedef struct
{
	int16_t a;      /*!< the first int16_t part of quarter16_t */
	int16_t b;      /*!< the second int16_t part of quarter16_t */
	int16_t c;      /*!< the third int16_t part of quarter16_t */
	int16_t d;      /*!< the forth int16_t part of quarter16_t */
} quarter16_t;

/*! \struct quarter32_t
 *  \brief A general purpose integer tetrad definition.
 */
typedef struct
{
	int32_t a;      /*!< the first int16_t part of quarter32_t */
	int32_t b;      /*!< the second int16_t part of quarter32_t */
	int32_t c;      /*!< the third int16_t part of quarter32_t */
	int32_t d;      /*!< the forth int16_t part of quarter32_t */
} quarter32_t;

/*! \struct quarter64_t
 *  \brief A general purpose integer tetrad definition.
 */
typedef struct
{
	int64_t a;      /*!< the first int16_t part of quarter64_t */
	int64_t b;      /*!< the second int16_t part of quarter64_t */
	int64_t c;      /*!< the third int16_t part of quarter64_t */
	int64_t d;      /*!< the forth int16_t part of quarter64_t */
} quarter64_t;

/*! \enum phase_t
 *  \brief A general purpose state definition used for state controlling.
 */
typedef enum
{
	ph_Initial,          /*!< the initial(start) state */
	ph_TransitingIn,     /*!< transitting state from initial */
	ph_Transitted,       /*!< state transitted(end) */
	ph_TransitingOut,    /*!< transitting state to initial */
	ph_Exception,        /*!< recoverable error phase */
	ph_Fatal,            /*!< unrecoverable error phase */
} phase_t;

/*! \enum data_opcode_t
 *  \brief the action code for general data manipulating.
 */
typedef enum
{
	dop_create = 0,      /*!< create new datum item */
	dop_update = 1,      /*!< update existed datum */
	dop_delete = 2,      /*!< erase datum */
	dop_search = 3,      /*!< inquire datum */
	dop_commit = 4,      /*!< finish a batch of operation */
	dop_notify = 5,      /*!< announce the creation of datum */
} data_opcode_t;

/*! \fn gr_callback_t
 *  \brief unified data callback with usage data.
 */
typedef void (GRCALL *gr_callback_t)(const void* Abuf, int Alen, void* Aud);

/*! \struct gr_junction_t
 *  \brief junction to report variant data accross module.
 */
typedef struct
{
	gr_callback_t cb;    /*!< callback function pointer */
	void* ud;            /*!< usage data as callback parameter */
} gr_junction_t;

// error code definitions

#define GE_OK                  0
#define GE_InvalidHandle      -1
#define GE_Unsupported        -2
#define GE_NoObjectFound      -3
#define GE_InProgress         -4
#define GE_NotReady           -5
#define GE_GeneralFail        -6
#define GE_DuplicatedObject   -7
#define GE_InvalidParameter   -8
#define GE_ModuleLoadFail     -9
#define GE_ModuleSymbolFail   -10
#define GE_Unauthorized       -11
#define GE_InvalidOperation   -12
#define GE_ResourceLimited    -13
#define GE_Deprecated         -14
#define GE_UnknownReason      -15
#define GE_InvalidResource    -16

#if defined(__cplusplus)
}
#endif

#endif // __UNI_TYPES_H__

