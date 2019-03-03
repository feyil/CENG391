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

const double pi = std::acos(-1);

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

uchar* Image::rotate_bilinear(float angle) {
    
        int height = m_height;
        int width = m_width;
        int step = width;

        uchar* rotatedImage = new uchar[step * height];

        // intialize the image
        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        rotatedImage[step * i + j] = 150;
                }
        }

        // define center
        int center[] = { height / 2, width / 2};
             
        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        // move center
                        float* cord = new float[2];
                        cord[0] = i - center[0];
                        cord[1] = j - center[1];

                        // multiply by inverse of rotation matrix
                        rotate_cord(angle, cord, 1);
                        // move center
                        float rotatedX = cord[0] + center[0];
                        float rotatedY = cord[1] + center[1];

                        // assign intensity value to destination
                        if(rotatedX < m_height && rotatedY < m_width) {
                               int intensity = interpolate_bilinear(rotatedX, rotatedY);
                               rotatedImage[step * i + j] = intensity;
                        }

                        delete cord;                        
                }
        }

        m_height = height;
        m_width = width;
        m_step = step;

        delete m_data;
        m_data = rotatedImage;

        return rotatedImage;
}

uchar* Image::rotate_full_bilinear(float angle) { 
        float** window = new float*[4];
        calculate_window_size(angle, window);
        
        int height = abs(window[0][0] - window[3][0]);
        int width = abs(window[1][1] - window[2][1]);
        int step = width;

        for(int i = 0; i < 4; i++) {
                delete window[i];
        }
        delete window;

        uchar* rotatedImage = new uchar[step * height];

        // intialize the image
        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        rotatedImage[step * i + j] = 150;
                }
        }

        // define center
        int center[] = { height / 2, width / 2};
             
        for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                        // move center
                        float* cord = new float[2];
                        cord[0] = i - center[0];
                        cord[1] = j - center[1];

                        // multiply by inverse of rotation matrix
                        rotate_cord(angle, cord, 1);
                        // move center
                     
                        float rotatedX = cord[0] + center[0];
                        float rotatedY = cord[1] + center[1];
                        
                        if(rotatedX < 0) {
                                rotatedX = height + rotatedX;
                        }
                 
                        // assign intensity value to destination
                        if(rotatedX < m_height && rotatedY < m_width) {
                               int intensity = interpolate_bilinear(rotatedX, rotatedY);
                               rotatedImage[step * i + j] = intensity;
                        }

                        delete cord;                        
                }
        }

        m_height = height;
        m_width = width;
        m_step = step;

        delete m_data;
        m_data = rotatedImage;

        return rotatedImage;
}

void Image::rotate_cord(float angle, float *cord, int flag = 1) {
        // degree to radian conversion
        float degree = (angle * pi) / 180.0;

        // multiply by inverse of rotation matrix and move center
        float rotatedX = cord[0] * cos(degree) - flag * cord[1] * sin(degree);
        float rotatedY = flag * cord[0] * sin(degree) + cord[1] * cos(degree);  

        cord[0] = rotatedX;
        cord[1] = rotatedY;
}

void Image::calculate_window_size(float angle, float** window) {
        float* topLeft = new float[2];
        topLeft[0] = 0 - 347;
        topLeft[1] = 0 - 512;        
        rotate_cord(angle, topLeft, -1);
        
        float* bottomRight = new float[2];
        bottomRight[0] = m_height - 347;
        bottomRight[1] = m_width - 512;
        rotate_cord(angle, bottomRight, -1);

        int height = abs(topLeft[0] - bottomRight[0]);
        cerr<<height<<std::endl;
       
        float* topRight = new float[2];
        topRight[0] = 0 - 347;
        topRight[1] = m_width - 512;
        rotate_cord(angle, topRight, -1);

        float* bottomLeft = new float[2];
        bottomLeft[0] = m_height - 347;
        bottomLeft[1] = 0 - 512;
        rotate_cord(angle, bottomLeft, -1);

        int width = abs(topRight[1] - bottomLeft[1]);
        cerr<<width<<std::endl;
      

        window[0] = topLeft;
        window[1] = topRight;
        window[2] = bottomLeft;
        window[3] = bottomRight;

}

int Image::interpolate_bilinear(float rotatedX, float rotatedY ) {
     
        int x = (int) rotatedX;
        int y = (int) rotatedY;
                 
        float alpha = rotatedX - x;
        float beta = rotatedY - y;
             
        int intensity =   (1 - alpha) * (1 - beta) * m_data[m_step * x + y]
                        + alpha       * (1 - beta) * m_data[m_step * (x + 1) + y]
                        + (1 - alpha) * beta       * m_data[m_step * x + y + 1]
                        + alpha       * beta       * m_data[m_step * (x + 1) + y + 1];
             

   
        return intensity;
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
