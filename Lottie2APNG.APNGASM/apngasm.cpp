/* APNG Assembler 2.91
 *
 * This program creates APNG animation from PNG/TGA image sequence.
 *
 * http://apngasm.sourceforge.net/
 *
 * Copyright (c) 2009-2016 Max Stepin
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
#include "png.h"     /* original (unpatched) libpng is ok */
#include "zlib.h"
#include "apngmanager.h"
#include <sstream>
#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
  std::vector<Image> img;
  unsigned int i;
  unsigned int first = 0;
  unsigned int loops = 0;
  int delay_num = -1;
  int delay_den = -1;
  int hs = 0;
  int vs = 0;
  int keep_palette = 0;
  int keep_coltype = 0;
  int deflate_method = 0;
  int iter = 15;
  int res = 0;

  printf("\nAPNG Assembler 2.91\n");

  api_result testres;

  ApngManager *manager;
  manager = new ApngManager();
  manager->SetSize(128, 128);
  manager->SetFps(1, 10);
  
  const char* filename = "readme.txt"; // 替换为你的文件名
  testres = manager->AddFrameFromFile(filename);
  std::cout<<(testres.success)<<std::endl;
  // filename = "18.png"; // 替换为你的文件名
  // manager->AddFrameFromFile(filename);
  // manager->Optimize();
  // delete manager;
  
  // manager.Save("merge.png");
  printf("ok");
  return 0;

// #ifdef FEATURE_ZOPFLI
//   deflate_method = 2;
// #endif
// #ifdef FEATURE_7ZIP
//   deflate_method = 1;
// #endif

//   if (argc <= 2)
//   {
//     printf("\n\nUsage   : apngasm output.png frame001.png [options]\n"
//                "          apngasm output.png frame*.png   [options]\n\n"
//                "Options :\n"
//                "1 10    : frame delay is 1/10 sec. (default)\n"
//                "-l2     : 2 loops (default is 0, forever)\n"
//                "-f      : skip the first frame\n"
//                "-hs##   : input is horizontal strip of ## frames (example: -hs12)\n"
//                "-vs##   : input is vertical strip of ## frames   (example: -vs12)\n"
//                "-kp     : keep palette\n"
//                "-kc     : keep color type\n"
//                "-z0     : zlib compression\n");
// #ifdef FEATURE_7ZIP
//     printf(    "-z1     : 7zip compression%s\n", (deflate_method == 1) ? " (default)" : "");
// #endif
// #ifdef FEATURE_ZOPFLI
//     printf(    "-z2     : Zopfli compression%s\n", (deflate_method == 2) ? " (default)" : "");
// #endif
//     if (deflate_method)
//       printf(  "-i##    : number of iterations (default -i%d)\n", iter);
//     return 1;
//   }

//   char * szOut = argv[1];
//   char * szImage = argv[2];

//   for (int c=3; c<argc; c++)
//   {
//     char * szOption = argv[c];

//     if (szOption[0] == '/' || szOption[0] == '-')
//     {
//       if (szOption[1] == 'f' || szOption[1] == 'F')
//         first = 1;
//       else
//       if (szOption[1] == 'l' || szOption[1] == 'L')
//         loops = atoi(szOption+2);
//       else
//       if (szOption[1] == 'k' || szOption[1] == 'K')
//       {
//         if (szOption[2] == 'p' || szOption[2] == 'P')
//           keep_palette = 1;
//         else
//         if (szOption[2] == 'c' || szOption[2] == 'C')
//           keep_coltype = 1;
//       }
//       else
//       if (szOption[1] == 'z' || szOption[1] == 'Z')
//       {
//         if (szOption[2] == '0')
//           deflate_method = 0;
// #ifdef FEATURE_7ZIP
//         if (szOption[2] == '1')
//           deflate_method = 1;
// #endif
// #ifdef FEATURE_ZOPFLI
//         if (szOption[2] == '2')
//           deflate_method = 2;
// #endif
//       }
//       else
//       if (szOption[1] == 'i' || szOption[1] == 'I')
//       {
//         iter = atoi(szOption+2);
//         if (iter < 1) iter = 1;
//       }
//       else
//       if ((szOption[1] == 'h' || szOption[1] == 'H') && (szOption[2] == 's' || szOption[2] == 'S'))
//       {
//         hs = atoi(szOption+3);
//         if (hs < 1) hs = 1;
//       }
//       else
//       if ((szOption[1] == 'v' || szOption[1] == 'V') && (szOption[2] == 's' || szOption[2] == 'S'))
//       {
//         vs = atoi(szOption+3);
//         if (vs < 1) vs = 1;
//       }
//     }
//     else
//     {
//       int n = atoi(szOption);
//       if ((n != 0) || (strcmp(szOption, "0") == 0))
//       {
//         if (delay_num == -1) delay_num = n;
//         else
//         if (delay_den == -1) delay_den = n;
//       }
//     }
//   }

//   if (delay_num <= 0) delay_num = 1;
//   if (delay_den <= 0) delay_den = 10;

//   if (deflate_method == 0)
//     printf(" using ZLIB\n\n");
//   else if (deflate_method == 1)
//     printf(" using 7ZIP with %d iterations\n\n", iter);
//   else if (deflate_method == 2)
//     printf(" using ZOPFLI with %d iterations\n\n", iter);

//   unsigned char coltype = 6;

//   if (vs > 1)
//     res = load_from_vertical_strip(szImage, vs, img, delay_num, delay_den, &coltype);
//   else
//   if (hs > 1)
//     res = load_from_horizontal_strip(szImage, hs, img, delay_num, delay_den, &coltype);
//   else
//     res = load_image_sequence(szImage, first, img, delay_num, delay_den, &coltype);

//   if (res)
//     return 1;

//   for (i=0; i<img.size(); i++)
//   {
//     if (img[i].type != coltype)
//       optim_upconvert(&img[i], coltype);
//   }

//   if (coltype == 6 || coltype == 4)
//   {
//     for (i=0; i<img.size(); i++)
//       optim_dirty_transp(&img[i]);
//   }

  // optim_duplicates(img, first);

  // if (!keep_coltype)
  //   optim_downconvert(img);

  // coltype = img[0].type;

  // if (coltype == 3 && !keep_palette)
  //   optim_palette(img);

  // if (coltype == 2 || coltype == 0)
  //   optim_add_transp(img);

  // res = save_apng(szOut, img, loops, first, deflate_method, iter);

  for (i=0; i<img.size(); i++)
    img[i].free();

  if (res)
    return 1;

  printf("all done\n");

  return 0;
}



