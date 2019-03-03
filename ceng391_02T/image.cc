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

using std::ofstream;
using std::ios;
using std::cerr;
using std::string;
using std::cout;

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
                x = 0;
                width += x;
        }

        if (y < 0) {
                y = 0;
                height += y;
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

void Image::set_rect_rgb(int x , int y, int width, int height, uchar red, uchar green , uchar blue) {
        if(m_n_channels == 1) {
                set_rect(x,y,width,height, (red + green + blue) / 3);
        }
        else if(m_n_channels == 3) {
                uchar rgb[] = {red, green, blue};
                for(int j = y; j < y + height; ++j) {
                        if(j >= m_height)
                                break;
                        uchar* row_data = data(j);
                        for(int i = x; i < x + width; ++i) {
                                if(i > m_width)
                                        break;
                                for(int c = 0; c < m_n_channels; ++c) 
                                        row_data[i*m_n_channels + c] = rgb[c];
                        }
                }
        }
        else {
                cerr << "Only grayscale and rgb images supoorted.";
        }
}

bool Image::write_pnm(const std::string& filename) const
{
        string magic_head = "P5";
        ofstream fout;
        string extension = ".pgm";
        string extended_name = filename;
        
        if (m_n_channels == 3) {
                magic_head = "P6";
                extension = ".ppm";
                extended_name += extension;
                fout.open(extended_name.c_str(), ios::out | ios::binary);

                fout << magic_head << "\n";
                fout << m_width << " " << m_height << " 255\n";
                for(int y = 0; y < m_height; ++y) {
                        const uchar *row_data = data(y);
                        fout.write(reinterpret_cast<const char*>(row_data), m_step * sizeof(uchar));
                }
               
        }
        else if(m_n_channels == 1) {
                extension = ".pgm";
                extended_name += extension;
                fout.open(extended_name.c_str(), ios::out | ios::binary);

                fout << magic_head << "\n";
                fout << m_width << " " << m_height << " 255\n";
                for (int y = 0; y < m_height; ++y) {
                        const uchar *row_data = data(y);
                        fout.write(reinterpret_cast<const char*>(row_data), m_width*sizeof(uchar));
                }
        }
        else {
                cerr << "Only rgb and grayscale images accepted";
        }

        fout.close();

        return true;
}


}
