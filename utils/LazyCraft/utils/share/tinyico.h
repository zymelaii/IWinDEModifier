#ifndef TINYICO_H
#define TINYICO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#pragma pack(push)
#pragma pack(2)

typedef struct ti_bmpinfoheader_s {
	uint32_t biSize;
	int32_t	 biWidth;
	int32_t	 biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t	 biXPelsPerMeter;
	int32_t	 biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} ti_bmpinfoheader_t;

typedef union ti_colrgb_s {
	struct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t reserved;
	};
	uint32_t u32col;
} ti_colrgb_t;

typedef union ti_image_s {
	struct {						 //!< old XOR/AND DIB format
		ti_bmpinfoheader_t header;	 //!< DIB info header
									 //!< note: only biSize, biWidth, biHeight, biPlanes,
									 //!< biBitCount, biSizeImage are used, other members
									 //!< must be 0. notice that biXPelsPerMeter and
									 //!< biYPelsPerMeter may have values in 32bpp DIBs.
		ti_colrgb_t palette[1];		 //!< color table
		uint8_t		xorbits[1];		 //!< DIB bits for XOR mask
		uint8_t		andbits[1];		 //!< DIB bits for AND mask, assuming 1bpp
	};
	struct {					//!< new PNG format
		uint8_t signature[8];	//!< PNG signature (i.e. {
								//!< 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a
								//!< })
	};
} ti_image_t;

typedef struct ti_entry_s {
	uint8_t width;		 //!< width of image (in pixels)
	uint8_t height;		 //!< height of image (in pixels)
						 //!< note: size provided by the entry may differ from image data,
						 //!< use the size provided by image data if necessary
	uint8_t	 ncol;		 //!< number of colors in image (0 if >= 8bpp)
	uint8_t	 reserved;	 //!< 0 required
	uint16_t planes;	 //!< color planes
	uint16_t bpp;		 //!< bits per pixel
	uint32_t nbytes;	 //!< size of image (in bytes)
	uint32_t offset;	 //!< offset to the image of current entry (from icon's beginning)
} ti_entry_t;

typedef struct ti_dfentry_s {
	uint8_t width;		 //!< width of image (in pixels)
	uint8_t height;		 //!< height of image (in pixels)
						 //!< note: size provided by the entry may differ from image data,
						 //!< use the size provided by image data if necessary
	uint8_t	 ncol;		 //!< number of colors in image (0 if >= 8bpp)
	uint8_t	 reserved;	 //!< 0 required
	uint16_t planes;	 //!< color planes
	uint16_t bpp;		 //!< bits per pixel
	uint32_t nbytes;	 //!< size of image (in bytes)
	uint16_t id;		 //!< ID of image resource
} ti_dfentry_t;

typedef struct ti_dir_s {
	uint16_t   reserved;	 //!< 0 required
	uint16_t   type;		 //!< resource type (0 for cursors, 1 for icons)
	uint16_t   count;		 //!< number of image
	ti_entry_t entries[1];	 //!< entries of all images
} ti_dir_t;

typedef struct ti_dfdir_s {
	uint16_t	 reserved;	   //!< 0 required
	uint16_t	 type;		   //!< resource type (0 for cursors, 1 for icons)
	uint16_t	 count;		   //!< number of image
	ti_dfentry_t entries[1];   //!< entries of all images
} ti_dfdir_t;

typedef struct ti_resource_s {
	void*	 handle;
	size_t	 nbytes;
	ti_dir_t directory;
} ti_resource_t;

#pragma pack(pop)

enum TINYICO_ERROR {
	E_NULL = 0,
	E_ILLEGAL_PARAMS,
	E_BAD_ICON_HEADER,
	E_BAD_ICON_IMAGE,
	E_BROKEN_ICON,
	E_BAD_ALLOC,
	E_INVALID_HRES,
	E_UNDEFINED,
	E_OUT_OF_RANGE,
};

int ti_memopen(ti_resource_t** pres, uint8_t** pdata, size_t len);
int ti_dfopen(ti_resource_t** pres, uint8_t** pdata, size_t len);
int ti_close(ti_resource_t** pres);
int ti_extract_as_argb(const ti_resource_t* res, uint8_t** ppixels, int index);
int ti_strentryinfo(const ti_resource_t* res, int index, char* buf, int szbuf);

#ifdef __cplusplus
}
#endif

#endif /* TINYICO_H */