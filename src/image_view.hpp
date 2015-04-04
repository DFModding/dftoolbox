/*  $Id: image_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_IMAGE_VIEW_HPP
#define HEADER_IMAGE_VIEW_HPP

#include "fx.h"
#include <string>

/** */
class ImageView : FXVerticalFrame
{
  FXDECLARE(ImageView);
private:
  FXToolBar*   toolbar;
  FXImageView* imageview;
  FXImage*     image;
  FXText*      text;

  int scale;
  std::vector<FXImage*>  scaled_images;

public:
  ImageView();
  ImageView(FXComposite* parent, FXDockSite* dock);

  void set_image(const std::string& filename);
  void set_save_image(const std::string& filename);

  void cleanup();
  void set_scale(int scale_);

  void show_toolbar(bool t);

  long onCmdZoom1_1(FXObject*,FXSelector,void*);
  long onCmdZoomIn(FXObject*,FXSelector,void*);
  long onCmdZoomOut(FXObject*,FXSelector,void*);

  enum {
    ID_ZOOM_IN = FXVerticalFrame::ID_LAST,
    ID_ZOOM_OUT,
    ID_ZOOM_1_1
  };
};

#endif

/* EOF */
