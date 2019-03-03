// ------------------------------
// Written by Mustafa Ozuysal
// Contact <mustafaozuysal@iyte.edu.tr> for comments and bug reports
// ------------------------------
// Copyright (c) 2018, Mustafa Ozuysal
// All rights reserved.
// ------------------------------
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the copyright holders nor the
//       names of his/its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// ------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------------------------------
#include "image.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>


using std::ofstream;
using std::ios;
using std::cerr;
using std::string;
using std::fopen;
using std::fputc;
using std::exit;
using std::fclose;
using std::fscanf;
using std::fread;
using std::cos;
using std::sin;

namespace ceng391 {

Image::Image(int width, int height, int n_channels, int step)
{
        m_width = width;
        m_height = height;
        m_n_channels = n_channels;

        m_step = m_width*m_n_channels;
        if (m_step < step)
                m_step = step;
        m_data = new uchar[m_step*height];
}

Image::~Image()
{
        delete [] m_data;
}

Image* Image::new_gray(int width, int height)
{
        return new Image(width, height, 1);
}

Image* Image::new_rgb(int width, int height)
{
        return new Image(width, height, 3);
}

void Image::set_rect(int x, int y, int width, int height, uchar value)
{
        if (x < 0) {
                width += x;
                x = 0;
        }

        if (y < 0) {
                height += y;
                y = 0;
        }

        for (int j = y; j < y+height; ++j) {
                if (j >= m_height)
                        break;
                uchar* row_data = data(j);
                for (int i = x; i < x+width; ++i) {
                        if (i >= m_width)
                                break;
                        for (int c = 0; c < m_n_channels; ++c)
                                row_data[i*m_n_channels + c] = value;
                }
        }
}

uchar* Image::scaleup_nn(int scale) {
        int height = scale * m_height;
        int width = scale * m_width;
        int step = width;
        uchar* imgScaled = new uchar[step * height];

        // initialize all pixel to black
        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        imgScaled[step * i + j] = 0;
                }
        }

        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        imgScaled[step * i + j] = data((int) round(i / scale))[(int) round(j / scale)];
                }
        }
        
        m_step = step;
        m_height = height;
        m_width = width;
        
        delete m_data;
        m_data = imgScaled;

        return imgScaled;
}

uchar* Image::scaleup_bilinear(int scale) {
        int height = scale * m_height;
        int width = scale * m_width;
        int step = width;
        
        float iRatio = (m_height - 1) / (float) height;
        float jRatio = (m_width - 1) / (float) width;

        uchar* imgScaled = new uchar[step * height];

        // initialize all pixel to black
        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        imgScaled[step * i + j] = 0;
                }
        }

        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        int x = (int) (iRatio * i);
                        int y = (int) (jRatio * j);
                 
                        float alpha = iRatio * i- x;
                        float beta = jRatio * j - y;
                
                        int intensity =   (1 - alpha) * (1 - beta) * m_data[m_step * x + y]
                                        + alpha       * (1 - beta) * m_data[m_step * (x + 1) + y]
                                        + (1 - alpha) * beta       * m_data[m_step * x + y + 1]
                                        + alpha       * beta       * m_data[m_step * (x + 1) + y + 1];
                        
                        imgScaled[step * i + j] = intensity;
                }
        }
        
        m_step = step;
        m_height = height;
        m_width = width;
        
        delete m_data;
        m_data = imgScaled;

        return 0;
}

bool Image::write_pnm(const std::string& filename) const
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be saved as PNM files!\n";
                return false;
        }

        const string magic_head = "P5";
        ofstream fout;
        string extended_name = filename + ".pgm";
        fout.open(extended_name.c_str(), ios::out | ios::binary);
        fout << magic_head << "\n";
        fout << m_width << " " << m_height << " 255\n";
        for (int y = 0; y < m_height; ++y) {
                const uchar *row_data = data(y);
                fout.write(reinterpret_cast<const char*>(row_data), m_width*sizeof(uchar));
        }
        fout.close();

        return true;
}

Image* Image::read_pnm(const std::string& filename)
{
        FILE *pnm = fopen(filename.c_str(), "rb");
        if (!pnm) {
                fprintf(stderr, "Could not open image file %s\n", filename.c_str());
                exit(EXIT_FAILURE);
        }

        char ch1, ch2;
        if (fscanf(pnm, "%c%c", &ch1, &ch2) != 2) {
                fprintf(stderr, "Could not read image header from %s\n", filename.c_str());
                exit(EXIT_FAILURE);
        }

        if(ch1 != 'P' || (ch2 != '5' && ch2 != '6')) {
                fprintf(stderr, "Image %s is not a valid binary PGM or PPM file", filename.c_str());
                exit(EXIT_FAILURE);
        }

        int n_ch = -1;
        if(ch2 == '5') {
                n_ch = 1;
        } else if(ch2 == '6') {
                n_ch = 3;
        }

        int c = fgetc(pnm);
        while (c == ' ' || c == '\n' || c == '\t') {
                c = fgetc(pnm);
        }

        while (c == '#') {
                while (c != '\n') {
                        c = fgetc(pnm);
                        if (c == EOF) {
                                fprintf(stderr, "%s does not contain image data", filename.c_str());
                                exit(EXIT_FAILURE);
                        }
                }
                c = fgetc(pnm);
        }
        ungetc(c, pnm);

        int pnm_width;
        int pnm_height;
        int pnm_levels;
        if (fscanf(pnm, "%d %d %d", &pnm_width, &pnm_height, &pnm_levels) != 3) {
                fprintf(stderr, "Could not read image attributes from %s", filename.c_str());
                exit(EXIT_FAILURE);
        }
        c = fgetc(pnm);

        Image* img = new Image(pnm_width, pnm_height, n_ch);
        for (int y = 0; y < img->m_height; ++y) {
                uchar* row = img->data(y);
                if (fread((void*)row, sizeof(uchar), img->m_width*n_ch, pnm) != img->m_width*n_ch) {
                        fprintf(stderr, "Could not read data line %d from %s", y, filename.c_str());
                        exit(EXIT_FAILURE);
                }
        }
        fclose(pnm);

        return img;
}

}
