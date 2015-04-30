#if !defined(__GR_LICENSE_H__)
#define __GR_LICENSE_H__

//#include "unitypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GRLIC_MAGIC_CODE  0x6332387437588645ULL

/*! \struct gr_credit_path_t
 *  \brief the path to specify a license item.
 */
typedef struct
{
	uint32_t module:6;       /*!< grl_module_t enumerations */
	uint32_t facet:4;        /*!< module specific grlf_* enumerations */
	uint32_t func:8;         /*!< module/facet specific grlff_* enumerations */
	uint32_t point:8;        /*!< module/facet/func specific grlffp_* enumerations */
	uint32_t pad:6;
} gr_credit_path_t;

typedef struct
{
    uint64_t lower;
    uint64_t upper;
}gr_credit_quota_t;

typedef struct
{
	uint32_t valType:5;
	uint32_t len:22;
	uint32_t pad:5;
} gr_credit_meta_t;

typedef struct
{
	gr_credit_path_t path;
	gr_credit_meta_t meta;
    gr_credit_quota_t quota;
} gr_credit_entry_t;

typedef struct
{
    unsigned char digest[20];
    unsigned char sn[22];
    uint16_t pad;
	uint32_t pad1;
    uint32_t entries;
    uint16_t type;
    uint16_t version;
    uint64_t effect;
    uint64_t expired;
}gr_license_head_t;

typedef struct
{
    uint64_t numval;
    char* strval;
    void* blobval;
    uint64_t pad;
}gr_licence_entry_val_t;

typedef struct
{
    uint64_t id;
    gr_credit_entry_t credit;
    gr_licence_entry_val_t val;
}gr_license_entry_t;

typedef struct
{
    gr_license_entry_t *entries;
}gr_license_body_t;

typedef struct
{
    gr_license_head_t head;
    gr_license_body_t body;
} gr_license_t;

typedef enum
{
	glm_undefined = 0,
	glm_general1 = 1,
	glm_general2 = 2,
	glm_general3 = 3,
	glm_general4 = 4,
	glm_general5 = 5,
	glm_access = 6,             /*!< shanghai and sh plugins' common */
	glm_dispatcher = 7,         /*!< zhengzhou */
	glm_realtime = 8,           /*!< chameleon */
	glm_matrix = 9,             /*!< anole */
	glm_mat_designer = 10,      /*!< anole client */
	glm_recorder = 11,          /*!< sichuan */
	glm_playback = 12,          /*!< xian/chemeleon playback */
	glm_directory = 13,         /*!< beijing */
	glm_dir_client = 14,        /*!< tianjin */
	glm_cor_engine = 15,        /*!< wuhan */
	glm_cor_designer = 16,      /*!< chameleon correlation designer */
	glm_mediaplay = 17,
} grl_module_t;

//belong to glm_general4 module
typedef enum
{
	glfq_issuer = 1,
	glfq_issue_to = 2,
}grlf_general_lic_t;

typedef enum
{
	glfq_max_cam = 1,
	glfq_max_screen = 2,
	glfq_max_trial_cam = 3,
	glfq_max_trial_screen = 4
}grlf_general5_quota_t;

typedef enum
{
	glfa_connection = 1,
} grlf_access_t;

typedef enum
{
	glfa_conn_net = 1,
}grlff_access_network_t;

typedef enum
{
	glanp_max_cam = 1,
} grlffp_access_net_point_t;

typedef enum
{
	glfdp_max_cli = 1,
	glfdp_max_forward = 2,
	glfdp_cam_soft_conn = 3,
	glfdp_cam_hard_conn = 4,
	glfdp_max_cpu = 5
} grlf_dispatcher_t;

typedef enum
{
	glfm_control = 1,
	glfm_multi_screen = 2
}grlf_matrix_t;

typedef enum
{
	glfmp_soft = 1,              /* !< integer: maximum soft designercoding limit */
	glfmp_hard = 2,              /* !< integer: maximum hwacclcl decoding limit */
	glfmp_max = 3,               /* !< integer: total decoding/rendering limit */
} grlf_mediaplay_t;

typedef enum
{
	glfr_map = 1,
	glfr_multi_screen = 2,
	glfr_layout_row = 3,
	glfr_layout_col = 4,
	glfr_matrix_client = 5
}grlf_realtime_t;

typedef enum
{
	glfr_iscsi = 1,
	glfr_max_client = 2,
	glfr_bak_group = 3
}grlf_recorder_t;

/*! \fn grlic_load
 *  \brief load the license object.
 *  \param Afname the license object to be freed.
 *  \param Alic the license object to be freed.
 *  \return GE_OK if succeeds, other error code if failed.
 */

int grlic_load(const char* Afname, gr_license_t* Alic);
int grlic_save(const char* Afname, const gr_license_t* Alic);
int grlic_take(gr_license_t* Alic);

/* ! \fn grlic_gen_sn
 *   \generate unique machine serial number
 *   \param Asn store serial number generated
 *   \param Als indicate device type. see grl_dev_t enum
 */
int grlic_gen_sn(unsigned char Asn[17]);
int grlic_gen_sns(unsigned char **Asn, int Alen); 
int grlic_cmp_sn(unsigned char Asn[17]);
uint64_t grlic_path_id(gr_credit_path_t* Apath);

/*! \fn grlic_free
 *  \brief release the license object.
 *  \param Alic the license object to be freed.
 *  \return GE_OK if succeeds, otherwise return error code.
 */
int grlic_free(gr_license_t* Alic);

/*! \fn grlic_get_bool
 *  \brief get credit item boolean value.
 *  \param Alic the license object.
 *  \param Apath the credit item path.
 *  \param Aval the reference to save the boolean value.
 *  \return GE_OK if read it, other error code if failed.
 */
int grlic_get_bool(gr_license_t* Alic, gr_credit_path_t* Apath, int* Aval);

/*! \fn grlic_get_int
 *  \brief get credit item integer value.
 *  \param Alic the license object.
 *  \param Apath the credit item path.
 *  \param Aval the reference to save the integet value.
 *  \return GE_OK if read it, other error code if failed.
 */
int grlic_get_int(gr_license_t* Alic, gr_credit_path_t* Apath, int* Aval);

/*! \fn grlic_get_str
 *  \brief get credit item string value.
 *  \param Alic the license object.
 *  \param Apath the credit item path.
 *  \param Aval the reference to save the string value.
 *  \param Alen the string length with the terminate NULL character.
 *  \return GE_OK if read it, other error code if failed.
 */
int grlic_get_str(gr_license_t* Alic, gr_credit_path_t* Apath, char* Astr, int* Alen);

/*! \fn grlic_get_blob
 *  \brief get credit item binary chunk value.
 *  \param Alic the license object.
 *  \param Apath the credit item path.
 *  \param Aval the reference to save the string value.
 *  \param Alen the chunk length.
 *  \return GE_OK if read it, other error code if failed.
 */
int grlic_get_blob(gr_license_t* Alic, gr_credit_path_t* Apath, void* Astr, int* Alen);

#ifdef __cplusplus
}
#endif

#endif // __GR_LICENSE_H__

