

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

// To get a header file for this, either cut and paste the header,
// or create stb_image.h, #define STBI_HEADER_FILE_ONLY, and
// then include stb_image.c from it.

////   begin header file  ////////////////////////////////////////////////////
//
// Limitations:
//    - no jpeg progressive support
//    - non-HDR formats support 8-bit samples only (jpeg, png)
//    - no delayed line count (jpeg) -- IJG doesn't support either
//    - no 1-bit BMP
//    - GIF always returns *comp=4
//
// Basic usage (see HDR discussion below):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ... 
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x       -- outputs image width in pixels
//    int *y       -- outputs image height in pixels
//    int *comp    -- outputs # of image components in image file
//    int req_comp -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'req_comp' if req_comp is non-zero, or *comp otherwise.
// If req_comp is non-zero, *comp has the number of components that _would_
// have been output otherwise. E.g. if you set req_comp to 4, you will always
// get RGBA output, but you can check *comp to easily see if it's opaque.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *comp will be unchanged. The function stbi_failure_reason()
// can be queried for an extremely brief, end-user unfriendly explanation
// of why the load failed. Define STBI_NO_FAILURE_STRINGS to avoid
// compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG, BMP, GIF, and PIC images are automatically depalettized.
//
// ===========================================================================
//
// iPhone PNG support:
//
// By default we convert iphone-formatted PNGs back to RGB; nominally they
// would silently load as BGR, except the existing code should have just
// failed on such iPhone PNGs. But you can disable this conversion by
// by calling stbi_convert_iphone_png_to_rgb(0), in which case
// you will always just get the native iphone "format" through.
//
// Call stbi_set_unpremultiply_on_load(1) as well to force a divide per
// pixel to remove any premultiplied alpha *only* if the image file explicitly
// says there's premultiplied data (currently only happens in iPhone images,
// and only if iPhone convert-to-rgb processing is on).
//
// ===========================================================================


#ifndef STBI_NO_STDIO

#if defined(_MSC_VER) && _MSC_VER >= 0x1400
#define _CRT_SECURE_NO_WARNINGS // suppress bogus warnings about fopen()
#endif

#include <stdio.h>
#endif

#define STBI_VERSION 1

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4
};

typedef unsigned char stbi_uc;

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on images of any type
//

//
// load image by filename, open file, or memory buffer
//

extern stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
extern stbi_uc *stbi_load            (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_load_from_file  (FILE *f,                  int *x, int *y, int *comp, int req_comp);
// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

typedef struct
{
   int      (*read)  (void *user,char *data,int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read 
   void     (*skip)  (void *user,unsigned n);            // skip the next 'n' bytes
   int      (*eof)   (void *user);                       // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

extern stbi_uc *stbi_load_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

// stbi_is_hdr is always defined
extern int    stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
extern int    stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
#ifndef STBI_NO_STDIO
extern int      stbi_is_hdr          (char const *filename);
extern int      stbi_is_hdr_from_file(FILE *f);
#endif // STBI_NO_STDIO


// get a VERY brief reason for failure
// NOT THREADSAFE
extern const char *stbi_failure_reason  (void); 

// free the loaded image -- this is just free()
extern void     stbi_image_free      (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
extern int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
extern int      stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
extern int      stbi_info            (char const *filename,     int *x, int *y, int *comp);
extern int      stbi_info_from_file  (FILE *f,                  int *x, int *y, int *comp);

#endif



// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
extern void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
extern void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);


// ZLIB client - used by PNG, available for other purposes

extern char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
extern char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

extern char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);


// define faster low-level operations (typically SIMD support)
#ifdef STBI_SIMD
typedef void (*stbi_idct_8x8)(stbi_uc *out, int out_stride, short data[64], unsigned short *dequantize);
// compute an integer IDCT on "input"
//     input[x] = data[x] * dequantize[x]
//     write results to 'out': 64 samples, each run of 8 spaced by 'out_stride'
//                             CLAMP results to 0..255
typedef void (*stbi_YCbCr_to_RGB_run)(stbi_uc *output, stbi_uc const  *y, stbi_uc const *cb, stbi_uc const *cr, int count, int step);
// compute a conversion from YCbCr to RGB
//     'count' pixels
//     write pixels to 'output'; each pixel is 'step' bytes (either 3 or 4; if 4, write '255' as 4th), order R,G,B
//     y: Y input channel
//     cb: Cb input channel; scale/biased to be 0..255
//     cr: Cr input channel; scale/biased to be 0..255

extern void stbi_install_idct(stbi_idct_8x8 func);
extern void stbi_install_YCbCr_to_RGB(stbi_YCbCr_to_RGB_run func);
#endif // STBI_SIMD


#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBI_INCLUDE_STB_IMAGE_H

#ifndef STBI_HEADER_FILE_ONLY

#ifndef STBI_NO_HDR
#include <math.h>  // ldexp
#include <string.h> // strcmp, strtok
#endif
#define NDEBUG
#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>

#ifndef _MSC_VER
   #ifdef __cplusplus
   #define stbi_inline inline
   #else
   #define stbi_inline
   #endif
#else
   #define stbi_inline __forceinline
#endif


// implementation:
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef   signed short  int16;
typedef unsigned int   uint32;
typedef   signed int    int32;
typedef unsigned int   uint;

// should produce compiler error if size is wrong
typedef unsigned char validate_uint32[sizeof(uint32)==4 ? 1 : -1];

#if defined(STBI_NO_STDIO) && !defined(STBI_NO_WRITE)
#define STBI_NO_WRITE
#endif

#define STBI_NOTUSED(v)  (void)sizeof(v)

#ifdef _MSC_VER
#define STBI_HAS_LROTL
#endif

#ifdef STBI_HAS_LROTL
   #define stbi_lrot(x,y)  _lrotl(x,y)
#else
   #define stbi_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif

///////////////////////////////////////////////
//
//  stbi struct and start_xxx functions

// stbi structure is our basic context used by all images, so it
// contains all the IO context, plus some basic image information
typedef struct
{
   uint32 img_x, img_y;
   int img_n, img_out_n;
   
   stbi_io_callbacks io;
   void *io_user_data;

   int read_from_callbacks;
   int buflen;
   uint8 buffer_start[128];

   uint8 *img_buffer, *img_buffer_end;
   uint8 *img_buffer_original;
} stbi;


static void refill_buffer(stbi *s);

// initialize a memory-decode context
static void start_mem(stbi *s, uint8 const *buffer, int len)
{
   s->io.read = NULL;
   s->read_from_callbacks = 0;
   s->img_buffer = s->img_buffer_original = (uint8 *) buffer;
   s->img_buffer_end = (uint8 *) buffer+len;
}

// initialize a callback-based context
static void start_callbacks(stbi *s, stbi_io_callbacks *c, void *user)
{
   s->io = *c;
   s->io_user_data = user;
   s->buflen = sizeof(s->buffer_start);
   s->read_from_callbacks = 1;
   s->img_buffer_original = s->buffer_start;
   refill_buffer(s);
}

#ifndef STBI_NO_STDIO

static int stdio_read(void *user, char *data, int size)
{
   return (int) fread(data,1,size,(FILE*) user);
}

static void stdio_skip(void *user, unsigned n)
{
   fseek((FILE*) user, n, SEEK_CUR);
}

static int stdio_eof(void *user)
{
   return feof((FILE*) user);
}

static stbi_io_callbacks stbi_stdio_callbacks =
{
   stdio_read,
   stdio_skip,
   stdio_eof,
};

static void start_file(stbi *s, FILE *f)
{
   start_callbacks(s, &stbi_stdio_callbacks, (void *) f);
}

//static void stop_file(stbi *s) { }

#endif // !STBI_NO_STDIO

static void stbi_rewind(stbi *s)
{
   // conceptually rewind SHOULD rewind to the beginning of the stream,
   // but we just rewind to the beginning of the initial buffer, because
   // we only use it after doing 'test', which only ever looks at at most 92 bytes
   s->img_buffer = s->img_buffer_original;
}

static int      stbi_png_test(stbi *s);
static stbi_uc *stbi_png_load(stbi *s, int *x, int *y, int *comp, int req_comp);
static int      stbi_png_info(stbi *s, int *x, int *y, int *comp);

// this is not threadsafe
static const char *failure_reason;

const char *stbi_failure_reason(void)
{
   return failure_reason;
}

static int e(const char *str)
{
   failure_reason = str;
   return 0;
}

// e - error
// epf - error returning pointer to float
// epuc - error returning pointer to unsigned char

#ifdef STBI_NO_FAILURE_STRINGS
   #define e(x,y)  0
#elif defined(STBI_FAILURE_USERMSG)
   #define e(x,y)  e(y)
#else
   #define e(x,y)  e(x)
#endif

#define epf(x,y)   ((float *) (e(x,y)?NULL:NULL))
#define epuc(x,y)  ((unsigned char *) (e(x,y)?NULL:NULL))

void stbi_image_free(void *retval_from_stbi_load)
{
   free(retval_from_stbi_load);
}

static unsigned char *stbi_load_main(stbi *s, int *x, int *y, int *comp, int req_comp)
{
   if (stbi_png_test(s))  return stbi_png_load(s,x,y,comp,req_comp);
   return epuc("unknown image type", "Image not of any known type, or corrupt");
}

#ifndef STBI_NO_STDIO
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   FILE *f = fopen(filename, "rb");
   unsigned char *result;
   if (!f) return epuc("can't fopen", "Unable to open file");
   result = stbi_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}

unsigned char *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_file(&s,f);
   return stbi_load_main(&s,x,y,comp,req_comp);
}
#endif //!STBI_NO_STDIO

unsigned char *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_mem(&s,buffer,len);
   return stbi_load_main(&s,x,y,comp,req_comp);
}

unsigned char *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi_load_main(&s,x,y,comp,req_comp);
}


//////////////////////////////////////////////////////////////////////////////
//
// Common code used by all image loaders
//

enum
{
   SCAN_load=0,
   SCAN_type,
   SCAN_header
};

static void refill_buffer(stbi *s)
{
   int n = (s->io.read)(s->io_user_data,(char*)s->buffer_start,s->buflen);
   if (n == 0) {
      // at end of file, treat same as if from memory
      s->read_from_callbacks = 0;
      s->img_buffer = s->img_buffer_end-1;
      *s->img_buffer = 0;
   } else {
      s->img_buffer = s->buffer_start;
      s->img_buffer_end = s->buffer_start + n;
   }
}

stbi_inline static int get8(stbi *s)
{
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;
   if (s->read_from_callbacks) {
      refill_buffer(s);
      return *s->img_buffer++;
   }
   return 0;
}

stbi_inline static int at_eof(stbi *s)
{
   if (s->io.read) {
      if (!(s->io.eof)(s->io_user_data)) return 0;
      // if feof() is true, check if buffer = end
      // special case: we've only got the special 0 character at the end
      if (s->read_from_callbacks == 0) return 1;
   }

   return s->img_buffer >= s->img_buffer_end;   
}

stbi_inline static uint8 get8u(stbi *s)
{
   return (uint8) get8(s);
}

static void skip(stbi *s, int n)
{
   if (s->io.read) {
      int blen = s->img_buffer_end - s->img_buffer;
      if (blen < n) {
         s->img_buffer = s->img_buffer_end;
         (s->io.skip)(s->io_user_data, n - blen);
         return;
      }
   }
   s->img_buffer += n;
}

static int getn(stbi *s, stbi_uc *buffer, int n)
{
   if (s->io.read) {
      int blen = s->img_buffer_end - s->img_buffer;
      if (blen < n) {
         int res, count;

         memcpy(buffer, s->img_buffer, blen);
         
         count = (s->io.read)(s->io_user_data, (char*) buffer + blen, n - blen);
         res = (count == (n-blen));
         s->img_buffer = s->img_buffer_end;
         return res;
      }
   }

   if (s->img_buffer+n <= s->img_buffer_end) {
      memcpy(buffer, s->img_buffer, n);
      s->img_buffer += n;
      return 1;
   } else
      return 0;
}

static int get16(stbi *s)
{
   int z = get8(s);
   return (z << 8) + get8(s);
}

static uint32 get32(stbi *s)
{
   uint32 z = get16(s);
   return (z << 16) + get16(s);
}

static int get16le(stbi *s)
{
   int z = get8(s);
   return z + (get8(s) << 8);
}

static uint32 get32le(stbi *s)
{
   uint32 z = get16le(s);
   return z + (get16le(s) << 16);
}

//////////////////////////////////////////////////////////////////////////////
//
//  generic converter from built-in img_n to req_comp
//    individual types do this automatically as much as possible (e.g. jpeg
//    does all cases internally since it needs to colorspace convert anyway,
//    and it never has alpha, so very few cases ). png can automatically
//    interleave an alpha=255 channel, but falls back to this for other cases
//
//  assume data buffer is malloced, so malloc a new one and free that one
//  only failure mode is malloc failing

static uint8 compute_y(int r, int g, int b)
{
   return (uint8) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static unsigned char *convert_format(unsigned char *data, int img_n, int req_comp, uint x, uint y)
{
   int i,j;
   unsigned char *good;

   if (req_comp == img_n) return data;
   assert(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) malloc(req_comp * x * y);
   if (good == NULL) {
      free(data);
      return epuc("outofmem", "Out of memory");
   }

   for (j=0; j < (int) y; ++j) {
      unsigned char *src  = data + j * x * img_n   ;
      unsigned char *dest = good + j * x * req_comp;

      #define COMBO(a,b)  ((a)*8+(b))
      #define CASE(a,b)   case COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
      // convert source image with img_n components to one with req_comp components;
      // avoid switch per pixel, so use switch per scanline and massive macros
      switch (COMBO(img_n, req_comp)) {
         CASE(1,2) dest[0]=src[0], dest[1]=255; break;
         CASE(1,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(1,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=255; break;
         CASE(2,1) dest[0]=src[0]; break;
         CASE(2,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(2,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1]; break;
         CASE(3,4) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255; break;
         CASE(3,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(3,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = 255; break;
         CASE(4,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(4,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = src[3]; break;
         CASE(4,3) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2]; break;
         default: assert(0);
      }
      #undef CASE
   }

   free(data);
   return good;
}

// public domain zlib decode    v0.2  Sean Barrett 2006-11-18
//    simple implementation
//      - all input must be provided in an upfront buffer
//      - all output is written to a single output buffer (can malloc/realloc)
//    performance
//      - fast huffman

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define ZFAST_BITS  9 // accelerate all cases in default tables
#define ZFAST_MASK  ((1 << ZFAST_BITS) - 1)

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
typedef struct
{
   uint16 fast[1 << ZFAST_BITS];
   uint16 firstcode[16];
   int maxcode[17];
   uint16 firstsymbol[16];
   uint8  size[288];
   uint16 value[288]; 
} zhuffman;

stbi_inline static int bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

stbi_inline static int bit_reverse(int v, int bits)
{
   assert(bits <= 16);
   // to bit reverse n bits, reverse 16 and shift
   // e.g. 11 bits, bit reverse and shift away 5
   return bitreverse16(v) >> (16-bits);
}

static int zbuild_huffman(zhuffman *z, uint8 *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   // DEFLATE spec for generating codes
   memset(sizes, 0, sizeof(sizes));
   memset(z->fast, 255, sizeof(z->fast));
   for (i=0; i < num; ++i) 
      ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
      assert(sizes[i] <= (1 << i));
   code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (uint16) code;
      z->firstsymbol[i] = (uint16) k;
      code = (code + sizes[i]);
      if (sizes[i])
         if (code-1 >= (1 << i)) return e("bad codelengths","Corrupt JPEG");
      z->maxcode[i] = code << (16-i); // preshift for inner loop
      code <<= 1;
      k += sizes[i];
   }
   z->maxcode[16] = 0x10000; // sentinel
   for (i=0; i < num; ++i) {
      int s = sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         z->size[c] = (uint8)s;
         z->value[c] = (uint16)i;
         if (s <= ZFAST_BITS) {
            int k = bit_reverse(next_code[s],s);
            while (k < (1 << ZFAST_BITS)) {
               z->fast[k] = (uint16) c;
               k += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}

// zlib-from-memory implementation for PNG reading
//    because PNG allows splitting the zlib stream arbitrarily,
//    and it's annoying structurally to have PNG call ZLIB call PNG,
//    we require PNG read all the IDATs and combine them into a single
//    memory buffer

typedef struct
{
   uint8 *zbuffer, *zbuffer_end;
   int num_bits;
   uint32 code_buffer;

   char *zout;
   char *zout_start;
   char *zout_end;
   int   z_expandable;

   zhuffman z_length, z_distance;
} zbuf;

stbi_inline static int zget8(zbuf *z)
{
   if (z->zbuffer >= z->zbuffer_end) return 0;
   return *z->zbuffer++;
}

static void fill_bits(zbuf *z)
{
   do {
      assert(z->code_buffer < (1U << z->num_bits));
      z->code_buffer |= zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

stbi_inline static unsigned int zreceive(zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;   
}

stbi_inline static int zhuffman_decode(zbuf *a, zhuffman *z)
{
   int b,s,k;
   if (a->num_bits < 16) fill_bits(a);
   b = z->fast[a->code_buffer & ZFAST_MASK];
   if (b < 0xffff) {
      s = z->size[b];
      a->code_buffer >>= s;
      a->num_bits -= s;
      return z->value[b];
   }

   // not resolved by fast table, so compute it the slow way
   // use jpeg approach, which requires MSbits at top
   k = bit_reverse(a->code_buffer, 16);
   for (s=ZFAST_BITS+1; ; ++s)
      if (k < z->maxcode[s])
         break;
   if (s == 16) return -1; // invalid code!
   // code size is s, so:
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   assert(z->size[b] == s);
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

static int expand(zbuf *z, int n)  // need to make room for n bytes
{
   char *q;
   int cur, limit;
   if (!z->z_expandable) return e("output buffer limit","Corrupt PNG");
   cur   = (int) (z->zout     - z->zout_start);
   limit = (int) (z->zout_end - z->zout_start);
   while (cur + n > limit)
      limit *= 2;
   q = (char *) realloc(z->zout_start, limit);
   if (q == NULL) return e("outofmem", "Out of memory");
   z->zout_start = q;
   z->zout       = q + cur;
   z->zout_end   = q + limit;
   return 1;
}

static int length_base[31] = {
   3,4,5,6,7,8,9,10,11,13,
   15,17,19,23,27,31,35,43,51,59,
   67,83,99,115,131,163,195,227,258,0,0 };

static int length_extra[31]= 
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };

static int dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};

static int dist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int parse_huffman_block(zbuf *a)
{
   for(;;) {
      int z = zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return e("bad huffman code","Corrupt PNG"); // error in huffman codes
         if (a->zout >= a->zout_end) if (!expand(a, 1)) return 0;
         *a->zout++ = (char) z;
      } else {
         uint8 *p;
         int len,dist;
         if (z == 256) return 1;
         z -= 257;
         len = length_base[z];
         if (length_extra[z]) len += zreceive(a, length_extra[z]);
         z = zhuffman_decode(a, &a->z_distance);
         if (z < 0) return e("bad huffman code","Corrupt PNG");
         dist = dist_base[z];
         if (dist_extra[z]) dist += zreceive(a, dist_extra[z]);
         if (a->zout - a->zout_start < dist) return e("bad dist","Corrupt PNG");
         if (a->zout + len > a->zout_end) if (!expand(a, len)) return 0;
         p = (uint8 *) (a->zout - dist);
         while (len--)
            *a->zout++ = *p++;
      }
   }
}

static int compute_huffman_codes(zbuf *a)
{
   static uint8 length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   zhuffman z_codelength;
   uint8 lencodes[286+32+137];//padding for maximum single op
   uint8 codelength_sizes[19];
   int i,n;

   int hlit  = zreceive(a,5) + 257;
   int hdist = zreceive(a,5) + 1;
   int hclen = zreceive(a,4) + 4;

   memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (uint8) s;
   }
   if (!zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < hlit + hdist) {
      int c = zhuffman_decode(a, &z_codelength);
      assert(c >= 0 && c < 19);
      if (c < 16)
         lencodes[n++] = (uint8) c;
      else if (c == 16) {
         c = zreceive(a,2)+3;
         memset(lencodes+n, lencodes[n-1], c);
         n += c;
      } else if (c == 17) {
         c = zreceive(a,3)+3;
         memset(lencodes+n, 0, c);
         n += c;
      } else {
         assert(c == 18);
         c = zreceive(a,7)+11;
         memset(lencodes+n, 0, c);
         n += c;
      }
   }
   if (n != hlit+hdist) return e("bad codelengths","Corrupt PNG");
   if (!zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

static int parse_uncompressed_block(zbuf *a)
{
   uint8 header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
      zreceive(a, a->num_bits & 7); // discard
   // drain the bit-packed data into header
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (uint8) (a->code_buffer & 255); // wtf this warns?
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   assert(a->num_bits == 0);
   // now fill header the normal way
   while (k < 4)
      header[k++] = (uint8) zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return e("zlib corrupt","Corrupt PNG");
   if (a->zbuffer + len > a->zbuffer_end) return e("read past buffer","Corrupt PNG");
   if (a->zout + len > a->zout_end)
      if (!expand(a, len)) return 0;
   memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static int parse_zlib_header(zbuf *a)
{
   int cmf   = zget8(a);
   int cm    = cmf & 15;
   /* int cinfo = cmf >> 4; */
   int flg   = zget8(a);
   if ((cmf*256+flg) % 31 != 0) return e("bad zlib header","Corrupt PNG"); // zlib spec
   if (flg & 32) return e("no preset dict","Corrupt PNG"); // preset dictionary not allowed in png
   if (cm != 8) return e("bad compression","Corrupt PNG"); // DEFLATE required for png
   // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
   return 1;
}

// @TODO: should statically initialize these for optimal thread safety
static uint8 default_length[288], default_distance[32];
static void init_defaults(void)
{
   int i;   // use <= to match clearly with spec
   for (i=0; i <= 143; ++i)     default_length[i]   = 8;
   for (   ; i <= 255; ++i)     default_length[i]   = 9;
   for (   ; i <= 279; ++i)     default_length[i]   = 7;
   for (   ; i <= 287; ++i)     default_length[i]   = 8;

   for (i=0; i <=  31; ++i)     default_distance[i] = 5;
}

int stbi_png_partial; // a quick hack to only allow decoding some of a PNG... I should implement real streaming support instead
static int parse_zlib(zbuf *a, int parse_header)
{
   int final, type;
   if (parse_header)
      if (!parse_zlib_header(a)) return 0;
   a->num_bits = 0;
   a->code_buffer = 0;
   do {
      final = zreceive(a,1);
      type = zreceive(a,2);
      if (type == 0) {
         if (!parse_uncompressed_block(a)) return 0;
      } else if (type == 3) {
         return 0;
      } else {
         if (type == 1) {
            // use fixed code lengths
            if (!default_distance[31]) init_defaults();
            if (!zbuild_huffman(&a->z_length  , default_length  , 288)) return 0;
            if (!zbuild_huffman(&a->z_distance, default_distance,  32)) return 0;
         } else {
            if (!compute_huffman_codes(a)) return 0;
         }
         if (!parse_huffman_block(a)) return 0;
      }
      if (stbi_png_partial && a->zout - a->zout_start > 65536)
         break;
   } while (!final);
   return 1;
}

static int do_zlib(zbuf *a, char *obuf, int olen, int exp, int parse_header)
{
   a->zout_start = obuf;
   a->zout       = obuf;
   a->zout_end   = obuf + olen;
   a->z_expandable = exp;

   return parse_zlib(a, parse_header);
}

char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen)
{
   zbuf a;
   char *p = (char *) malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (uint8 *) buffer;
   a.zbuffer_end = (uint8 *) buffer + len;
   if (do_zlib(&a, p, initial_size, 1, 1)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      free(a.zout_start);
      return NULL;
   }
}

char *stbi_zlib_decode_malloc(char const *buffer, int len, int *outlen)
{
   return stbi_zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
}

char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header)
{
   zbuf a;
   char *p = (char *) malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (uint8 *) buffer;
   a.zbuffer_end = (uint8 *) buffer + len;
   if (do_zlib(&a, p, initial_size, 1, parse_header)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      free(a.zout_start);
      return NULL;
   }
}

int stbi_zlib_decode_buffer(char *obuffer, int olen, char const *ibuffer, int ilen)
{
   zbuf a;
   a.zbuffer = (uint8 *) ibuffer;
   a.zbuffer_end = (uint8 *) ibuffer + ilen;
   if (do_zlib(&a, obuffer, olen, 0, 1))
      return (int) (a.zout - a.zout_start);
   else
      return -1;
}

char *stbi_zlib_decode_noheader_malloc(char const *buffer, int len, int *outlen)
{
   zbuf a;
   char *p = (char *) malloc(16384);
   if (p == NULL) return NULL;
   a.zbuffer = (uint8 *) buffer;
   a.zbuffer_end = (uint8 *) buffer+len;
   if (do_zlib(&a, p, 16384, 1, 0)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      free(a.zout_start);
      return NULL;
   }
}

int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen)
{
   zbuf a;
   a.zbuffer = (uint8 *) ibuffer;
   a.zbuffer_end = (uint8 *) ibuffer + ilen;
   if (do_zlib(&a, obuffer, olen, 0, 0))
      return (int) (a.zout - a.zout_start);
   else
      return -1;
}

// public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
//    simple implementation
//      - only 8-bit samples
//      - no CRC checking
//      - allocates lots of intermediate memory
//        - avoids problem of streaming data between subsystems
//        - avoids explicit window management
//    performance
//      - uses stb_zlib, a PD zlib implementation with fast huffman decoding


typedef struct
{
   uint32 length;
   uint32 type;
} chunk;

#define PNG_TYPE(a,b,c,d)  (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))

static chunk get_chunk_header(stbi *s)
{
   chunk c;
   c.length = get32(s);
   c.type   = get32(s);
   return c;
}

static int check_png_header(stbi *s)
{
   static uint8 png_sig[8] = { 137,80,78,71,13,10,26,10 };
   int i;
   for (i=0; i < 8; ++i)
      if (get8u(s) != png_sig[i]) return e("bad png sig","Not a PNG");
   return 1;
}

typedef struct
{
   stbi *s;
   uint8 *idata, *expanded, *out;
} png;


enum {
   F_none=0, F_sub=1, F_up=2, F_avg=3, F_paeth=4,
   F_avg_first, F_paeth_first
};

static uint8 first_row_filter[5] =
{
   F_none, F_sub, F_none, F_avg_first, F_paeth_first
};

static int paeth(int a, int b, int c)
{
   int p = a + b - c;
   int pa = abs(p-a);
   int pb = abs(p-b);
   int pc = abs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

// create the png data from post-deflated data
static int create_png_image_raw(png *a, uint8 *raw, uint32 raw_len, int out_n, uint32 x, uint32 y)
{
   stbi *s = a->s;
   uint32 i,j,stride = x*out_n;
   int k;
   int img_n = s->img_n; // copy it into a local for later
   assert(out_n == s->img_n || out_n == s->img_n+1);
   if (stbi_png_partial) y = 1;
   a->out = (uint8 *) malloc(x * y * out_n);
   if (!a->out) return e("outofmem", "Out of memory");
   if (!stbi_png_partial) {
      if (s->img_x == x && s->img_y == y) {
         if (raw_len != (img_n * x + 1) * y) return e("not enough pixels","Corrupt PNG");
      } else { // interlaced:
         if (raw_len < (img_n * x + 1) * y) return e("not enough pixels","Corrupt PNG");
      }
   }
   for (j=0; j < y; ++j) {
      uint8 *cur = a->out + stride*j;
      uint8 *prior = cur - stride;
      int filter = *raw++;
      if (filter > 4) return e("invalid filter","Corrupt PNG");
      // if first row, use special filter that doesn't sample previous row
      if (j == 0) filter = first_row_filter[filter];
      // handle first pixel explicitly
      for (k=0; k < img_n; ++k) {
         switch (filter) {
            case F_none       : cur[k] = raw[k]; break;
            case F_sub        : cur[k] = raw[k]; break;
            case F_up         : cur[k] = raw[k] + prior[k]; break;
            case F_avg        : cur[k] = raw[k] + (prior[k]>>1); break;
            case F_paeth      : cur[k] = (uint8) (raw[k] + paeth(0,prior[k],0)); break;
            case F_avg_first  : cur[k] = raw[k]; break;
            case F_paeth_first: cur[k] = raw[k]; break;
         }
      }
      if (img_n != out_n) cur[img_n] = 255;
      raw += img_n;
      cur += out_n;
      prior += out_n;
      // this is a little gross, so that we don't switch per-pixel or per-component
      if (img_n == out_n) {
         #define CASE(f) \
             case f:     \
                for (i=x-1; i >= 1; --i, raw+=img_n,cur+=img_n,prior+=img_n) \
                   for (k=0; k < img_n; ++k)
         switch (filter) {
            CASE(F_none)  cur[k] = raw[k]; break;
            CASE(F_sub)   cur[k] = raw[k] + cur[k-img_n]; break;
            CASE(F_up)    cur[k] = raw[k] + prior[k]; break;
            CASE(F_avg)   cur[k] = raw[k] + ((prior[k] + cur[k-img_n])>>1); break;
            CASE(F_paeth)  cur[k] = (uint8) (raw[k] + paeth(cur[k-img_n],prior[k],prior[k-img_n])); break;
            CASE(F_avg_first)    cur[k] = raw[k] + (cur[k-img_n] >> 1); break;
            CASE(F_paeth_first)  cur[k] = (uint8) (raw[k] + paeth(cur[k-img_n],0,0)); break;
         }
         #undef CASE
      } else {
         assert(img_n+1 == out_n);
         #define CASE(f) \
             case f:     \
                for (i=x-1; i >= 1; --i, cur[img_n]=255,raw+=img_n,cur+=out_n,prior+=out_n) \
                   for (k=0; k < img_n; ++k)
         switch (filter) {
            CASE(F_none)  cur[k] = raw[k]; break;
            CASE(F_sub)   cur[k] = raw[k] + cur[k-out_n]; break;
            CASE(F_up)    cur[k] = raw[k] + prior[k]; break;
            CASE(F_avg)   cur[k] = raw[k] + ((prior[k] + cur[k-out_n])>>1); break;
            CASE(F_paeth)  cur[k] = (uint8) (raw[k] + paeth(cur[k-out_n],prior[k],prior[k-out_n])); break;
            CASE(F_avg_first)    cur[k] = raw[k] + (cur[k-out_n] >> 1); break;
            CASE(F_paeth_first)  cur[k] = (uint8) (raw[k] + paeth(cur[k-out_n],0,0)); break;
         }
         #undef CASE
      }
   }
   return 1;
}

static int create_png_image(png *a, uint8 *raw, uint32 raw_len, int out_n, int interlaced)
{
   uint8 *final;
   int p;
   int save;
   if (!interlaced)
      return create_png_image_raw(a, raw, raw_len, out_n, a->s->img_x, a->s->img_y);
   save = stbi_png_partial;
   stbi_png_partial = 0;

   // de-interlacing
   final = (uint8 *) malloc(a->s->img_x * a->s->img_y * out_n);
   for (p=0; p < 7; ++p) {
      int xorig[] = { 0,4,0,2,0,1,0 };
      int yorig[] = { 0,0,4,0,2,0,1 };
      int xspc[]  = { 8,8,4,4,2,2,1 };
      int yspc[]  = { 8,8,8,4,4,2,2 };
      int i,j,x,y;
      // pass1_x[4] = 0, pass1_x[5] = 1, pass1_x[12] = 1
      x = (a->s->img_x - xorig[p] + xspc[p]-1) / xspc[p];
      y = (a->s->img_y - yorig[p] + yspc[p]-1) / yspc[p];
      if (x && y) {
         if (!create_png_image_raw(a, raw, raw_len, out_n, x, y)) {
            free(final);
            return 0;
         }
         for (j=0; j < y; ++j)
            for (i=0; i < x; ++i)
               memcpy(final + (j*yspc[p]+yorig[p])*a->s->img_x*out_n + (i*xspc[p]+xorig[p])*out_n,
                      a->out + (j*x+i)*out_n, out_n);
         free(a->out);
         raw += (x*out_n+1)*y;
         raw_len -= (x*out_n+1)*y;
      }
   }
   a->out = final;

   stbi_png_partial = save;
   return 1;
}

static int compute_transparency(png *z, uint8 tc[3], int out_n)
{
   stbi *s = z->s;
   uint32 i, pixel_count = s->img_x * s->img_y;
   uint8 *p = z->out;

   // compute color-based transparency, assuming we've
   // already got 255 as the alpha value in the output
   assert(out_n == 2 || out_n == 4);

   if (out_n == 2) {
      for (i=0; i < pixel_count; ++i) {
         p[1] = (p[0] == tc[0] ? 0 : 255);
         p += 2;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
            p[3] = 0;
         p += 4;
      }
   }
   return 1;
}

static int expand_palette(png *a, uint8 *palette, int len, int pal_img_n)
{
   uint32 i, pixel_count = a->s->img_x * a->s->img_y;
   uint8 *p, *temp_out, *orig = a->out;

   p = (uint8 *) malloc(pixel_count * pal_img_n);
   if (p == NULL) return e("outofmem", "Out of memory");

   // between here and free(out) below, exitting would leak
   temp_out = p;

   if (pal_img_n == 3) {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p += 3;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p[3] = palette[n+3];
         p += 4;
      }
   }
   free(a->out);
   a->out = temp_out;

   STBI_NOTUSED(len);

   return 1;
}

static int stbi_unpremultiply_on_load = 0;
static int stbi_de_iphone_flag = 0;

void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply)
{
   stbi_unpremultiply_on_load = flag_true_if_should_unpremultiply;
}
void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert)
{
   stbi_de_iphone_flag = flag_true_if_should_convert;
}

static void stbi_de_iphone(png *z)
{
   stbi *s = z->s;
   uint32 i, pixel_count = s->img_x * s->img_y;
   uint8 *p = z->out;

   if (s->img_out_n == 3) {  // convert bgr to rgb
      for (i=0; i < pixel_count; ++i) {
         uint8 t = p[0];
         p[0] = p[2];
         p[2] = t;
         p += 3;
      }
   } else {
      assert(s->img_out_n == 4);
      if (stbi_unpremultiply_on_load) {
         // convert bgr to rgb and unpremultiply
         for (i=0; i < pixel_count; ++i) {
            uint8 a = p[3];
            uint8 t = p[0];
            if (a) {
               p[0] = p[2] * 255 / a;
               p[1] = p[1] * 255 / a;
               p[2] =  t   * 255 / a;
            } else {
               p[0] = p[2];
               p[2] = t;
            } 
            p += 4;
         }
      } else {
         // convert bgr to rgb
         for (i=0; i < pixel_count; ++i) {
            uint8 t = p[0];
            p[0] = p[2];
            p[2] = t;
            p += 4;
         }
      }
   }
}

static int parse_png_file(png *z, int scan, int req_comp)
{
   uint8 palette[1024], pal_img_n=0;
   uint8 has_trans=0, tc[3];
   uint32 ioff=0, idata_limit=0, i, pal_len=0;
   int first=1,k,interlace=0, iphone=0;
   stbi *s = z->s;

   z->expanded = NULL;
   z->idata = NULL;
   z->out = NULL;

   if (!check_png_header(s)) return 0;

   if (scan == SCAN_type) return 1;

   for (;;) {
      chunk c = get_chunk_header(s);
      switch (c.type) {
         case PNG_TYPE('C','g','B','I'):
            iphone = stbi_de_iphone_flag;
            skip(s, c.length);
            break;
         case PNG_TYPE('I','H','D','R'): {
            int depth,color,comp,filter;
            if (!first) return e("multiple IHDR","Corrupt PNG");
            first = 0;
            if (c.length != 13) return e("bad IHDR len","Corrupt PNG");
            s->img_x = get32(s); if (s->img_x > (1 << 24)) return e("too large","Very large image (corrupt?)");
            s->img_y = get32(s); if (s->img_y > (1 << 24)) return e("too large","Very large image (corrupt?)");
            depth = get8(s);  if (depth != 8)        return e("8bit only","PNG not supported: 8-bit only");
            color = get8(s);  if (color > 6)         return e("bad ctype","Corrupt PNG");
            if (color == 3) pal_img_n = 3; else if (color & 1) return e("bad ctype","Corrupt PNG");
            comp  = get8(s);  if (comp) return e("bad comp method","Corrupt PNG");
            filter= get8(s);  if (filter) return e("bad filter method","Corrupt PNG");
            interlace = get8(s); if (interlace>1) return e("bad interlace method","Corrupt PNG");
            if (!s->img_x || !s->img_y) return e("0-pixel image","Corrupt PNG");
            if (!pal_img_n) {
               s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
               if ((1 << 30) / s->img_x / s->img_n < s->img_y) return e("too large", "Image too large to decode");
               if (scan == SCAN_header) return 1;
            } else {
               // if paletted, then pal_n is our final components, and
               // img_n is # components to decompress/filter.
               s->img_n = 1;
               if ((1 << 30) / s->img_x / 4 < s->img_y) return e("too large","Corrupt PNG");
               // if SCAN_header, have to scan to see if we have a tRNS
            }
            break;
         }

         case PNG_TYPE('P','L','T','E'):  {
            if (first) return e("first not IHDR", "Corrupt PNG");
            if (c.length > 256*3) return e("invalid PLTE","Corrupt PNG");
            pal_len = c.length / 3;
            if (pal_len * 3 != c.length) return e("invalid PLTE","Corrupt PNG");
            for (i=0; i < pal_len; ++i) {
               palette[i*4+0] = get8u(s);
               palette[i*4+1] = get8u(s);
               palette[i*4+2] = get8u(s);
               palette[i*4+3] = 255;
            }
            break;
         }

         case PNG_TYPE('t','R','N','S'): {
            if (first) return e("first not IHDR", "Corrupt PNG");
            if (z->idata) return e("tRNS after IDAT","Corrupt PNG");
            if (pal_img_n) {
               if (scan == SCAN_header) { s->img_n = 4; return 1; }
               if (pal_len == 0) return e("tRNS before PLTE","Corrupt PNG");
               if (c.length > pal_len) return e("bad tRNS len","Corrupt PNG");
               pal_img_n = 4;
               for (i=0; i < c.length; ++i)
                  palette[i*4+3] = get8u(s);
            } else {
               if (!(s->img_n & 1)) return e("tRNS with alpha","Corrupt PNG");
               if (c.length != (uint32) s->img_n*2) return e("bad tRNS len","Corrupt PNG");
               has_trans = 1;
               for (k=0; k < s->img_n; ++k)
                  tc[k] = (uint8) get16(s); // non 8-bit images will be larger
            }
            break;
         }

         case PNG_TYPE('I','D','A','T'): {
            if (first) return e("first not IHDR", "Corrupt PNG");
            if (pal_img_n && !pal_len) return e("no PLTE","Corrupt PNG");
            if (scan == SCAN_header) { s->img_n = pal_img_n; return 1; }
            if (ioff + c.length > idata_limit) {
               uint8 *p;
               if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
               while (ioff + c.length > idata_limit)
                  idata_limit *= 2;
               p = (uint8 *) realloc(z->idata, idata_limit); if (p == NULL) return e("outofmem", "Out of memory");
               z->idata = p;
            }
            if (!getn(s, z->idata+ioff,c.length)) return e("outofdata","Corrupt PNG");
            ioff += c.length;
            break;
         }

         case PNG_TYPE('I','E','N','D'): {
            uint32 raw_len;
            if (first) return e("first not IHDR", "Corrupt PNG");
            if (scan != SCAN_load) return 1;
            if (z->idata == NULL) return e("no IDAT","Corrupt PNG");
            z->expanded = (uint8 *) stbi_zlib_decode_malloc_guesssize_headerflag((char *) z->idata, ioff, 16384, (int *) &raw_len, !iphone);
            if (z->expanded == NULL) return 0; // zlib should set error
            free(z->idata); z->idata = NULL;
            if ((req_comp == s->img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
               s->img_out_n = s->img_n+1;
            else
               s->img_out_n = s->img_n;
            if (!create_png_image(z, z->expanded, raw_len, s->img_out_n, interlace)) return 0;
            if (has_trans)
               if (!compute_transparency(z, tc, s->img_out_n)) return 0;
            if (iphone && s->img_out_n > 2)
               stbi_de_iphone(z);
            if (pal_img_n) {
               // pal_img_n == 3 or 4
               s->img_n = pal_img_n; // record the actual colors we had
               s->img_out_n = pal_img_n;
               if (req_comp >= 3) s->img_out_n = req_comp;
               if (!expand_palette(z, palette, pal_len, s->img_out_n))
                  return 0;
            }
            free(z->expanded); z->expanded = NULL;
            return 1;
         }

         default:
            // if critical, fail
            if (first) return e("first not IHDR", "Corrupt PNG");
            if ((c.type & (1 << 29)) == 0) {
               #ifndef STBI_NO_FAILURE_STRINGS
               // not threadsafe
               static char invalid_chunk[] = "XXXX chunk not known";
               invalid_chunk[0] = (uint8) (c.type >> 24);
               invalid_chunk[1] = (uint8) (c.type >> 16);
               invalid_chunk[2] = (uint8) (c.type >>  8);
               invalid_chunk[3] = (uint8) (c.type >>  0);
               #endif
               return e(invalid_chunk, "PNG not supported: unknown chunk type");
            }
            skip(s, c.length);
            break;
      }
      // end of chunk, read and skip CRC
      get32(s);
   }
}

static unsigned char *do_png(png *p, int *x, int *y, int *n, int req_comp)
{
   unsigned char *result=NULL;
   if (req_comp < 0 || req_comp > 4) return epuc("bad req_comp", "Internal error");
   if (parse_png_file(p, SCAN_load, req_comp)) {
      result = p->out;
      p->out = NULL;
      if (req_comp && req_comp != p->s->img_out_n) {
         result = convert_format(result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
         p->s->img_out_n = req_comp;
         if (result == NULL) return result;
      }
      *x = p->s->img_x;
      *y = p->s->img_y;
      if (n) *n = p->s->img_n;
   }
   free(p->out);      p->out      = NULL;
   free(p->expanded); p->expanded = NULL;
   free(p->idata);    p->idata    = NULL;

   return result;
}

static unsigned char *stbi_png_load(stbi *s, int *x, int *y, int *comp, int req_comp)
{
   png p;
   p.s = s;
   return do_png(&p, x,y,comp,req_comp);
}

static int stbi_png_test(stbi *s)
{
   int r;
   r = check_png_header(s);
   stbi_rewind(s);
   return r;
}

static int stbi_png_info_raw(png *p, int *x, int *y, int *comp)
{
   if (!parse_png_file(p, SCAN_header, 0)) {
      stbi_rewind( p->s );
      return 0;
   }
   if (x) *x = p->s->img_x;
   if (y) *y = p->s->img_y;
   if (comp) *comp = p->s->img_n;
   return 1;
}

static int      stbi_png_info(stbi *s, int *x, int *y, int *comp)
{
   png p;
   p.s = s;
   return stbi_png_info_raw(&p, x, y, comp);
}

static int stbi_info_main(stbi *s, int *x, int *y, int *comp)
{
   if (stbi_png_info(s, x, y, comp))
       return 1;
   return e("unknown image type", "Image not of any known type, or corrupt");
}

#ifndef STBI_NO_STDIO
int stbi_info(char const *filename, int *x, int *y, int *comp)
{
    FILE *f = fopen(filename, "rb");
    int result;
    if (!f) return e("can't fopen", "Unable to open file");
    result = stbi_info_from_file(f, x, y, comp);
    fclose(f);
    return result;
}

int stbi_info_from_file(FILE *f, int *x, int *y, int *comp)
{
   int r;
   stbi s;
   long pos = ftell(f);
   start_file(&s, f);
   r = stbi_info_main(&s,x,y,comp);
   fseek(f,pos,SEEK_SET);
   return r;
}
#endif // !STBI_NO_STDIO

int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp)
{
   stbi s;
   start_mem(&s,buffer,len);
   return stbi_info_main(&s,x,y,comp);
}

int stbi_info_from_callbacks(stbi_io_callbacks const *c, void *user, int *x, int *y, int *comp)
{
   stbi s;
   start_callbacks(&s, (stbi_io_callbacks *) c, user);
   return stbi_info_main(&s,x,y,comp);
}

#endif // STBI_HEADER_FILE_ONLY