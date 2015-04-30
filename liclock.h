#ifndef LICENSE_H
#define LICENSE_H

#include <grutil.hpp>
#include <grutilex.hpp>
#include <sysinc.hpp>
#include <license.h>

#if defined(__cplusplus)
extern "C"{
#endif

//error code
#define HY_ENODEVICE        0x01
#define HY_EOUTMEM          0x02
#define HY_EENUMDEV         0x03
#define HY_EOPENDEV         0x04
#define HY_EROOTDIR         0x05
#define HY_EUSERPIN         0x06
#define HY_EDEVPIN          0x07

typedef struct{
    uint64_t id;
    uint64_t lower;
    uint64_t upper;
}hy_license_quota_t;

typedef struct{
    uint32_t len;
    uint32_t pad;
}hy_license_head_t;

typedef struct{
    uint16_t total;
    uint16_t nentry;
    uint16_t pad;
    hy_license_quota_t *entries;
}hy_license_body_t;

typedef struct{
    hy_license_head_t head;
    hy_license_body_t body;
}hy_license_t;

typedef struct{
    char *licpath;
    char *licext;
    char *licprefix;
}hy_license_init_t;

typedef enum{
    lic_NVR,
    lic_TVW,
    lic_CLIENT
}hy_license_type_t;

typedef enum{
    LRT_SUCCESS     = 0,
    LRT_NO_INIT     = 1,
    LRT_NO_LIC      = 2,
    LRT_NO_MATCH    = 3,
    LRT_DAMAGE      = 4,
    LRT_LIMIT       = 5,
    LRT_DUP_LIC     = 6,
    LRT_FAILURE     = 7,
	LRT_SN_ERROR	= 8
}hy_ret_t;

typedef struct{
    uint64_t req_cam_num;
    uint64_t req_screen_num;
    gr_license_t lic;
}hy_dev_license_t;

//key
int GRCALL hy_verify(unsigned char sn[22], int Alen);
int GRCALL hy_reset();
int GRCALL hy_write_file(const char *Aid, const char* Apath, int Aoverwrite);
//quota
int GRCALL hy_get_quota(uint64_t id, uint64_t *lower, uint64_t *upper);
int GRCALL hy_set_quota(uint64_t id, uint64_t lower, uint64_t upper);
//license
int GRCALL hy_license_init(hy_license_init_t *Ainit);
int GRCALL hy_license_destroy();
int GRCALL hy_active_dev_license(hy_dev_license_t *Alic);
int GRCALL hy_load_dev_license(hy_dev_license_t *Alic);

#if defined(__cplusplus)
}
#endif



#endif // LICENSE_H
