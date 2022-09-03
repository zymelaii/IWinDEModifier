#ifndef TINYICO_H
#define TINYICO_H

#include <stdint.h>
#include <assert.h>
#include <malloc.h>
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

#define TINYICON_IMPLEMENT
#ifdef TINYICON_IMPLEMENT

void ti__convert_4bpp_to_argb(uint8_t* const bytes, const ti_bmpinfoheader_t* const info,
							  const uint8_t* const data) {
	ti_colrgb_t palette[16];
	memcpy(palette, data, sizeof(ti_colrgb_t) * info->biClrUsed);

	const size_t   npixels	= info->biWidth * info->biHeight;
	const uint8_t* index	= data + sizeof(ti_colrgb_t) * info->biClrUsed;
	const uint8_t* maskbits = index + npixels / 2;

	for (int bit = 0; bit < 8; bit += 2) {
		ti_colrgb_t* col = (ti_colrgb_t*)bytes + bit;
		for (int i = bit; i < npixels; i += 8) {
			ti_colrgb_t tmp = palette[index[i / 2] & 0x0f];
			col[0].r		= tmp.b;
			col[0].g		= tmp.g;
			col[0].b		= tmp.r;
			col[0].reserved = (maskbits[i / 8] & (0x80 >> bit)) ? 0x00 : 0xff;
			tmp				= palette[index[i / 2] & 0xf0];
			col[1].r		= tmp.b;
			col[1].g		= tmp.g;
			col[1].b		= tmp.r;
			col[1].reserved = (maskbits[i / 8] & (0x80 >> bit)) ? 0x00 : 0xff;
			col += 8;
		}
	}
}

void ti__convert_8bpp_to_argb(uint8_t* const bytes, const ti_bmpinfoheader_t* const info,
							  const uint8_t* const data) {
	ti_colrgb_t palette[256];
	memcpy(palette, data, sizeof(ti_colrgb_t) * info->biClrUsed);

	const size_t   npixels	= info->biWidth * info->biHeight;
	const uint8_t* index	= data + sizeof(ti_colrgb_t) * info->biClrUsed;
	const uint8_t* maskbits = index + npixels;

	for (int bit = 0; bit < 8; ++bit) {
		ti_colrgb_t *col = (ti_colrgb_t*)bytes + bit, tmp;
		for (int i = bit; i < npixels; i += 8) {
			tmp				= palette[index[i]];
			col[0].r		= tmp.b;
			col[0].g		= tmp.g;
			col[0].b		= tmp.r;
			col[0].reserved = (maskbits[i / 8] & (0x80 >> bit)) ? 0x00 : 0xff;
			col += 8;
		}
	}
}

void ti__convert_16bpp_to_argb(uint8_t* const bytes, const ti_bmpinfoheader_t* const info,
							   const uint8_t* const data) {}

void ti__convert_24bpp_to_argb(uint8_t* const bytes, const ti_bmpinfoheader_t* const info,
							   const uint8_t* const data) {}

void ti__convert_32bpp_to_argb(uint8_t* const bytes, const ti_bmpinfoheader_t* const info,
							   const uint8_t* const data) {
	const size_t	   npixels = info->biWidth * info->biHeight;
	ti_colrgb_t*	   dest	   = (ti_colrgb_t*)bytes;
	const ti_colrgb_t* src	   = (ti_colrgb_t*)data;
	for (int i = 0; i < npixels; ++i) {
		dest[i].u32col = src[i].u32col;
		dest[i].r	   = src[i].b;
		dest[i].b	   = src[i].r;
	}
}

void ti__flip_argb_vertically(uint8_t* pixels, int width, int height) {
	assert(pixels != NULL && "ti__flip_argb_vertically requires valid data as param");

	const size_t szcache = 256;
	const size_t stride	 = width * 4;
	uint8_t *	 first = pixels, *second = pixels + (height - 1) * stride;
	uint8_t		 cache[szcache];

	while (first < second) {
		int bytes = stride;
		while (bytes >= szcache) {
			memcpy(cache, first + bytes - szcache, szcache);
			memcpy(first + bytes - szcache, second + bytes - szcache, szcache);
			memcpy(second + bytes - szcache, cache, szcache);
			bytes -= szcache;
		}
		memcpy(cache, first, bytes);
		memcpy(first, second, bytes);
		memcpy(second, cache, bytes);
		first += stride;
		second -= stride;
	}
}

int ti__extract_as_argb(const uint8_t* const data, uint8_t** ppixels) {
	assert(data != NULL && "ti__extract_as_argb requires valid data as param");

	ti_image_t image;
	memcpy(&image, data, sizeof(ti_bmpinfoheader_t));

	const uint8_t png_sign[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	if (memcmp(image.signature, png_sign, 8) == 0) {
		return E_UNDEFINED;
	}

	const size_t   width  = image.header.biWidth;
	const size_t   height = image.header.biHeight / 2;
	const size_t   nbytes = width * height * 4;
	uint8_t* const pixels = (uint8_t*)malloc(nbytes);
	if (!pixels) {
		return E_BAD_ALLOC;
	}

	image.header.biHeight /= 2;
	const ti_bmpinfoheader_t* const header	   = &image.header;
	const uint8_t* const			image_data = data + sizeof(ti_bmpinfoheader_t);

	if (header->biBitCount > 8 && header->biClrUsed != 0) {
		free(pixels);
		return E_UNDEFINED;
	}

	if (header->biBitCount == 16 || header->biBitCount == 24) {
		free(pixels);
		return E_UNDEFINED;
	}

	switch (header->biBitCount) {
		case 4: {
			ti__convert_4bpp_to_argb(pixels, header, image_data);
			break;
		}
		case 8: {
			ti__convert_8bpp_to_argb(pixels, header, image_data);
			break;
		}
		case 16: {
			ti__convert_16bpp_to_argb(pixels, header, image_data);
			break;
		}
		case 24: {
			ti__convert_24bpp_to_argb(pixels, header, image_data);
			break;
		}
		case 32: {
			ti__convert_32bpp_to_argb(pixels, header, image_data);
			break;
		}
		default: {
			free(pixels);
			return E_UNDEFINED;
		}
	}

	ti__flip_argb_vertically(pixels, width, height);
	*ppixels = pixels;

	return E_NULL;
}

int ti_memopen(ti_resource_t** pres, uint8_t** pdata, size_t len) {
	assert(pres != NULL && *pres == NULL && "ti_memopen requires pointer to null resource ptr");
	assert(pdata != NULL && *pdata != NULL && "ti_memopen requires pointer to valid data");

	if (len > sizeof(ti_dir_t)) {
		return E_ILLEGAL_PARAMS;
	}

	const uint8_t* data = *pdata;
	ti_dir_t	   dir	= *(ti_dir_t*)data;

	if (dir.reserved != 0 || dir.type != 1 || dir.count == 0) {
		return E_BAD_ICON_HEADER;
	}

	const size_t tmpsize  = (dir.count - 1) * sizeof(ti_entry_t);
	const size_t szheader = sizeof(ti_resource_t) + tmpsize;
	if (szheader >= len) {
		return E_BROKEN_ICON;
	}

	ti_resource_t* res = (ti_resource_t*)malloc(szheader);
	if (!res) {
		return E_BAD_ALLOC;
	}

	res->handle	   = pdata;
	res->nbytes	   = len;
	res->directory = dir;
	memcpy(res->directory.entries + 1, data + sizeof(ti_dir_t), tmpsize);

	*pres = res;

	return E_NULL;
}

int ti_dfopen(ti_resource_t** pres, uint8_t** pdata, size_t len) {
	assert(pres != NULL && *pres == NULL && "ti_memopen requires pointer to null resource ptr");
	assert(pdata != NULL && *pdata != NULL && "ti_memopen requires pointer to valid data");

	if (len < sizeof(ti_dfdir_t)) {
		return E_ILLEGAL_PARAMS;
	}

	const uint8_t* data = *pdata;
	ti_dir_t	   dir;
	memcpy(&dir, data, sizeof(uint16_t) * 3);

	if (dir.reserved != 0 || dir.type != 1 || dir.count == 0) {
		return E_BAD_ICON_HEADER;
	}

	const size_t tmpsize  = (dir.count - 1) * sizeof(ti_dfentry_t);
	const size_t szheader = sizeof(ti_resource_t) - sizeof(ti_dir_t) + sizeof(ti_dfdir_t) + tmpsize;
	if (sizeof(ti_dfdir_t) + tmpsize != len) {
		return E_BROKEN_ICON;
	}

	ti_resource_t* res = (ti_resource_t*)malloc(szheader);
	if (!res) {
		return E_BAD_ALLOC;
	}

	res->handle = pdata;
	res->nbytes = len;
	memcpy(&res->directory, data, sizeof(ti_dfdir_t));
	memcpy((ti_dfentry_t*)res->directory.entries + 1, data + sizeof(ti_dfdir_t), tmpsize);

	*pres = res;

	return E_NULL;
}

int ti_close(ti_resource_t** pres) {
	if (!pres || !*pres) {
		return E_INVALID_HRES;
	}

	free(*pres);
	*pres = NULL;

	return E_NULL;
}

int ti_extract_as_argb(const ti_resource_t* res, uint8_t** ppixels, int index) {
	if (!res) {
		return E_INVALID_HRES;
	}
	if (index < 0 || index >= res->directory.count) {
		return E_OUT_OF_RANGE;
	}

	const uint8_t* data = *(uint8_t**)res->handle + res->directory.entries[index].offset;

	return ti__extract_as_argb(data, ppixels);
}

#endif

#endif /* TINYICO_H */