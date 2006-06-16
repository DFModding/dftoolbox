/*  $Id: image_view.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#include <stdexcept>
#include <fstream>
#include <iostream>
#include "icons.hpp"
#include "dds.hpp"
#include "system.hpp"
#include "image_view.hpp"

FXDEFMAP(ImageView) ImageViewMap[] = {
  FXMAPFUNC(SEL_COMMAND,       ImageView::ID_ZOOM_1_1,        ImageView::onCmdZoom1_1),
  FXMAPFUNC(SEL_COMMAND,       ImageView::ID_ZOOM_IN,         ImageView::onCmdZoomIn),
  FXMAPFUNC(SEL_COMMAND,       ImageView::ID_ZOOM_OUT,        ImageView::onCmdZoomOut),
};

// Object implementation
FXIMPLEMENT(ImageView, FXVerticalFrame, ImageViewMap, ARRAYNUMBER(ImageViewMap));

#define SCALE_LIMIT 4

ImageView::ImageView()
{ 
}

ImageView::ImageView(FXComposite* parent, FXDockSite* dock)
  : FXVerticalFrame(parent, FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y,
                    0, 0, 0, 0,
                    0, 0, 0, 0),
    image(0),
    scale(0)
{
  scaled_images.resize(2*SCALE_LIMIT+1, 0);

  toolbar = new FXToolBar(dock, LAYOUT_SIDE_TOP|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_DOCK_SAME);

  new FXButton(toolbar, "\tZoom 1:1", Icon::zoom_1_1, this, ID_ZOOM_1_1, BUTTON_TOOLBAR|FRAME_RAISED);
  new FXButton(toolbar, "\tZoom in",  Icon::zoom_in,  this, ID_ZOOM_IN,  BUTTON_TOOLBAR|FRAME_RAISED);
  new FXButton(toolbar, "\tZoom out", Icon::zoom_out, this, ID_ZOOM_OUT, BUTTON_TOOLBAR|FRAME_RAISED);
  new FXHorizontalSeparator(toolbar, SEPARATOR_GROOVE);
  (new FXButton(toolbar, "\tGrayscale", Icon::grayscale, NULL, 0, BUTTON_TOOLBAR|FRAME_RAISED))->disable();
  (new FXButton(toolbar, "\tNormalmap", Icon::normalmap, NULL, 0, BUTTON_TOOLBAR|FRAME_RAISED))->disable();

  // Text view for info on file format and stuff
  text = new FXText(this, NULL, 0, LAYOUT_FILL_X);
  text->setText("Hello World");
  text->hide();

  imageview  = new FXImageView(this, NULL, NULL,0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
}

void
ImageView::set_image(const std::string& filename)
{
  cleanup();

  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    throw std::runtime_error("Error: Couldn't open " + filename);
              
  DDS dds(in);
  std::cout << "DDS: " << dds.width << "x" << dds.height << std::endl;

  // FIXME: is this ok or should we use malloc()?
  FXColor* image_data;
  FXMALLOC(&image_data, FXColor, dds.get_data().size()/4);

  memcpy(image_data,
         reinterpret_cast<const FXColor*>(&*dds.get_data().begin()),
         dds.get_data().size());
  
  image = new FXImage(getApp(), image_data,
                      IMAGE_KEEP|IMAGE_SHMI|IMAGE_SHMP|IMAGE_OWNED, dds.width, dds.height);

  image->create();
  imageview->setImage(image);
  //delete old;
  in.close(); 

  set_scale(scale);
}

void
ImageView::cleanup()
{
  for(std::vector<FXImage*>::iterator i = scaled_images.begin(); 
      i != scaled_images.end(); ++i)
    {
      delete *i;
      *i = 0;
    }
  delete image;
}

void
ImageView::set_save_image(const std::string& filename)
{
  cleanup();

  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    {
      throw std::runtime_error("Couldn't open: " + filename);
    }
  else
    {
      FXColor* buffer;
      FXMALLOC(&buffer, FXColor, 256*256);

      // Extract screenshot
      in.seekg(-(256*256*4), std::ios::end);
      in.read(reinterpret_cast<char*>(buffer), 256*256*4);
      in.close();
      
      // FIXME: Memleak
      image = new FXImage(getApp(), buffer,
                          IMAGE_KEEP|IMAGE_SHMI|IMAGE_SHMP|IMAGE_OWNED, 
                          256, 256);
      image->create();
      imageview->setImage(image);
    }

  set_scale(scale);
}

void
ImageView::set_scale(int scale_)
{
  if (scale_ >= -SCALE_LIMIT && scale_ <= SCALE_LIMIT)
    {
      float factor = powf(2, scale_);
      FXColor* data = image->getData();
      int width     = image->getWidth();
      int height    = image->getHeight();
      
      if (width*height * factor <= 1048*1048*2 && width*height*factor > 8*8)
        {
          scale = scale_;
      
          if (scaled_images[scale + SCALE_LIMIT])
            {
              imageview->setImage(scaled_images[scale + SCALE_LIMIT]);
            }
          else
            {      
              // FIXME: this is slow, but FXImageView doesn't support scaling of the view
              FXColor* newdata = new FXColor[width*height*4];
              memcpy(newdata, data, width*height*4);
              scaled_images[scale+SCALE_LIMIT] = new FXImage(getApp(), newdata,
                                                             IMAGE_SHMI|IMAGE_SHMP|IMAGE_OWNED, 
                                                             width, height);
              scaled_images[scale+SCALE_LIMIT]->scale(static_cast<int>(width*factor), 
                                                      static_cast<int>(height*factor));
              scaled_images[scale+SCALE_LIMIT]->create();
              imageview->setImage(scaled_images[scale+SCALE_LIMIT]);
            }
        }
    }
  else
    {
      std::cout << "Ignoring scale request" << std::endl;
    }  
}

long
ImageView::onCmdZoom1_1(FXObject*,FXSelector,void*)
{
  set_scale(0);
  return 1;
}

long
ImageView::onCmdZoomIn(FXObject*,FXSelector,void*)
{
  set_scale(scale+1);
  return 1;
}

long
ImageView::onCmdZoomOut(FXObject*,FXSelector,void*)
{
  set_scale(scale-1);
  return 1;
}

void
ImageView::show_toolbar(bool t)
{
  if (t)
    {
      toolbar->show();
    }
  else
    {
      toolbar->hide();
    }
}

/* EOF */
