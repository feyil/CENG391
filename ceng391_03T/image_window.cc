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
#include "image_window.h"

#include <iostream>
#include <QApplication>

using std::cerr;
using std::endl;

namespace ceng391 {

static QImage* convert_to_qimage(Image* img);

ImageWindow::ImageWindow(const QString &title, Image *img)
{
        m_image = img;
        setWindowTitle(title);

        m_label = new QLabel(this);
        if (img != 0) {
                QImage* qimg = convert_to_qimage(img);
                if (qimg != 0) {
                        m_label->setPixmap(QPixmap::fromImage(*qimg));
                        resize(img->w(), img->h() + 80);
                        delete qimg;
                }
        }

        m_brightness= new QScrollBar(this);
        m_brightness->setOrientation(Qt::Horizontal);
        m_brightness->setRange(-100,100);
        m_brightness->setValue(0);
        m_brightness->setGeometry(0,img->h(),128,30);

        m_contrast = new QScrollBar(this);
        m_contrast->setOrientation(Qt::Horizontal);
        m_contrast->setRange(0,200);
        m_contrast->setValue(100);
        m_contrast->setGeometry(0,img->h() + 40,128,30);

        QObject::connect(m_brightness, SIGNAL (valueChanged(int)), this, SLOT (changeBrightness(int)));
        QObject::connect(m_contrast, SIGNAL (valueChanged(int)), this, SLOT (changeContrast(int)));
        QObject::connect(QApplication::instance(), SIGNAL (aboutToQuit()), this, SLOT (releaseData()));
}

QImage* convert_to_qimage(Image* img)
{       
        if (img->n_ch() == 1) { 
                QImage *qimg = new QImage(img->data(), img->w(), img->h(), img->step(), QImage::Format_Grayscale8);
                
                return qimg;
        } else {
                cerr << "[ERROR][ImageWindow] Can only load grayscale images!" << endl;
                return 0;
        }
}

void ImageWindow::changeBrightness(int value) {
        uchar* transform_data = m_image->transformImage(m_contrast->value() * 0.01,value);
        QImage *qimg = new QImage(transform_data, m_image->w(), m_image->h(), m_image->step(), QImage::Format_Grayscale8);
        m_label->setPixmap(QPixmap::fromImage(*qimg));
        
        delete transform_data;
        delete qimg;
}

void ImageWindow::changeContrast(int value) {
        float c_value = value * 0.01;
 
        uchar* transform_data = m_image->transformImage(c_value,m_brightness->value());
        QImage *qimg = new QImage(transform_data, m_image->w(), m_image->h(), m_image->step(), QImage::Format_Grayscale8);
        m_label->setPixmap(QPixmap::fromImage(*qimg));

        delete transform_data;
        delete qimg;
}

void ImageWindow::releaseData() {
       delete m_image;
}

}
