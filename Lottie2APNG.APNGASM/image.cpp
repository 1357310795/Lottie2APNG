/* Image library
 *
 * Copyright (c) 2016 Max Stepin
 * maxst at users.sourceforge.net
 *
 * zlib license
 * ------------
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "png.h"
#include "pngstruct.h"
#include "zlib.h"
#include <sstream>
#ifdef FEATURE_7ZIP
#include "7z.h"
#endif
#ifdef FEATURE_ZOPFLI
#include "zopfli.h"
#endif

void png_custom_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  png_size_t check;
  std::stringstream * ss;
  ss = (std::stringstream*)(png_ptr->io_ptr);

  if (png_ptr == NULL)
    return;
     
  int lastpos = ss->tellg();
  ss->read((char*)data, length);
  int curpos = ss->tellg();

  if (curpos - lastpos != length)
    png_error(png_ptr, "Read Error");
}

int load_png_from_stream(std::stringstream * ss, Image * image)
{
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop  info_ptr = png_create_info_struct(png_ptr);

  if (!png_ptr || !info_ptr)
    return 1;

  if (ss->bad())
  {
    printf("Error: can't open stream\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    printf("Error: setjmp failed\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 1;
  }
  png_set_read_fn(png_ptr, (void*)ss, png_custom_read_data);
  png_read_info(png_ptr, info_ptr);
  unsigned int depth = png_get_bit_depth(png_ptr, info_ptr);
  if (depth < 8)
  {
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
      png_set_packing(png_ptr);
    else
      png_set_expand(png_ptr);
  }
  else
  if (depth > 8)
  {
    png_set_expand(png_ptr);
    png_set_strip_16(png_ptr);
  }
  (void)png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);
  unsigned int w = png_get_image_width(png_ptr, info_ptr);
  unsigned int h = png_get_image_height(png_ptr, info_ptr);
  unsigned int bpp = png_get_channels(png_ptr, info_ptr);
  unsigned int type = png_get_color_type(png_ptr, info_ptr);
  image->init(w, h, bpp, type);

  png_colorp palette;
  png_color_16p trans_color;
  png_bytep trans_alpha;

  if (png_get_PLTE(png_ptr, info_ptr, &palette, &image->ps))
    memcpy(image->pl, palette, image->ps * 3);

  if (png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &image->ts, &trans_color) && image->ts > 0)
  {
    if (type == PNG_COLOR_TYPE_GRAY)
    {
      image->tr[0] = 0;
      image->tr[1] = trans_color->gray & 0xFF;
      image->ts = 2;
    }
    else
    if (type == PNG_COLOR_TYPE_RGB)
    {
      image->tr[0] = 0;
      image->tr[1] = trans_color->red & 0xFF;
      image->tr[2] = 0;
      image->tr[3] = trans_color->green & 0xFF;
      image->tr[4] = 0;
      image->tr[5] = trans_color->blue & 0xFF;
      image->ts = 6;
    }
    else
    if (type == PNG_COLOR_TYPE_PALETTE)
      memcpy(image->tr, trans_alpha, image->ts);
  }

  png_read_image(png_ptr, image->rows);
  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  return 0;
}

int load_png(char * szName, Image * image)
{
  FILE * f;
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop  info_ptr = png_create_info_struct(png_ptr);

  if (!png_ptr || !info_ptr)
    return 1;

  if ((f = fopen(szName, "rb")) == 0)
  {
    printf("Error: can't open '%s'\n", szName);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    printf("Error: can't load '%s'\n", szName);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(f);
    return 1;
  }

  png_init_io(png_ptr, f);
  png_read_info(png_ptr, info_ptr);
  unsigned int depth = png_get_bit_depth(png_ptr, info_ptr);
  if (depth < 8)
  {
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
      png_set_packing(png_ptr);
    else
      png_set_expand(png_ptr);
  }
  else
  if (depth > 8)
  {
    png_set_expand(png_ptr);
    png_set_strip_16(png_ptr);
  }
  (void)png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);
  unsigned int w = png_get_image_width(png_ptr, info_ptr);
  unsigned int h = png_get_image_height(png_ptr, info_ptr);
  unsigned int bpp = png_get_channels(png_ptr, info_ptr);
  unsigned int type = png_get_color_type(png_ptr, info_ptr);
  image->init(w, h, bpp, type);

  png_colorp palette;
  png_color_16p trans_color;
  png_bytep trans_alpha;

  if (png_get_PLTE(png_ptr, info_ptr, &palette, &image->ps))
    memcpy(image->pl, palette, image->ps * 3);

  if (png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &image->ts, &trans_color) && image->ts > 0)
  {
    if (type == PNG_COLOR_TYPE_GRAY)
    {
      image->tr[0] = 0;
      image->tr[1] = trans_color->gray & 0xFF;
      image->ts = 2;
    }
    else
    if (type == PNG_COLOR_TYPE_RGB)
    {
      image->tr[0] = 0;
      image->tr[1] = trans_color->red & 0xFF;
      image->tr[2] = 0;
      image->tr[3] = trans_color->green & 0xFF;
      image->tr[4] = 0;
      image->tr[5] = trans_color->blue & 0xFF;
      image->ts = 6;
    }
    else
    if (type == PNG_COLOR_TYPE_PALETTE)
      memcpy(image->tr, trans_alpha, image->ts);
  }

  png_read_image(png_ptr, image->rows);
  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(f);
  return 0;
}

int load_tga(char * szName, Image * image)
{
  FILE * f;
  unsigned int i, j, k, n;
  unsigned int w, h;
  unsigned int compressed, top_bottom;
  unsigned char c;
  unsigned char col[4];
  unsigned char header[18];

  if ((f = fopen(szName, "rb")) == 0)
  {
    printf("Error: can't open '%s'\n", szName);
    return 1;
  }

  if (fread(&header, 1, 18, f) != 18)
    goto fail;

  w = header[12] + header[13]*256;
  h = header[14] + header[15]*256;
  compressed = header[2] & 8;
  top_bottom = header[17] & 0x20;

  if ((header[2] & 7) == 1 && header[16] == 8 && header[1] == 1 && header[7] == 24)
    image->init(w, h, 1, 3);
  else
  if ((header[2] & 7) == 3 && header[16] == 8)
    image->init(w, h, 1, 0);
  else
  if ((header[2] & 7) == 2 && header[16] == 24)
    image->init(w, h, 3, 2);
  else
  if ((header[2] & 7) == 2 && header[16] == 32)
    image->init(w, h, 4, 6);
  else
    goto fail;

  if (header[0] != 0)
    fseek(f, header[0], SEEK_CUR);

  if (header[1] == 1)
  {
    unsigned int start = header[3] + header[4]*256;
    unsigned int size  = header[5] + header[6]*256;
    for (i=start; i<start+size && i<256; i++)
    {
      if (fread(&col, 1, 3, f) != 3)
        goto fail;
      image->pl[i].r = col[2];
      image->pl[i].g = col[1];
      image->pl[i].b = col[0];
    }
    image->ps = i;
    if (start+size > 256)
      fseek(f, (start+size-256)*3, SEEK_CUR);
  }

  for (j=0; j<h; j++)
  {
    unsigned char * row = image->rows[(top_bottom) ? j : h-1-j];
    if (compressed == 0)
    {
      if (image->bpp >= 3)
      {
        for (i=0; i<w; i++)
        {
          if (fread(&col, 1, image->bpp, f) != image->bpp)
            goto fail;
          *row++ = col[2];
          *row++ = col[1];
          *row++ = col[0];
          if (image->bpp == 4)
            *row++ = col[3];
        }
      }
      else
      {
        if (fread(row, 1, w, f) != w)
          goto fail;
      }
    }
    else
    {
      i = 0;
      while (i < w)
      {
        if (fread(&c, 1, 1, f) != 1)
          goto fail;
        n = (c & 0x7F)+1;

        if ((c & 0x80) != 0)
        {
          if (image->bpp >= 3)
          {
            if (fread(&col, 1, image->bpp, f) != image->bpp)
              goto fail;
            for (k=0; k<n; k++)
            {
              *row++ = col[2];
              *row++ = col[1];
              *row++ = col[0];
              if (image->bpp == 4)
                *row++ = col[3];
            }
          }
          else
          {
            if (fread(&col, 1, 1, f) != 1)
              goto fail;
            memset(row, col[0], n);
            row += n;
          }
        }
        else
        {
          if (image->bpp >= 3)
          {
            for (k=0; k<n; k++)
            {
              if (fread(&col, 1, image->bpp, f) != image->bpp)
                goto fail;
              *row++ = col[2];
              *row++ = col[1];
              *row++ = col[0];
              if (image->bpp == 4)
                *row++ = col[3];
            }
          }
          else
          {
            if (fread(row, 1, n, f) != n)
              goto fail;
            row += n;
          }
        }
        i+=n;
      }
    }
  }
  fclose(f);
  return 0;
fail:
  printf("Error: can't load '%s'\n", szName);
  fclose(f);
  return 1;
}

int load_image(char * szName, Image * image)
{
  FILE * f;

  if ((f = fopen(szName, "rb")) != 0)
  {
    unsigned int sign;
    size_t res = fread(&sign, sizeof(sign), 1, f);
    fclose(f);

    if (res == 1)
    {
      if (sign == 0x474E5089)
        return load_png(szName, image);
      else
        return load_tga(szName, image);
    }
  }

  printf("Error: can't load '%s'\n", szName);
  return 1;
}

unsigned char find_common_coltype(std::vector<Image*>& img)
{
  unsigned char coltype = img[0]->type;

  for (size_t i=1; i<img.size(); i++)
  {
    if (img[0]->ps != img[i]->ps || memcmp(img[0]->pl, img[i]->pl, img[0]->ps*3) != 0)
      coltype = 6;
    else
    if (img[0]->ts != img[i]->ts || memcmp(img[0]->tr, img[i]->tr, img[0]->ts) != 0)
      coltype = 6;
    else
    if (img[i]->type != 3)
    {
      if (coltype != 3)
        coltype |= img[i]->type;
      else
        coltype = 6;
    }
    else
      if (coltype != 3)
        coltype = 6;
  }
  return coltype;
}

void up0to6(Image * image)
{
  image->type = 6;
  image->bpp = 4;
  unsigned int x, y;
  unsigned char g, a;
  unsigned char * sp, * dp;
  unsigned int rowbytes = image->w *image->bpp;
  unsigned char * dst = new unsigned char[image->h * rowbytes];

  for (y=0; y<image->h; y++)
  {
    sp = image->rows[y];
    image->rows[y] = (y == 0) ? dst : image->rows[y-1] + rowbytes;
    dp = image->rows[y];
    for (x=0; x<image->w; x++)
    {
      g = *sp++;
      a = (image->ts > 0 && image->tr[1] == g) ? 0 : 255;
      *dp++ = g;
      *dp++ = g;
      *dp++ = g;
      *dp++ = a;
    }
  }
  delete[] image->p;
  image->p = dst;
}

void up2to6(Image * image)
{
  image->type = 6;
  image->bpp = 4;
  unsigned int x, y;
  unsigned char r, g, b, a;
  unsigned char * sp, * dp;
  unsigned int rowbytes = image->w *image->bpp;
  unsigned char * dst = new unsigned char[image->h * rowbytes];

  for (y=0; y<image->h; y++)
  {
    sp = image->rows[y];
    image->rows[y] = (y == 0) ? dst : image->rows[y-1] + rowbytes;
    dp = image->rows[y];
    for (x=0; x<image->w; x++)
    {
      r = *sp++;
      g = *sp++;
      b = *sp++;
      a = (image->ts > 0 && image->tr[1] == r && image->tr[3] == g && image->tr[5] == b) ? 0 : 255;
      *dp++ = r;
      *dp++ = g;
      *dp++ = b;
      *dp++ = a;
    }
  }
  delete[] image->p;
  image->p = dst;
}

void up3to6(Image * image)
{
  image->type = 6;
  image->bpp = 4;
  unsigned int x, y;
  unsigned char * sp, * dp;
  unsigned int rowbytes = image->w *image->bpp;
  unsigned char * dst = new unsigned char[image->h * rowbytes];

  for (y=0; y<image->h; y++)
  {
    sp = image->rows[y];
    image->rows[y] = (y == 0) ? dst : image->rows[y-1] + rowbytes;
    dp = image->rows[y];
    for (x=0; x<image->w; x++)
    {
      *dp++ = image->pl[*sp].r;
      *dp++ = image->pl[*sp].g;
      *dp++ = image->pl[*sp].b;
      *dp++ = image->tr[*sp++];
    }
  }
  delete[] image->p;
  image->p = dst;
}

void up4to6(Image * image)
{
  image->type = 6;
  image->bpp = 4;
  unsigned int x, y;
  unsigned char * sp, * dp;
  unsigned int rowbytes = image->w *image->bpp;
  unsigned char * dst = new unsigned char[image->h * rowbytes];

  for (y=0; y<image->h; y++)
  {
    sp = image->rows[y];
    image->rows[y] = (y == 0) ? dst : image->rows[y-1] + rowbytes;
    dp = image->rows[y];
    for (x=0; x<image->w; x++)
    {
      *dp++ = *sp;
      *dp++ = *sp;
      *dp++ = *sp++;
      *dp++ = *sp++;
    }
  }
  delete[] image->p;
  image->p = dst;
}

void up0to4(Image * image)
{
  image->type = 4;
  image->bpp = 2;
  unsigned int x, y;
  unsigned char * sp, * dp;
  unsigned int rowbytes = image->w *image->bpp;
  unsigned char * dst = new unsigned char[image->h * rowbytes];

  for (y=0; y<image->h; y++)
  {
    sp = image->rows[y];
    image->rows[y] = (y == 0) ? dst : image->rows[y-1] + rowbytes;
    dp = image->rows[y];
    for (x=0; x<image->w; x++)
    {
      *dp++ = *sp++;
      *dp++ = 255;
    }
  }
  delete[] image->p;
  image->p = dst;
}

void up0to2(Image * image)
{
  image->type = 2;
  image->bpp = 3;
  unsigned int x, y;
  unsigned char * sp, * dp;
  unsigned int rowbytes = image->w *image->bpp;
  unsigned char * dst = new unsigned char[image->h * rowbytes];

  for (y=0; y<image->h; y++)
  {
    sp = image->rows[y];
    image->rows[y] = (y == 0) ? dst : image->rows[y-1] + rowbytes;
    dp = image->rows[y];
    for (x=0; x<image->w; x++)
    {
      *dp++ = *sp;
      *dp++ = *sp;
      *dp++ = *sp++;
    }
  }
  delete[] image->p;
  image->p = dst;
}

void optim_upconvert(Image * image, unsigned char coltype)
{
  if (image->type == 0 && coltype == 6)
    up0to6(image);
  else
  if (image->type == 2 && coltype == 6)
    up2to6(image);
  else
  if (image->type == 3 && coltype == 6)
    up3to6(image);
  else
  if (image->type == 4 && coltype == 6)
    up4to6(image);
  else
  if (image->type == 0 && coltype == 4)
    up0to4(image);
  else
  if (image->type == 0 && coltype == 2)
    up0to2(image);
}

void optim_dirty_transp(Image * image)
{
  unsigned int x, y;
  if (image->type == 6)
  {
    for (y=0; y<image->h; y++)
    {
      unsigned char * sp = image->rows[y];
      for (x=0; x<image->w; x++, sp+=4)
      {
        if (sp[3] == 0)
           sp[0] = sp[1] = sp[2] = 0;
      }
    }
  }
  else
  if (image->type == 4)
  {
    for (y=0; y<image->h; y++)
    {
      unsigned char * sp = image->rows[y];
      for (x=0; x<image->w; x++, sp+=2)
      {
        if (sp[1] == 0)
          sp[0] = 0;
      }
    }
  }
}

int different(Image * image1, Image * image2)
{
  return memcmp(image1->p, image2->p, image1->w * image1->h * image1->bpp);
}

void optim_duplicates(std::vector<Image*>& img, unsigned int first)
{
  unsigned int i = first;

  while (++i < img.size())
  {
    if (different(img[i-1], img[i]))
      continue;

    i--;
    unsigned int num = img[i]->delay_num;
    unsigned int den = img[i]->delay_den;
      
    img[i]->free();
    img.erase(img.begin() + i);

    if (img[i]->delay_den == den)
      img[i]->delay_num += num;
    else
    {
      img[i]->delay_num = num = num*img[i]->delay_den + den*img[i]->delay_num;
      img[i]->delay_den = den = den*img[i]->delay_den;
      while (num && den)
      {
        if (num > den)
          num = num % den;
        else
          den = den % num;
      }
      num += den;
      img[i]->delay_num /= num;
      img[i]->delay_den /= num;
    }
  }
}

typedef struct { unsigned int num; unsigned char r, g, b, a; } COLORS;

int cmp_colors(const void *arg1, const void *arg2)
{
  if ( ((COLORS*)arg1)->a != ((COLORS*)arg2)->a )
    return (int)(((COLORS*)arg1)->a) - (int)(((COLORS*)arg2)->a);

  if ( ((COLORS*)arg1)->num != ((COLORS*)arg2)->num )
    return (int)(((COLORS*)arg2)->num) - (int)(((COLORS*)arg1)->num);

  if ( ((COLORS*)arg1)->r != ((COLORS*)arg2)->r )
    return (int)(((COLORS*)arg1)->r) - (int)(((COLORS*)arg2)->r);

  if ( ((COLORS*)arg1)->g != ((COLORS*)arg2)->g )
    return (int)(((COLORS*)arg1)->g) - (int)(((COLORS*)arg2)->g);

  return (int)(((COLORS*)arg1)->b) - (int)(((COLORS*)arg2)->b);
}

void down6(std::vector<Image*>& img)
{
  unsigned int i, k, x, y;
  unsigned char * sp;
  unsigned char * dp;
  unsigned char r, g, b, a;
  int simple_transp = 1;
  int full_transp = 0;
  int grayscale = 1;
  unsigned char cube[4096];
  unsigned char gray[256];
  COLORS col[256];
  unsigned int colors = 0;
  Image * image = img[0];

  memset(&cube, 0, sizeof(cube));
  memset(&gray, 0, sizeof(gray));

  for (i=0; i<256; i++)
  {
    col[i].num = 0;
    col[i].r = col[i].g = col[i].b = i;
    col[i].a = image->tr[i] = 255;
  }

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
      {
        r = *sp++;
        g = *sp++;
        b = *sp++;
        a = *sp++;

        if (a != 0)
        {
          if (a != 255)
            simple_transp = 0;
          else
            if (((r | g | b) & 15) == 0)
              cube[(r<<4) + g + (b>>4)] = 1;

          if (r != g || g != b)
            grayscale = 0;
          else
            gray[r] = 1;
        }
        else
          full_transp = 1;

        if (colors <= 256)
        {
          int found = 0;
          for (k=0; k<colors; k++)
          if (col[k].r == r && col[k].g == g && col[k].b == b && col[k].a == a)
          {
            found = 1;
            col[k].num++;
            break;
          }
          if (found == 0)
          {
            if (colors < 256)
            {
              col[colors].num++;
              col[colors].r = r;
              col[colors].g = g;
              col[colors].b = b;
              col[colors].a = a;
            }
            colors++;
          }
        }
      }
    }
  }

  if (colors <= 256)
  {
    if (grayscale && simple_transp && colors > 128) /* 6 -> 0 */
    {
      image->type = 0;
      image->bpp = 1;
      unsigned char t = 0;

      for (i=0; i<256; i++)
      if (gray[i] == 0)
      {
        image->tr[0] = 0;
        image->tr[1] = t = i;
        image->ts = 2;
        break;
      }

      for (i=0; i<img.size(); i++)
      {
        for (y=0; y<image->h; y++)
        {
          sp = dp = img[i]->rows[y];
          for (x=0; x<image->w; x++, sp+=4)
          {
            *dp++ = (sp[3] == 0) ? t : sp[0];
          }
        }
      }
    }
    else /* 6 -> 3 */
    {
      image->type = 3;
      image->bpp = 1;

      if (full_transp == 0 && colors < 256)
        col[colors++].a = 0;

      qsort(&col[0], colors, sizeof(COLORS), cmp_colors);

      for (i=0; i<img.size(); i++)
      {
        for (y=0; y<image->h; y++)
        {
          sp = dp = img[i]->rows[y];
          for (x=0; x<image->w; x++)
          {
            r = *sp++;
            g = *sp++;
            b = *sp++;
            a = *sp++;
            for (k=0; k<colors; k++)
              if (col[k].r == r && col[k].g == g && col[k].b == b && col[k].a == a)
                break;
            *dp++ = k;
          }
        }
      }

      image->ps = colors;
      for (i=0; i<colors; i++)
      {
        image->pl[i].r = col[i].r;
        image->pl[i].g = col[i].g;
        image->pl[i].b = col[i].b;
        image->tr[i]   = col[i].a;
        if (image->tr[i] != 255) 
          image->ts = i+1;
      }
    }
  }
  else
  if (grayscale)     /* 6 -> 4 */
  {
    image->type = 4;
    image->bpp = 2;
    for (i=0; i<img.size(); i++)
    {
      for (y=0; y<image->h; y++)
      {
        sp = dp = img[i]->rows[y];
        for (x=0; x<image->w; x++, sp+=4)
        {
          *dp++ = sp[2];
          *dp++ = sp[3];
        }
      }
    }
  }
  else
  if (simple_transp)  /* 6 -> 2 */
  {
    for (i=0; i<4096; i++)
    if (cube[i] == 0)
    {
      image->tr[0] = 0;
      image->tr[1] = (i>>4)&0xF0;
      image->tr[2] = 0;
      image->tr[3] = i&0xF0;
      image->tr[4] = 0;
      image->tr[5] = (i<<4)&0xF0;
      image->ts = 6;
      break;
    }
    if (image->ts != 0)
    {
      image->type = 2;
      image->bpp = 3;
      for (i=0; i<img.size(); i++)
      {
        for (y=0; y<image->h; y++)
        {
          sp = dp = img[i]->rows[y];
          for (x=0; x<image->w; x++)
          {
            r = *sp++;
            g = *sp++;
            b = *sp++;
            a = *sp++;
            if (a == 0)
            {
              *dp++ = image->tr[1];
              *dp++ = image->tr[3];
              *dp++ = image->tr[5];
            }
            else
            {
              *dp++ = r;
              *dp++ = g;
              *dp++ = b;
            }
          }
        }
      }
    }
  }
}

void down2(std::vector<Image*>& img)
{
  unsigned int i, k, x, y;
  unsigned char * sp;
  unsigned char * dp;
  unsigned char r, g, b, a;
  int full_transp = 0;
  int grayscale = 1;
  unsigned char gray[256];
  COLORS col[256];
  unsigned int colors = 0;
  Image * image = img[0];

  memset(&gray, 0, sizeof(gray));

  for (i=0; i<256; i++)
  {
    col[i].num = 0;
    col[i].r = col[i].g = col[i].b = i;
    col[i].a =  255;
  }

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
      {
        r = *sp++;
        g = *sp++;
        b = *sp++;
        a = (image->ts > 0 && image->tr[1] == r && image->tr[3] == g && image->tr[5] == b) ? 0 : 255;

        if (a != 0)
        {
          if (r != g || g != b)
            grayscale = 0;
          else
            gray[r] = 1;
        }
        else
          full_transp = 1;

        if (colors <= 256)
        {
          int found = 0;
          for (k=0; k<colors; k++)
          if (col[k].r == r && col[k].g == g && col[k].b == b && col[k].a == a)
          {
            found = 1;
            col[k].num++;
            break;
          }
          if (found == 0)
          {
            if (colors < 256)
            {
              col[colors].num++;
              col[colors].r = r;
              col[colors].g = g;
              col[colors].b = b;
              col[colors].a = a;
            }
            colors++;
          }
        }
      }
    }
  }

  if (colors <= 256)
  {
    if (grayscale && colors > 128) /* 2 -> 0 */
    {
      image->type = 0;
      image->bpp = 1;
      unsigned char t = 0;
      int ts = 0;

      for (i=0; i<256; i++)
      {
        if (gray[i] == 0)
        {
          t = i;
          ts = 2;
          break;
        }
      }
      for (i=0; i<img.size(); i++)
      {
        for (y=0; y<image->h; y++)
        {
          sp = dp = img[i]->rows[y];
          for (x=0; x<image->w; x++, sp+=3)
          {
            *dp++ = (image->ts > 0 && image->tr[1] == sp[0] && image->tr[3] == sp[1] && image->tr[5] == sp[2]) ? t : sp[0];
          }
        }
      }
      if (ts > 0)
      {
        image->tr[0] = 0;
        image->tr[1] = t;
        image->ts = ts;
      }
    }
    else  /* 2 -> 3 */
    {
      image->type = 3;
      image->bpp = 1;

      if (full_transp == 0 && colors < 256)
        col[colors++].a = 0;

      qsort(&col[0], colors, sizeof(COLORS), cmp_colors);

      for (i=0; i<img.size(); i++)
      {
        for (y=0; y<image->h; y++)
        {
          sp = dp = img[i]->rows[y];
          for (x=0; x<image->w; x++)
          {
            r = *sp++;
            g = *sp++;
            b = *sp++;
            a = (image->ts > 0 && image->tr[1] == r && image->tr[3] == g && image->tr[5] == b) ? 0 : 255;
            for (k=0; k<colors; k++)
              if (col[k].r == r && col[k].g == g && col[k].b == b && col[k].a == a)
                break;
            *dp++ = k;
          }
        }
      }

      image->ps = colors;
      for (i=0; i<colors; i++)
      {
        image->pl[i].r = col[i].r;
        image->pl[i].g = col[i].g;
        image->pl[i].b = col[i].b;
        image->tr[i]   = col[i].a;
        if (image->tr[i] != 255) 
          image->ts = i+1;
      }
    }
  }
}

void down4(std::vector<Image*>& img)
{
  unsigned int i, k, x, y;
  unsigned char * sp;
  unsigned char * dp;
  unsigned char g, a;
  int simple_transp = 1;
  int full_transp = 0;
  unsigned char gray[256];
  COLORS col[256];
  unsigned int colors = 0;
  Image * image = img[0];

  memset(&gray, 0, sizeof(gray));

  for (i=0; i<256; i++)
  {
    col[i].num = 0;
    col[i].r = col[i].g = col[i].b = i;
    col[i].a = image->tr[i] = 255;
  }

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
      {
        g = *sp++;
        a = *sp++;

        if (a != 0)
        {
          if (a != 255)
            simple_transp = 0;
          else
            gray[g] = 1;
        }
        else
          full_transp = 1;

        if (colors <= 256)
        {
          int found = 0;
          for (k=0; k<colors; k++)
          if (col[k].g == g && col[k].a == a)
          {
            found = 1;
            col[k].num++;
            break;
          }
          if (found == 0)
          {
            if (colors < 256)
            {
              col[colors].num++;
              col[colors].r = g;
              col[colors].g = g;
              col[colors].b = g;
              col[colors].a = a;
            }
            colors++;
          }
        }
      }
    }
  }

  if (simple_transp && colors <= 256) /* 4 -> 0 */
  {
    image->type = 0;
    image->bpp = 1;
    unsigned char t = 0;

    for (i=0; i<256; i++)
    if (gray[i] == 0)
    {
      image->tr[0] = 0;
      image->tr[1] = t = i;
      image->ts = 2;
      break;
    }

    for (i=0; i<img.size(); i++)
    {
      for (y=0; y<image->h; y++)
      {
        sp = dp = img[i]->rows[y];
        for (x=0; x<image->w; x++, sp+=2)
        {
          *dp++ = (sp[1] == 0) ? t : sp[0];
        }
      }
    }
  }
  else
  if (colors <= 256)   /* 4 -> 3 */
  {
    image->type = 3;
    image->bpp = 1;

    if (full_transp == 0 && colors < 256)
      col[colors++].a = 0;

    qsort(&col[0], colors, sizeof(COLORS), cmp_colors);

    for (i=0; i<img.size(); i++)
    {
      for (y=0; y<image->h; y++)
      {
        sp = dp = img[i]->rows[y];
        for (x=0; x<image->w; x++)
        {
          g = *sp++;
          a = *sp++;
          for (k=0; k<colors; k++)
            if (col[k].g == g && col[k].a == a)
              break;
          *dp++ = k;
        }
      }
    }

    image->ps = colors;
    for (i=0; i<colors; i++)
    {
      image->pl[i].r = col[i].r;
      image->pl[i].g = col[i].g;
      image->pl[i].b = col[i].b;
      image->tr[i]   = col[i].a;
      if (image->tr[i] != 255) 
        image->ts = i+1;
    }
  }
}

void down3(std::vector<Image*>& img)
{
  unsigned int i, x, y;
  unsigned char * sp;
  unsigned char * dp;
  int c;
  int simple_transp = 1;
  int grayscale = 1;
  unsigned char gray[256];
  COLORS col[256];
  Image * image = img[0];

  memset(&gray, 0, sizeof(gray));

  for (c=0; c<256; c++)
  {
    col[c].num = 0;
    if (c < image->ps)
    {
      col[c].r = image->pl[c].r;
      col[c].g = image->pl[c].g;
      col[c].b = image->pl[c].b;
      col[c].a = image->tr[c];
    }
    else
    {
      col[c].r = col[c].g = col[c].b = c;
      col[c].a = 255;
    }
  }

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
        col[*sp++].num++;
    }
  }

  for (i=0; i<256; i++)
  if (col[i].num != 0)
  {
    if (col[i].a != 0)
    {
      if (col[i].a != 255)
        simple_transp = 0;
      else
      if (col[i].r != col[i].g || col[i].g != col[i].b)
        grayscale = 0;
      else
        gray[col[i].g] = 1;
    }
  }

  if (grayscale && simple_transp) /* 3 -> 0 */
  {
    image->type = 0;
    image->bpp = 1;
    unsigned char t = 0;
    int ts = 0;

    for (i=0; i<256; i++)
    {
      if (gray[i] == 0)
      {
        t = i;
        ts = 2;
        break;
      }
    }

    for (i=0; i<img.size(); i++)
    {
      for (y=0; y<image->h; y++)
      {
        dp = img[i]->rows[y];
        for (x=0; x<image->w; x++, dp++)
        {
          *dp = (col[*dp].a == 0) ? t : image->pl[*dp].g;
        }
      }
    }
    image->ps = 0;
    image->ts = 0;
    if (ts > 0)
    {
      image->tr[0] = 0;
      image->tr[1] = t;
      image->ts = ts;
    }
  }
}

void optim_downconvert(std::vector<Image*>& img)
{
  if (img[0]->type == 6)
    down6(img);
  else
  if (img[0]->type == 2)
    down2(img);
  else
  if (img[0]->type == 4)
    down4(img);
  else
  if (img[0]->type == 3)
    down3(img);
}

void optim_palette(std::vector<Image*>& img)
{
  unsigned int i, x, y;
  unsigned char * sp;
  unsigned char r, g, b, a;
  int c;
  int full_transp = 0;
  COLORS col[256];
  Image * image = img[0];

  for (c=0; c<256; c++)
  {
    col[c].num = 0;
    if (c < image->ps)
    {
      col[c].r = image->pl[c].r;
      col[c].g = image->pl[c].g;
      col[c].b = image->pl[c].b;
      col[c].a = image->tr[c];
    }
    else
    {
      col[c].r = col[c].g = col[c].b = c;
      col[c].a = 255;
    }
  }

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
        col[*sp++].num++;
    }
  }

  for (i=0; i<256; i++)
  {
    if (col[i].num != 0 && col[i].a == 0)
    {
      full_transp = 1;
      break;
    }
  }

  for (i=0; i<256; i++)
  {
    if (col[i].num == 0)
    {
      col[i].a = 255;
      if (full_transp == 0)
      {
        col[i].a = 0;
        full_transp = 1;
      }
    }
  }

  qsort(&col[0], 256, sizeof(COLORS), cmp_colors);

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
      {
        r = image->pl[*sp].r;
        g = image->pl[*sp].g;
        b = image->pl[*sp].b;
        a = image->tr[*sp];
        for (c=0; c<image->ps; c++)
          if (col[c].r == r && col[c].g == g && col[c].b == b && col[c].a == a)
            break;
        *sp++ = c;
      }
    }
  }

  for (i=0; i<256; i++)
  {
    image->pl[i].r = col[i].r;
    image->pl[i].g = col[i].g;
    image->pl[i].b = col[i].b;
    image->tr[i]   = col[i].a;
    if (col[i].num != 0)
      image->ps = i+1;
    if (image->tr[i] != 255) 
      image->ts = i+1;
  }
}

void add_transp2(std::vector<Image*>& img)
{
  unsigned int i, x, y;
  unsigned char * sp;
  unsigned char r, g, b;
  unsigned char cube[4096];
  Image * image = img[0];

  memset(&cube, 0, sizeof(cube));

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
      {
        r = *sp++;
        g = *sp++;
        b = *sp++;
        if (((r | g | b) & 15) == 0)
          cube[(r<<4) + g + (b>>4)] = 1;
      }
    }
  }

  for (i=0; i<4096; i++)
  if (cube[i] == 0)
  {
    image->tr[0] = 0;
    image->tr[1] = (i>>4)&0xF0;
    image->tr[2] = 0;
    image->tr[3] = i&0xF0;
    image->tr[4] = 0;
    image->tr[5] = (i<<4)&0xF0;
    image->ts = 6;
    break;
  }
}

void add_transp0(std::vector<Image*>& img)
{
  unsigned int i, x, y;
  unsigned char * sp;
  unsigned char gray[256];
  Image * image = img[0];

  memset(&gray, 0, sizeof(gray));

  for (i=0; i<img.size(); i++)
  {
    for (y=0; y<image->h; y++)
    {
      sp = img[i]->rows[y];
      for (x=0; x<image->w; x++)
        gray[*sp++] = 1;
    }
  }

  for (i=0; i<256; i++)
  if (gray[i] == 0)
  {
    image->tr[0] = 0;
    image->tr[1] = i;
    image->ts = 2;
    break;
  }
}

void optim_add_transp(std::vector<Image*>& img)
{
  if (img[0]->ts == 0)
  {
    if (img[0]->type == 2)
      add_transp2(img);
    else
    if (img[0]->type == 0)
      add_transp0(img);
  }
}

typedef struct { Image * image; unsigned int size; int x, y, w, h, valid, filters; } OP;
OP   op[6];

unsigned char * op_zbuf1;
unsigned char * op_zbuf2;
z_stream        op_zstream1;
z_stream        op_zstream2;

unsigned int    next_seq_num;
unsigned char * row_buf;
unsigned char * sub_row;
unsigned char * up_row;
unsigned char * avg_row;
unsigned char * paeth_row;
unsigned char   png_sign[8] = {137,  80,  78,  71,  13,  10,  26,  10};
unsigned char   png_Software[28] = { 83, 111, 102, 116, 119, 97, 114, 101, '\0',
                                     65,  80,  78,  71,  32, 65, 115, 115, 101,
                                    109,  98, 108, 101, 114, 32,  50,  46,  57,  49};

void write_chunk(FILE * f, const char * name, unsigned char * data, unsigned int length)
{
  unsigned char buf[4];
  unsigned int crc = (unsigned int)crc32(0, Z_NULL, 0);

  png_save_uint_32(buf, length);
  fwrite(buf, 1, 4, f);
  fwrite(name, 1, 4, f);
  crc = (unsigned int)crc32(crc, (const Bytef *)name, 4);

  if (memcmp(name, "fdAT", 4) == 0)
  {
    png_save_uint_32(buf, next_seq_num++);
    fwrite(buf, 1, 4, f);
    crc = (unsigned int)crc32(crc, buf, 4);
    length -= 4;
  }

  if (data != NULL && length > 0)
  {
    fwrite(data, 1, length, f);
    crc = (unsigned int)crc32(crc, data, length);
  }

  png_save_uint_32(buf, crc);
  fwrite(buf, 1, 4, f);
}

void write_IDATs(FILE * f, unsigned int frame, unsigned char * data, unsigned int length, unsigned int idat_size)
{
  unsigned int z_cmf = data[0];
  if ((z_cmf & 0x0f) == 8 && (z_cmf & 0xf0) <= 0x70)
  {
    if (length >= 2)
    {
      unsigned int z_cinfo = z_cmf >> 4;
      unsigned int half_z_window_size = 1 << (z_cinfo + 7);
      while (idat_size <= half_z_window_size && half_z_window_size >= 256)
      {
        z_cinfo--;
        half_z_window_size >>= 1;
      }
      z_cmf = (z_cmf & 0x0f) | (z_cinfo << 4);
      if (data[0] != (unsigned char)z_cmf)
      {
        data[0] = (unsigned char)z_cmf;
        data[1] &= 0xe0;
        data[1] += (unsigned char)(0x1f - ((z_cmf << 8) + data[1]) % 0x1f);
      }
    }
  }

  while (length > 0)
  {
    unsigned int ds = length;
    if (ds > 32768)
      ds = 32768;

    if (frame == 0)
      write_chunk(f, "IDAT", data, ds);
    else
      write_chunk(f, "fdAT", data, ds+4);

    data += ds;
    length -= ds;
  }
}

void process_rect(Image * image, int xbytes, int rowbytes, int y, int h, int bpp, unsigned char * dest)
{
  int i, j, v;
  int a, b, c, pa, pb, pc, p;
  unsigned char * prev = NULL;
  unsigned char * dp  = dest;
  unsigned char * out;

  for (j=y; j<y+h; j++)
  {
    unsigned char * row = image->rows[j] + xbytes;
    unsigned int    sum = 0;
    unsigned char * best_row = row_buf;
    unsigned int    mins = ((unsigned int)(-1)) >> 1;

    out = row_buf+1;
    for (i=0; i<rowbytes; i++)
    {
      v = out[i] = row[i];
      sum += (v < 128) ? v : 256 - v;
    }
    mins = sum;

    sum = 0;
    out = sub_row+1;
    for (i=0; i<bpp; i++)
    {
      v = out[i] = row[i];
      sum += (v < 128) ? v : 256 - v;
    }
    for (i=bpp; i<rowbytes; i++)
    {
      v = out[i] = row[i] - row[i-bpp];
      sum += (v < 128) ? v : 256 - v;
      if (sum > mins) break;
    }
    if (sum < mins)
    {
      mins = sum;
      best_row = sub_row;
    }

    if (prev)
    {
      sum = 0;
      out = up_row+1;
      for (i=0; i<rowbytes; i++)
      {
        v = out[i] = row[i] - prev[i];
        sum += (v < 128) ? v : 256 - v;
        if (sum > mins) break;
      }
      if (sum < mins)
      {
        mins = sum;
        best_row = up_row;
      }

      sum = 0;
      out = avg_row+1;
      for (i=0; i<bpp; i++)
      {
        v = out[i] = row[i] - prev[i]/2;
        sum += (v < 128) ? v : 256 - v;
      }
      for (i=bpp; i<rowbytes; i++)
      {
        v = out[i] = row[i] - (prev[i] + row[i-bpp])/2;
        sum += (v < 128) ? v : 256 - v;
        if (sum > mins) break;
      }
      if (sum < mins)
      {
        mins = sum;
        best_row = avg_row;
      }

      sum = 0;
      out = paeth_row+1;
      for (i=0; i<bpp; i++)
      {
        v = out[i] = row[i] - prev[i];
        sum += (v < 128) ? v : 256 - v;
      }
      for (i=bpp; i<rowbytes; i++)
      {
        a = row[i-bpp];
        b = prev[i];
        c = prev[i-bpp];
        p = b - c;
        pc = a - c;
        pa = abs(p);
        pb = abs(pc);
        pc = abs(p + pc);
        p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;
        v = out[i] = row[i] - p;
        sum += (v < 128) ? v : 256 - v;
        if (sum > mins) break;
      }
      if (sum < mins)
      {
        best_row = paeth_row;
      }
    }

    if (dest == NULL)
    {
      // deflate_rect_op()
      op_zstream1.next_in = row_buf;
      op_zstream1.avail_in = rowbytes + 1;
      deflate(&op_zstream1, Z_NO_FLUSH);

      op_zstream2.next_in = best_row;
      op_zstream2.avail_in = rowbytes + 1;
      deflate(&op_zstream2, Z_NO_FLUSH);
    }
    else
    {
      // deflate_rect_fin()
      memcpy(dp, best_row, rowbytes+1);
      dp += rowbytes+1;
    }

    prev = row;
  }
}

void deflate_rect_fin(int deflate_method, int iter, unsigned char * zbuf, unsigned int * zsize, int bpp, unsigned char * dest, int zbuf_size, int n)
{
  Image * image = op[n].image;
  int xbytes = op[n].x*bpp;
  int rowbytes = op[n].w*bpp;
  int y = op[n].y;
  int h = op[n].h;

  if (op[n].filters == 0)
  {
    unsigned char * dp = dest;
    for (int j=y; j<y+h; j++)
    {
      *dp++ = 0;
      memcpy(dp, image->rows[j] + xbytes, rowbytes);
      dp += rowbytes;
    }
  }
  else
    process_rect(image, xbytes, rowbytes, y, h, bpp, dest);

#ifdef FEATURE_ZOPFLI
  if (deflate_method == 2)
  {
    ZopfliOptions opt_zopfli;
    unsigned char* data = 0;
    size_t size = 0;
    ZopfliInitOptions(&opt_zopfli);
    opt_zopfli.numiterations = iter;
    ZopfliCompress(&opt_zopfli, ZOPFLI_FORMAT_ZLIB, dest, h*(rowbytes + 1), &data, &size);
    if (size < (size_t)zbuf_size)
    {
      memcpy(zbuf, data, size);
      *zsize = (unsigned int)size;
    }
    free(data);
  }
  else
#endif
#ifdef FEATURE_7ZIP
  if (deflate_method == 1)
  {
    unsigned size = zbuf_size;
    compress_rfc1950_7z(dest, h*(rowbytes + 1), zbuf, size, iter<100 ? iter : 100, 255);
    *zsize = size;
  }
  else
#endif
  {
    z_stream fin_zstream;

    fin_zstream.data_type = Z_BINARY;
    fin_zstream.zalloc = Z_NULL;
    fin_zstream.zfree = Z_NULL;
    fin_zstream.opaque = Z_NULL;
    deflateInit2(&fin_zstream, Z_BEST_COMPRESSION, 8, 15, 8, op[n].filters ? Z_FILTERED : Z_DEFAULT_STRATEGY);

    fin_zstream.next_out = zbuf;
    fin_zstream.avail_out = zbuf_size;
    fin_zstream.next_in = dest;
    fin_zstream.avail_in = h*(rowbytes + 1);
    deflate(&fin_zstream, Z_FINISH);
    *zsize = (unsigned int)fin_zstream.total_out;
    deflateEnd(&fin_zstream);
  }
}

void deflate_rect_op(Image * image, int x, int y, int w, int h, int bpp, int zbuf_size, int n)
{
  op_zstream1.data_type = Z_BINARY;
  op_zstream1.next_out = op_zbuf1;
  op_zstream1.avail_out = zbuf_size;

  op_zstream2.data_type = Z_BINARY;
  op_zstream2.next_out = op_zbuf2;
  op_zstream2.avail_out = zbuf_size;

  process_rect(image, x * bpp, w * bpp, y, h, bpp, NULL);

  deflate(&op_zstream1, Z_FINISH);
  deflate(&op_zstream2, Z_FINISH);
  op[n].image = image;

  if (op_zstream1.total_out < op_zstream2.total_out)
  {
    op[n].size = (unsigned int)op_zstream1.total_out;
    op[n].filters = 0;
  }
  else
  {
    op[n].size = (unsigned int)op_zstream2.total_out;
    op[n].filters = 1;
  }
  op[n].x = x;
  op[n].y = y;
  op[n].w = w;
  op[n].h = h;
  op[n].valid = 1;
  deflateReset(&op_zstream1);
  deflateReset(&op_zstream2);
}

void get_rect(unsigned int w, unsigned int h, Image * image1, Image * image2, Image * temp, unsigned int bpp, int zbuf_size, unsigned int has_tcolor, unsigned int tcolor, int n)
{
  unsigned int   i, j, x0, y0, w0, h0;
  unsigned int   x_min = w-1;
  unsigned int   y_min = h-1;
  unsigned int   x_max = 0;
  unsigned int   y_max = 0;
  unsigned int   diffnum = 0;
  unsigned int   over_is_possible = 1;

  if (!has_tcolor)
    over_is_possible = 0;

  if (bpp == 1)
  {
    for (j=0; j<h; j++)
    {
      unsigned char *pa = image1->rows[j];
      unsigned char *pb = image2->rows[j];
      unsigned char *pc = temp->rows[j];
      for (i=0; i<w; i++)
      {
        unsigned char c = *pb++;
        if (*pa++ != c)
        {
          diffnum++;
          if (has_tcolor && c == tcolor) over_is_possible = 0;
          if (i<x_min) x_min = i;
          if (i>x_max) x_max = i;
          if (j<y_min) y_min = j;
          if (j>y_max) y_max = j;
        }
        else
          c = tcolor;

        *pc++ = c;
      }
    }
  }
  else
  if (bpp == 2)
  {
    for (j=0; j<h; j++)
    {
      unsigned short *pa = (unsigned short *)image1->rows[j];
      unsigned short *pb = (unsigned short *)image2->rows[j];
      unsigned short *pc = (unsigned short *)temp->rows[j];
      for (i=0; i<w; i++)
      {
        unsigned int c1 = *pa++;
        unsigned int c2 = *pb++;
        if ((c1 != c2) && ((c1>>8) || (c2>>8)))
        {
          diffnum++;
          if ((c2 >> 8) != 0xFF) over_is_possible = 0;
          if (i<x_min) x_min = i;
          if (i>x_max) x_max = i;
          if (j<y_min) y_min = j;
          if (j>y_max) y_max = j;
        }
        else
          c2 = 0;

        *pc++ = c2;
      }
    }
  }
  else
  if (bpp == 3)
  {
    for (j=0; j<h; j++)
    {
      unsigned char *pa = image1->rows[j];
      unsigned char *pb = image2->rows[j];
      unsigned char *pc = temp->rows[j];
      for (i=0; i<w; i++)
      {
        unsigned int c1 = (pa[2]<<16) + (pa[1]<<8) + pa[0];
        unsigned int c2 = (pb[2]<<16) + (pb[1]<<8) + pb[0];
        if (c1 != c2)
        {
          diffnum++;
          if (has_tcolor && c2 == tcolor) over_is_possible = 0;
          if (i<x_min) x_min = i;
          if (i>x_max) x_max = i;
          if (j<y_min) y_min = j;
          if (j>y_max) y_max = j;
        }
        else
          c2 = tcolor;

        memcpy(pc, &c2, 3);
        pa += 3;
        pb += 3;
        pc += 3;
      }
    }
  }
  else
  if (bpp == 4)
  {
    for (j=0; j<h; j++)
    {
      unsigned int *pa = (unsigned int *)image1->rows[j];
      unsigned int *pb = (unsigned int *)image2->rows[j];
      unsigned int *pc = (unsigned int *)temp->rows[j];
      for (i=0; i<w; i++)
      {
        unsigned int c1 = *pa++;
        unsigned int c2 = *pb++;
        if ((c1 != c2) && ((c1>>24) || (c2>>24)))
        {
          diffnum++;
          if ((c2 >> 24) != 0xFF) over_is_possible = 0;
          if (i<x_min) x_min = i;
          if (i>x_max) x_max = i;
          if (j<y_min) y_min = j;
          if (j>y_max) y_max = j;
        }
        else
          c2 = 0;

        *pc++ = c2;
      }
    }
  }

  if (diffnum == 0)
  {
    x0 = y0 = 0;
    w0 = h0 = 1;
  }
  else
  {
    x0 = x_min;
    y0 = y_min;
    w0 = x_max-x_min+1;
    h0 = y_max-y_min+1;
  }

  deflate_rect_op(image2, x0, y0, w0, h0, bpp, zbuf_size, n*2);

  if (over_is_possible)
    deflate_rect_op(temp, x0, y0, w0, h0, bpp, zbuf_size, n*2+1);
}

int save_apng_to_file(
  char * szOut, std::vector<Image*>& img, 
  unsigned int loops, 
  unsigned int first, 
  int deflate_method, 
  int iter,
  int &cur,
  int &total
) {
  unsigned char coltype = img[0]->type;
  unsigned int has_tcolor = 0;
  unsigned int tcolor = 0;

  if (coltype == 0)
  {
    if (img[0]->ts)
    {
      has_tcolor = 1;
      tcolor = img[0]->tr[1];
    }
  }
  else
  if (coltype == 2)
  {
    if (img[0]->ts)
    {
      has_tcolor = 1;
      tcolor = (((img[0]->tr[5]<<8)+img[0]->tr[3])<<8)+img[0]->tr[1];
    }
  }
  else
  if (coltype == 3)
  {
    for (int c=0; c<img[0]->ts; c++)
    if (img[0]->tr[c] == 0)
    {
      has_tcolor = 1;
      tcolor = c;
      break;
    }
  }
  else
    has_tcolor = 1;

  FILE * f;
  if ((f = fopen(szOut, "wb")) == 0)
  {
    printf("Error: can't save to file '%s'\n", szOut);
    return 1;
  }

  unsigned char buf_IHDR[13];
  unsigned char buf_acTL[8];
  unsigned char buf_fcTL[26];

  unsigned int width = img[0]->w;
  unsigned int height = img[0]->h;
  unsigned int bpp = img[0]->bpp;
  unsigned int rowbytes  = width * bpp;
  unsigned int visible = (unsigned int)(img.size() - first);

  Image temp, over1, over2, over3, rest;
  temp.init(img[0]);
  over1.init(img[0]);
  over2.init(img[0]);
  over3.init(img[0]);
  rest.init(img[0]);
  unsigned char * dest  = new unsigned char[(rowbytes + 1) * height];

  png_save_uint_32(buf_IHDR, width);
  png_save_uint_32(buf_IHDR + 4, height);
  buf_IHDR[8] = 8;
  buf_IHDR[9] = coltype;
  buf_IHDR[10] = 0;
  buf_IHDR[11] = 0;
  buf_IHDR[12] = 0;

  png_save_uint_32(buf_acTL, visible);
  png_save_uint_32(buf_acTL + 4, loops);

  fwrite(png_sign, 1, 8, f);

  write_chunk(f, "IHDR", buf_IHDR, 13);

  if (img.size() > 1)
    write_chunk(f, "acTL", buf_acTL, 8);
  else
    first = 0;

  if (img[0]->ps > 0)
    write_chunk(f, "PLTE", (unsigned char *)(&(img[0]->pl)), img[0]->ps*3);

  if (img[0]->ts > 0)
    write_chunk(f, "tRNS", img[0]->tr, img[0]->ts);

  op_zstream1.data_type = Z_BINARY;
  op_zstream1.zalloc = Z_NULL;
  op_zstream1.zfree = Z_NULL;
  op_zstream1.opaque = Z_NULL;
  deflateInit2(&op_zstream1, Z_BEST_SPEED+1, 8, 15, 8, Z_DEFAULT_STRATEGY);

  op_zstream2.data_type = Z_BINARY;
  op_zstream2.zalloc = Z_NULL;
  op_zstream2.zfree = Z_NULL;
  op_zstream2.opaque = Z_NULL;
  deflateInit2(&op_zstream2, Z_BEST_SPEED+1, 8, 15, 8, Z_FILTERED);

  unsigned int idat_size = (rowbytes + 1) * height;
  unsigned int zbuf_size = idat_size + ((idat_size + 7) >> 3) + ((idat_size + 63) >> 6) + 11;

  unsigned char * zbuf = new unsigned char[zbuf_size];
  op_zbuf1 = new unsigned char[zbuf_size];
  op_zbuf2 = new unsigned char[zbuf_size];
  row_buf = new unsigned char[rowbytes + 1];
  sub_row = new unsigned char[rowbytes + 1];
  up_row = new unsigned char[rowbytes + 1];
  avg_row = new unsigned char[rowbytes + 1];
  paeth_row = new unsigned char[rowbytes + 1];

  row_buf[0] = 0;
  sub_row[0] = 1;
  up_row[0] = 2;
  avg_row[0] = 3;
  paeth_row[0] = 4;

  unsigned int i, j, k;
  unsigned int zsize = 0;
  unsigned int x0 = 0;
  unsigned int y0 = 0;
  unsigned int w0 = width;
  unsigned int h0 = height;
  unsigned char bop = 0;
  unsigned char dop = 0;
  next_seq_num = 0;

  printf("saving %s (frame %d of %d)\n", szOut, 1-first, visible);
  for (j=0; j<6; j++)
    op[j].valid = 0;
  deflate_rect_op(img[0], x0, y0, w0, h0, bpp, zbuf_size, 0);
  deflate_rect_fin(deflate_method, iter, zbuf, &zsize, bpp, dest, zbuf_size, 0);

  if (first)
  {
    write_IDATs(f, 0, zbuf, zsize, idat_size);

    printf("saving %s (frame %d of %d)\n", szOut, 1, visible);
    for (j=0; j<6; j++)
      op[j].valid = 0;
    deflate_rect_op(img[1], x0, y0, w0, h0, bpp, zbuf_size, 0);
    deflate_rect_fin(deflate_method, iter, zbuf, &zsize, bpp, dest, zbuf_size, 0);
  }
  total = img.size();
  cur = 0;
  for (i=first; i<img.size()-1; i++)
  {
    cur = i;
    unsigned int op_min;
    int          op_best;

    printf("saving %s (frame %d of %d)\n", szOut, i-first+2, visible);
    for (j=0; j<6; j++)
      op[j].valid = 0;

    /* dispose = none */
    get_rect(width, height, img[i], img[i+1], &over1, bpp, zbuf_size, has_tcolor, tcolor, 0);

    /* dispose = background */
    if (has_tcolor)
    {
      for (j=0; j<height; j++)
        memcpy(temp.rows[j], img[i]->rows[j], rowbytes);
      if (coltype == 2)
        for (j=0; j<h0; j++)
          for (k=0; k<w0; k++)
            memcpy(temp.rows[j+y0] + (k+x0)*3, &tcolor, 3);
      else
        for (j=0; j<h0; j++)
          memset(temp.rows[j+y0] + x0*bpp, tcolor, w0*bpp);

      get_rect(width, height, &temp, img[i+1], &over2, bpp, zbuf_size, has_tcolor, tcolor, 1);
    }

    /* dispose = previous */
    if (i > first)
      get_rect(width, height, &rest, img[i+1], &over3, bpp, zbuf_size, has_tcolor, tcolor, 2);

    op_min = op[0].size;
    op_best = 0;
    for (j=1; j<6; j++)
    if (op[j].valid)
    {
      if (op[j].size < op_min)
      {
        op_min = op[j].size;
        op_best = j;
      }
    }

    dop = op_best >> 1;

    png_save_uint_32(buf_fcTL, next_seq_num++);
    png_save_uint_32(buf_fcTL + 4, w0);
    png_save_uint_32(buf_fcTL + 8, h0);
    png_save_uint_32(buf_fcTL + 12, x0);
    png_save_uint_32(buf_fcTL + 16, y0);
    png_save_uint_16(buf_fcTL + 20, img[i]->delay_num);
    png_save_uint_16(buf_fcTL + 22, img[i]->delay_den);
    buf_fcTL[24] = dop;
    buf_fcTL[25] = bop;
    write_chunk(f, "fcTL", buf_fcTL, 26);

    write_IDATs(f, i, zbuf, zsize, idat_size);

    /* process apng dispose - begin */
    if (dop != 2)
      for (j=0; j<height; j++)
        memcpy(rest.rows[j], img[i]->rows[j], rowbytes);

    if (dop == 1)
    {
      if (coltype == 2)
        for (j=0; j<h0; j++)
          for (k=0; k<w0; k++)
            memcpy(rest.rows[j+y0] + (k+x0)*3, &tcolor, 3);
      else
        for (j=0; j<h0; j++)
          memset(rest.rows[j+y0] + x0*bpp, tcolor, w0*bpp);
    }
    /* process apng dispose - end */

    x0 = op[op_best].x;
    y0 = op[op_best].y;
    w0 = op[op_best].w;
    h0 = op[op_best].h;
    bop = op_best & 1;

    deflate_rect_fin(deflate_method, iter, zbuf, &zsize, bpp, dest, zbuf_size, op_best);
  }

  if (img.size() > 1)
  {
    png_save_uint_32(buf_fcTL, next_seq_num++);
    png_save_uint_32(buf_fcTL + 4, w0);
    png_save_uint_32(buf_fcTL + 8, h0);
    png_save_uint_32(buf_fcTL + 12, x0);
    png_save_uint_32(buf_fcTL + 16, y0);
    png_save_uint_16(buf_fcTL + 20, img.back()->delay_num);
    png_save_uint_16(buf_fcTL + 22, img.back()->delay_den);
    buf_fcTL[24] = 0;
    buf_fcTL[25] = bop;
    write_chunk(f, "fcTL", buf_fcTL, 26);
  }

  write_IDATs(f, (unsigned int)(img.size()-1), zbuf, zsize, idat_size);

  write_chunk(f, "tEXt", png_Software, 28);
  write_chunk(f, "IEND", 0, 0);
  fclose(f);

  delete[] zbuf;
  delete[] op_zbuf1;
  delete[] op_zbuf2;
  delete[] row_buf;
  delete[] sub_row;
  delete[] up_row;
  delete[] avg_row;
  delete[] paeth_row;

  deflateEnd(&op_zstream1);
  deflateEnd(&op_zstream2);

  temp.free();
  over1.free();
  over2.free();
  over3.free();
  rest.free();
  delete[] dest;

  return 0;
}
