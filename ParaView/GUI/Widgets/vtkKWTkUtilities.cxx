/*=========================================================================

  Module:    vtkKWTkUtilities.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWTkUtilities.h"

#include "vtkKWCoreWidget.h"
#include "vtkKWApplication.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"

#include "vtkObjectFactory.h"
#include "vtkTclUtil.h"

#include "vtkWindows.h"
#include "X11/Xutil.h"

#include <vtksys/SystemTools.hxx>

// This has to be here because on HP varargs are included in 
// tcl.h and they have different prototypes for va_start so
// the build fails. Defining HAS_STDARG prevents that.
#if defined(__hpux) && !defined(HAS_STDARG)
#  define HAS_STDARG
#endif

#include "vtkTk.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWTkUtilities);
vtkCxxRevisionMacro(vtkKWTkUtilities, "1.69");

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::GetTclNameFromPointer(
  Tcl_Interp *interp,
  vtkObject *obj)
{
  if (!interp || !obj)
    {
    return NULL;
    }

  vtkTclGetObjectFromPointer(interp, (void *)obj, obj->GetClassName());
  return Tcl_GetStringResult(interp);
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::GetTclNameFromPointer(
  vtkKWApplication *app,  
  vtkObject *obj)
{
  if (!app)
    {
    return NULL;
    }
  return vtkKWTkUtilities::GetTclNameFromPointer(
    app->GetMainInterp(), obj);
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateString(
    Tcl_Interp *interp,
    const char* format, 
    ...)
{
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = vtkKWTkUtilities::EvaluateStringFromArgs(
    interp, format, var_args1, var_args2);
  va_end(var_args1);
  va_end(var_args2);
  return result;
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateString(
  vtkKWApplication *app,  
  const char* format, 
  ...)
{
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = vtkKWTkUtilities::EvaluateStringFromArgs(
    app, format, var_args1, var_args2);
  va_end(var_args1);
  va_end(var_args2);
  return result;
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateStringFromArgs(
  vtkKWApplication *app,  
  const char* format,
  va_list var_args1,
  va_list var_args2)
{
  if (!app)
    {
    return NULL;
    }
  return vtkKWTkUtilities::EvaluateStringFromArgsInternal(
    app->GetMainInterp(),
    app,
    format,
    var_args1,
    var_args2);
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateStringFromArgs(
  Tcl_Interp *interp,
  const char* format,
  va_list var_args1,
  va_list var_args2)
{
  return vtkKWTkUtilities::EvaluateStringFromArgsInternal(
    interp,
    NULL,
    format,
    var_args1,
    var_args2);
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateStringFromArgsInternal(
  Tcl_Interp *interp,
  vtkObject *obj,
  const char* format,
  va_list var_args1,
  va_list var_args2)
{
  const int buffer_on_stack_length = 1600;
  char buffer_on_stack[buffer_on_stack_length];
  char* buffer = buffer_on_stack;
  
  // Estimate the length of the result string.  Never underestimates.

  int length = vtksys::SystemTools::EstimateFormatLength(format, var_args1);
  
  // If our stack-allocated buffer is too small, allocate on one on
  // the heap that will be large enough.

  if(length > buffer_on_stack_length - 1)
    {
    buffer = new char[length + 1];
    }
  
  // Print to the buffer.

  vsprintf(buffer, format, var_args2);
  
  // Evaluate the string in Tcl.

  const char *res = 
    vtkKWTkUtilities::EvaluateSimpleStringInternal(interp, obj, buffer);
  
  // Free the buffer from the heap if we allocated it.

  if (buffer != buffer_on_stack)
    {
    delete [] buffer;
    }
  
  // Convert the Tcl result to its string representation.

  return res;
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateSimpleString(
  vtkKWApplication *app,  
  const char* str)
{
  if (!app)
    {
    return NULL;
    }
  return vtkKWTkUtilities::EvaluateSimpleStringInternal(
    app->GetMainInterp(), app, str);
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateSimpleString(
  Tcl_Interp *interp,  
  const char* str)
{
  return vtkKWTkUtilities::EvaluateSimpleStringInternal(
    interp, NULL, str);
}

//----------------------------------------------------------------------------
const char* vtkKWTkUtilities::EvaluateSimpleStringInternal(
  Tcl_Interp *interp,
  vtkObject *obj,
  const char *str)
{
  static vtksys_stl::string err;
  
  if (Tcl_GlobalEval(interp, str) != TCL_OK && obj)
    {
    err = Tcl_GetStringResult(interp); // need to save now
    vtkErrorWithObjectMacro(
      obj, "\n    Script: \n" << str
      << "\n    Returned Error on line "
      << interp->errorLine << ": \n"  
      << err.c_str() << endl);
    return err.c_str();
    }
  
  // Convert the Tcl result to its string representation.
  
  return Tcl_GetStringResult(interp);
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::GetRGBColor(Tcl_Interp *interp,
                                   const char *widget, 
                                   const char *color, 
                                   double *r, double *g, double *b)
{
  if (!interp || !widget || !color || !*color || !r || !g || !b)
    {
    return;
    }

  ostrstream command;
  command << "winfo rgb " << widget << " " << color << ends;
  if (Tcl_GlobalEval(interp, command.str()) != TCL_OK)
    {
    vtkGenericWarningMacro(
      << "Unable to get RGB color: " << Tcl_GetStringResult(interp));
    command.rdbuf()->freeze(0);     
    return;
    }
  command.rdbuf()->freeze(0);     

  int rr, gg, bb;
  if (sscanf(Tcl_GetStringResult(interp), "%d %d %d", &rr, &gg, &bb) == 3)
    {
    *r = static_cast<double>(rr) / 65535.0;
    *g = static_cast<double>(gg) / 65535.0;
    *b = static_cast<double>(bb) / 65535.0; 
    }
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::GetRGBColor(vtkKWWidget *widget, 
                                   const char *color, 
                                   double *r, double *g, double *b)
{
  if (!widget || !widget->IsCreated())
    {
    return;
    }
  
  vtkKWTkUtilities::GetRGBColor(widget->GetApplication()->GetMainInterp(),
                                widget->GetWidgetName(),
                                color,
                                r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::GetOptionColor(Tcl_Interp *interp,
                                      const char *widget,
                                      const char *option,
                                      double *r, double *g, double *b)
{
  if (!interp || !widget || !option || !r || !g || !b)
    {
    return;
    }

  ostrstream command;
  command << widget << " cget " << option << ends;
  if (Tcl_GlobalEval(interp, command.str()) != TCL_OK)
    {
    vtkGenericWarningMacro(
      << "Unable to get " << option << " option: " 
      << Tcl_GetStringResult(interp));
    command.rdbuf()->freeze(0);     
    return;
    }
  command.rdbuf()->freeze(0);     

  vtkKWTkUtilities::GetRGBColor(
    interp, widget, Tcl_GetStringResult(interp), r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::GetOptionColor(vtkKWWidget *widget, 
                                      const char *option,
                                      double *r, double *g, double *b)
{
  if (!widget || !widget->IsCreated())
    {
    return;
    }
  
  vtkKWTkUtilities::GetOptionColor(widget->GetApplication()->GetMainInterp(),
                                   widget->GetWidgetName(),
                                   option,
                                   r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWTkUtilities::GetOptionColor(vtkKWWidget *widget, 
                                         const char *option)
{
  static double rgb[3];
  vtkKWTkUtilities::GetOptionColor(widget, option, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::SetOptionColor(Tcl_Interp *interp,
                                      const char *widget,
                                      const char *option,
                                      double r, double g, double b)
{
  if (!interp || !widget || !option ||
      !(r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0))
    {
    return;
    }

  char color[10];
  sprintf(color, "#%02x%02x%02x", 
          (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));

  ostrstream command;
  command << widget << " configure " << option << " " << color << ends;
  if (Tcl_GlobalEval(interp, command.str()) != TCL_OK)
    {
    vtkGenericWarningMacro(
      << "Unable to set " << option << " option: " 
      << Tcl_GetStringResult(interp));
    }
  command.rdbuf()->freeze(0);     
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::SetOptionColor(vtkKWWidget *widget, 
                                      const char *option,
                                      double r, double g, double b)
{
  if (!widget || !widget->IsCreated())
    {
    return;
    }
  
  vtkKWTkUtilities::SetOptionColor(widget->GetApplication()->GetMainInterp(),
                                   widget->GetWidgetName(),
                                   option,
                                   r, g, b);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::QueryUserForColor(
  vtkKWApplication *app,  
  const char *dialog_parent,
  const char *dialog_title,
  double in_r, double in_g, double in_b,
  double *out_r, double *out_g, double *out_b)
{
  if (!app)
    {
    return 0;
    }

  app->RegisterDialogUp(NULL);

  int res = vtkKWTkUtilities::QueryUserForColor(
    app->GetMainInterp(), 
    dialog_parent,
    dialog_title,
    in_r, in_g, in_b,
    out_r, out_g, out_b);

  app->UnRegisterDialogUp(NULL);
  return res;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::QueryUserForColor(
  Tcl_Interp *interp,
  const char *dialog_parent,
  const char *dialog_title,
  double in_r, double in_g, double in_b,
  double *out_r, double *out_g, double *out_b)
{
  vtksys_stl::string cmd("tk_chooseColor");
  if (dialog_parent)
    {
    cmd += " -parent {";
    cmd += dialog_parent;
    cmd += "}";
    }
  if (dialog_title)
    {
    cmd += " -title {";
    cmd += dialog_title;
    cmd += "}";
    }
  
  if (in_r >= 0.0 && in_r <= 1.0 && 
      in_g >= 0.0 && in_g <= 1.0 && 
      in_b >= 0.0 && in_b <= 1.0)
    {
    char color_hex[8];
    sprintf(color_hex, "#%02x%02x%02x", 
            (int)(in_r * 255.5), (int)(in_g * 255.5), (int)(in_b * 255.5));
    cmd += " -initialcolor {";
    cmd += color_hex;
    cmd += "}";
    }

  if (Tcl_GlobalEval(interp, cmd.c_str()) != TCL_OK)
    {
    vtkGenericWarningMacro(
      << "Unable to query color: " << Tcl_GetStringResult(interp));
    return 0;
    }

  const char *result = Tcl_GetStringResult(interp);

  if (strlen(result) > 6)
    {
    char tmp[3];
    int r, g, b;
    tmp[2] = '\0';
    tmp[0] = result[1];
    tmp[1] = result[2];
    if (sscanf(tmp, "%x", &r) == 1)
      {
      tmp[0] = result[3];
      tmp[1] = result[4];
      if (sscanf(tmp, "%x", &g) == 1)
        {
        tmp[0] = result[5];
        tmp[1] = result[6];
        if (sscanf(tmp, "%x", &b) == 1)
          {
          *out_r = (double)r / 255.0;
          *out_g = (double)g / 255.0;
          *out_b = (double)b / 255.0;
          return 1;
          }
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetGeometry(Tcl_Interp *interp,
                                  const char *widget, 
                                  int *width, int *height, 
                                  int *x, int *y)
{
  if (!interp || !widget)
    {
    return 0;
    }

  vtksys_stl::string geometry;

  geometry = "winfo geometry ";
  geometry += widget;
  if (Tcl_GlobalEval(interp, geometry.c_str()) != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to query widget geometry! " << widget);
    return 0;
    }
  
  int ww, wh, wx, wy;
  if (sscanf(
        Tcl_GetStringResult(interp), "%dx%d+%d+%d", &ww, &wh, &wx, &wy) != 4)
    {
    vtkGenericWarningMacro(<< "Unable to parse geometry!");
    return 0;
    }
  
  // For some unknown reasons, "winfo geometry" can return the wrong
  // position for the window, if it is a toplevel (it will return (0, 0).
  // Check for it, and try "wm geometry" instead.

  if ((x || y) && 
      (wx == 0 && wy == 0) && 
      vtkKWTkUtilities::IsTopLevel(interp, widget))
    {
    geometry = "wm geometry ";
    geometry += widget;
    if (Tcl_GlobalEval(interp, geometry.c_str()) != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to query widget geometry! " << widget);
      return 0;
      }
    if (sscanf(
          Tcl_GetStringResult(interp), "%dx%d+%d+%d", &ww, &wh, &wx, &wy) != 4)
      {
      vtkGenericWarningMacro(<< "Unable to parse geometry!");
      return 0;
      }
    }

  if (width)
    {
    *width = ww;
    }
  if (height)
    {
    *height = wh;
    }
  if (x)
    {
    *x = wx;
    }
  if (y)
    {
    *y = wy;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetGeometry(vtkKWWidget *widget,
                                  int *width, int *height, 
                                  int *x, int *y)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }

  return vtkKWTkUtilities::GetGeometry(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    width, height, x, y);
}


//----------------------------------------------------------------------------
int vtkKWTkUtilities::ContainsCoordinates(Tcl_Interp *interp,
                                          const char *widget, 
                                          int x, int y)
{
  if (!interp || !widget)
    {
    return 0;
    }

  ostrstream geometry;
  geometry << "concat [winfo width " << widget << "] [winfo height "
           << widget << "] [winfo rootx " << widget << "] [winfo rooty "
           << widget << "]" << ends;
  int res = Tcl_GlobalEval(interp, geometry.str());
  geometry.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to query widget geometry! " << widget);
    return 0;
    }
  
  int ww, wh, wx, wy;
  if (sscanf(
        Tcl_GetStringResult(interp), "%d %d %d %d", &ww, &wh, &wx, &wy) != 4)
    {
    return 0;
    }

  return (x >= wx && x < (wx + ww) && y >= wy && y < (wy + wh)) ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ContainsCoordinates(vtkKWWidget *widget,
                                          int x, int y)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }

  return vtkKWTkUtilities::ContainsCoordinates(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    x, y);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::UpdatePhoto(Tcl_Interp *interp,
                                  const char *photo_name,
                                  const unsigned char *pixels, 
                                  int width, int height,
                                  int pixel_size,
                                  unsigned long buffer_length,
                                  const char *blend_with_name,
                                  const char *blend_color_option,
                                  int update_options)
{
  // Check params

  if (!interp)
    {
    vtkGenericWarningMacro(<< "Empty interpreter");
    return 0;
    }

  if (!photo_name || !photo_name[0])
    {
    vtkGenericWarningMacro(<< "Empty photo name");
    return 0;
    }

  if (!pixels)
    {
    vtkGenericWarningMacro(<< "No pixel data");
    return 0;
    }

  if (width <= 0 || height <= 0)
    {
    vtkGenericWarningMacro(<< "Invalid size: " << width << "x" << height);
    return 0;
    }

  if (pixel_size != 3 && pixel_size != 4)
    {
    vtkGenericWarningMacro(<< "Unsupported pixel size: " << pixel_size);
    return 0;
    }

  // Find the photo (create it if not found)

  Tk_PhotoHandle photo = Tk_FindPhoto(interp, const_cast<char *>(photo_name));

  if (!photo)
    {
    ostrstream create_photo;
    create_photo << "image create photo " << photo_name << ends;
    int res = Tcl_GlobalEval(interp, create_photo.str());
    create_photo.rdbuf()->freeze(0);
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(
        << "Unable to create photo " << photo_name << ": " 
        << Tcl_GetStringResult(interp));
      return 0;
      }

    photo = Tk_FindPhoto(interp, const_cast<char *>(photo_name));
    if (!photo)
      {
      vtkGenericWarningMacro(<< "Error looking up Tk photo:" << photo_name);
      return 0;
      }
    }

  Tk_PhotoSetSize(photo, width, height);

#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 4)
  Tk_PhotoBlank(photo);
#endif

  unsigned long nb_of_raw_bytes = width * height * pixel_size;

  // Is the data encoded (zlib and/or base64) ?
  
  unsigned char *decoded_data = NULL;
  if (buffer_length && buffer_length != nb_of_raw_bytes)
    {
    if (!vtkKWResourceUtilities::DecodeBuffer(
          pixels, buffer_length, &decoded_data, nb_of_raw_bytes))
      {
      vtkGenericWarningMacro(
        << "Error while decoding pixels for photo:" << photo_name);
      return 0;
      }
    pixels = decoded_data;
    }

  // Tk does not seem to support transparency blending, only 
  // Set block struct

  Tk_PhotoImageBlock sblock;

  sblock.width     = width;
  sblock.height    = height;
  sblock.offset[0] = 0;
  sblock.offset[1] = 1;
  sblock.offset[2] = 2;
  sblock.offset[3] = 3;

  sblock.pixelSize = 3;
  sblock.pitch     = sblock.width * sblock.pixelSize;
  unsigned long sblock_size = sblock.pitch * sblock.height;

  unsigned char *pp = NULL;

  if (pixel_size <= 3)
    {
    sblock.pixelPtr = const_cast<unsigned char *>(pixels);
    }
  else 
    {
#if 0
    sblock.pixelPtr = const_cast<unsigned char *>(pixels);

    sblock.pixelSize = 4;
    sblock.pitch     = sblock.width * sblock.pixelSize;
    unsigned long sblock_size = sblock.pitch * sblock.height;

#else
    pp = sblock.pixelPtr = new unsigned char[sblock_size];

    // At the moment let's not use the alpha layer inside the photo but 
    // blend with the current background color

    double r, g, b;
    if (blend_with_name && !blend_color_option)
      {
      blend_color_option = "-background";
      }
    if (blend_color_option || blend_with_name)
      {
      // If it is not an option, interpret it as a color right away
      // otherwise retrieve that option color from the widget to blend
      // it against (blend_with_name)

      if (blend_color_option && *blend_color_option != '-')
        {
        vtkKWTkUtilities::GetRGBColor(
          interp, 
          blend_with_name ? blend_with_name : ".", 
          blend_color_option, &r, &g, &b);
        }
      else
        {
        vtkKWTkUtilities::GetOptionColor(
          interp, 
          blend_with_name ? blend_with_name : ".", 
          blend_color_option, &r, &g, &b);
        }
      }
    else
      {
      vtkKWTkUtilities::GetOptionColor(interp, ".", "-background", &r, &g, &b);
      }
    
    r *= 255.0;
    g *= 255.0;
    b *= 255.0;

    // Create photo pixels

    int xx, yy;
    for (yy=0; yy < height; yy++)
      {
      for (xx=0; xx < width; xx++)
        {
        double alpha = static_cast<float>(*(pixels + 3)) / 255.0;
        
        *(pp)     = static_cast<int>(r * (1 - alpha) + *(pixels)    * alpha);
        *(pp + 1) = static_cast<int>(g * (1 - alpha) + *(pixels+ 1) * alpha);
        *(pp + 2) = static_cast<int>(b * (1 - alpha) + *(pixels+ 2) * alpha);
        
        pixels += pixel_size;
        pp += 3;
        }
      }

    pp = sblock.pixelPtr;
#endif
    }
  
  if (update_options & vtkKWTkUtilities::UpdatePhotoOptionFlipVertical)
    {
    sblock.pitch = -sblock.pitch;
    sblock.pixelPtr += sblock_size + sblock.pitch;
    }

#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 4) && !defined(USE_COMPOSITELESS_PHOTO_PUT_BLOCK)
  Tk_PhotoPutBlock(photo, &sblock, 0, 0, width, height, TK_PHOTO_COMPOSITE_SET);
#else
  Tk_PhotoPutBlock(photo, &sblock, 0, 0, width, height);
#endif

  // Free mem

  if (pp)
    {
    delete [] pp;
    }

  if (decoded_data)
    {
    delete [] decoded_data;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::UpdatePhoto(vtkKWApplication *app,
                                  const char *photo_name,
                                  const unsigned char *pixels, 
                                  int width, int height,
                                  int pixel_size,
                                  unsigned long buffer_length,
                                  const char *blend_with_name,
                                  const char *blend_color_option,
                                  int update_options)
{
  if (!app)
    {
    return 0;
    }
  return vtkKWTkUtilities::UpdatePhoto(
    app->GetMainInterp(),
    photo_name,
    pixels, 
    width, height,
    pixel_size,
    buffer_length,
    blend_with_name,
    blend_color_option,
    update_options);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::UpdatePhotoFromIcon(vtkKWApplication *app,
                                          const char *photo_name,
                                          vtkKWIcon *icon,
                                          const char *blend_with_name,
                                          const char *blend_color_option,
                                          int update_options)
{
  if (!app || !icon)
    {
    return 0;
    }
  return vtkKWTkUtilities::UpdatePhoto(
    app->GetMainInterp(),
    photo_name,
    icon->GetData(), 
    icon->GetWidth(), icon->GetHeight(), 
    icon->GetPixelSize(),
    0,
    blend_with_name,
    blend_color_option,
    update_options);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::UpdatePhotoFromPredefinedIcon(
  vtkKWApplication *app,
  const char *photo_name,
  int icon_index,
  const char *blend_with_name,
  const char *blend_color_option,
  int update_options)
{
  if (!app)
    {
    return 0;
    }
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  int res = vtkKWTkUtilities::UpdatePhotoFromIcon(
    app,
    photo_name,
    icon,
    blend_with_name,
    blend_color_option,
    update_options);
  icon->Delete();
  return res;
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::SetImageOptionToPixels(
  vtkKWCoreWidget *widget,
  const unsigned char* pixels, 
  int width, 
  int height,
  int pixel_size,
  unsigned long buffer_length,
  const char *blend_color_option,
  const char *image_option)
{
  if (!widget->IsCreated())
    {
    vtkWarningWithObjectMacro(widget, "Widget is not created yet !");
    return;
    }

  if (!image_option || !*image_option)
    {
    image_option = "-image";
    }

  if (!widget->HasConfigurationOption(image_option))
    {
    return;
    }

  vtksys_stl::string image_name(widget->GetWidgetName());
  image_name += ".";
  image_name += &image_option[1];

#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 4)
  // This work-around is put here to "fix" what looks like a bug
  // in Tk. Without this, there seems to be some weird problems
  // with Tk picking some alpha values for some colors.
  widget->Script("catch {destroy %s}", image_name.c_str());
#endif
  
  if (!vtkKWTkUtilities::UpdatePhoto(widget->GetApplication(),
                                     image_name.c_str(),
                                     pixels, 
                                     width, height, pixel_size,
                                     buffer_length,
                                     widget->GetWidgetName(),
                                     blend_color_option))
    {
    vtkWarningWithObjectMacro(
      widget, "Error updating Tk photo " << image_name.c_str());
    return;
    }

  widget->SetConfigurationOption(image_option, image_name.c_str());
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::UpdateOrLoadPhoto(Tcl_Interp *interp,
                                        const char *photo_name,
                                        const char *file_name,
                                        const char *directory,
                                        const unsigned char *pixels, 
                                        int width, int height,
                                        int pixel_size,
                                        unsigned long buffer_length,
                                        const char *blend_with_name,
                                        const char *blend_color_option)
{
  // Try to find a PNG file with the same name in directory 
  // or directory/Resources

  unsigned char *png_buffer = NULL;

  if (directory && file_name)
    {
    char buffer[1024];
    sprintf(buffer, "%s/%s.png", directory, file_name);
    int found = vtksys::SystemTools::FileExists(buffer);
    if (!found)
      {
      sprintf(buffer, "%s/Resources/%s.png", directory, file_name);
      found = vtksys::SystemTools::FileExists(buffer);
      }
    if (found && 
        vtkKWResourceUtilities::ReadPNGImage(
          buffer, &width, &height, &pixel_size, &png_buffer))
      {
      pixels = png_buffer;
      buffer_length = 0;
      }
    }

  // Otherwise use the provided data

  int res = vtkKWTkUtilities::UpdatePhoto(
    interp,
    (photo_name ? photo_name : file_name), 
    pixels, 
    width, height,
    pixel_size,
    buffer_length,
    blend_with_name,
    blend_color_option);
  
  if (png_buffer)
    {
    delete [] png_buffer;
    }

  return res;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::UpdateOrLoadPhoto(vtkKWApplication *app,
                                        const char *photo_name,
                                        const char *file_name,
                                        const char *directory,
                                        const unsigned char *pixels, 
                                        int width, int height,
                                        int pixel_size,
                                        unsigned long buffer_length,
                                        const char *blend_with_name,
                                        const char *blend_color_option)
{
  if (!app)
    {
    return 0;
    }
  return vtkKWTkUtilities::UpdateOrLoadPhoto(
    app->GetMainInterp(),
    photo_name,
    file_name,
    directory,
    pixels, 
    width, height,
    pixel_size,
    buffer_length,
    blend_with_name,
    blend_color_option);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::FindPhoto(Tcl_Interp *interp,
                                const char *photo_name)
{
  return Tk_FindPhoto(interp,const_cast<char *>(photo_name)) ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::FindPhoto(vtkKWApplication *app,
                                     const char *photo_name)
{
  if (!app)
    {
    return 0;
    }
  return vtkKWTkUtilities::FindPhoto(app->GetMainInterp(), photo_name);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetPhotoHeight(Tcl_Interp *interp,
                                     const char *photo_name)
{
  // Find the photo

  Tk_PhotoHandle photo = Tk_FindPhoto(interp,
                                      const_cast<char *>(photo_name));
  if (!photo)
    {
    vtkGenericWarningMacro(<< "Error looking up Tk photo:" << photo_name);
    return 0;
    }  

  // Return height

  int width, height;
  Tk_PhotoGetSize(photo, &width, &height);
  return height;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetPhotoHeight(vtkKWApplication *app,
                                     const char *photo_name)
{
  if (!app)
    {
    return 0;
    }
  return vtkKWTkUtilities::GetPhotoHeight(
    app->GetMainInterp(), photo_name);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetPhotoHeight(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }

  Tcl_Interp *interp = widget->GetApplication()->GetMainInterp();

  // Retrieve -image option

  vtksys_stl::string cmd;
  cmd += widget->GetWidgetName();
  cmd += " cget -image";
  
  if (Tcl_GlobalEval(interp, cmd.c_str()) != TCL_OK)
    {
    vtkGenericWarningMacro(
      << "Unable to get -image option: " << Tcl_GetStringResult(interp));
    return 0;
    }

  // No -image ?

  const char *result = Tcl_GetStringResult(interp);
  if (!result || !*result)
    {
    return 0;
    }

  // Get size

  vtksys_stl::string image_name(result);
  return vtkKWTkUtilities::GetPhotoHeight(interp, image_name.c_str());
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetPhotoWidth(Tcl_Interp *interp,
                                    const char *photo_name)
{
  // Find the photo

  Tk_PhotoHandle photo = Tk_FindPhoto(interp,
                                      const_cast<char *>(photo_name));
  if (!photo)
    {
    vtkGenericWarningMacro(<< "Error looking up Tk photo:" << photo_name);
    return 0;
    }  

  // Return width

  int width, height;
  Tk_PhotoGetSize(photo, &width, &height);
  return width;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetPhotoWidth(vtkKWApplication *app,
                                    const char *photo_name)
{
  if (!app)
    {
    return 0;
    }
  return vtkKWTkUtilities::GetPhotoWidth(
    app->GetMainInterp(), photo_name);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeight(Tcl_Interp *interp,
                                      const char *font, 
                                      char *new_font, 
                                      int weight)
{
  int res;

  // First try to modify the old -foundry-family-weigth-*-*-... form
  // Catch the weight field, replace it with bold or medium.

  ostrstream regsub;
  regsub << "regsub -- {(-[^-]*\\S-[^-]*\\S-)([^-]*)(-.*)} \""
         << font << "\" {\\1" << (weight ? "bold" : "medium") 
         << "\\3} __temp__" << ends;

  res = Tcl_GlobalEval(interp, regsub.str());
  regsub.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to regsub!");
    return 0;
    }
  if (atoi(Tcl_GetStringResult(interp)) == 1)
    {
    res = Tcl_GlobalEval(interp, "set __temp__");
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to replace result of regsub! ("
                             << Tcl_GetStringResult(interp) << ")");
      return 0;
      }
    strcpy(new_font, Tcl_GetStringResult(interp));
    return 1;
    }

  // Otherwise replace the -weight parameter with either bold or normal

  ostrstream regsub2;
  regsub2 << "regsub -- {(.* -weight )(\\w*\\M)(.*)} [font actual \""
          << font << "\"] {\\1" << (weight ? "bold" : "normal") 
          << "\\3} __temp__" << ends;
  res = Tcl_GlobalEval(interp, regsub2.str());
  regsub2.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to regsub (2)!");
    return 0;
    }
  if (atoi(Tcl_GetStringResult(interp)) == 1)
    {
    res = Tcl_GlobalEval(interp, "set __temp__");
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to replace result of regsub! (2) ("
                             << Tcl_GetStringResult(interp) << ")");
      return 0;
      }
    strcpy(new_font, Tcl_GetStringResult(interp));
    return 1;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeightToBold(Tcl_Interp *interp,
                                             const char *font, 
                                             char *new_font)
{
  return vtkKWTkUtilities::ChangeFontWeight(interp, font, new_font, 1);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeightToNormal(Tcl_Interp *interp,
                                               const char *font, 
                                               char *new_font)
{
  return vtkKWTkUtilities::ChangeFontWeight(interp, font, new_font, 0);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeight(Tcl_Interp *interp,
                                       const char *widget,
                                       int weight)
{
  char font[1024], new_font[1024];
  
  int res;

  // Get the font

  ostrstream getfont;
  getfont << widget << " cget -font" << ends;
  res = Tcl_GlobalEval(interp, getfont.str());
  getfont.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to getfont!");
    return 0;
    }
  strcpy(font, Tcl_GetStringResult(interp));

  // Change the font weight

  if (!vtkKWTkUtilities::ChangeFontWeight(interp, font, new_font, weight))
    {
    return 0;
    }

  // Set the font

  ostrstream setfont;
  setfont << widget << " configure -font \"" << new_font << "\"" << ends;
  res = Tcl_GlobalEval(interp, setfont.str());
  setfont.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to replace font ! ("
                           << Tcl_GetStringResult(interp) << ")");
    return 0;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeightToBold(Tcl_Interp *interp,
                                             const char *widget)
{
  return vtkKWTkUtilities::ChangeFontWeight(interp, widget, 1);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeightToBold(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::ChangeFontWeightToBold(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeightToNormal(Tcl_Interp *interp,
                                               const char *widget)
{
  return vtkKWTkUtilities::ChangeFontWeight(interp, widget, 0);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontWeightToNormal(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::ChangeFontWeightToNormal(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlant(Tcl_Interp *interp,
                                      const char *font, 
                                      char *new_font, 
                                      int slant)
{
  int res;

  // First try to modify the old -foundry-family-weigth-slant-*-*-... form
  // Catch the slant field, replace it with i (italic) or r (roman).

  ostrstream regsub;
  regsub << "regsub -- {(-[^-]*\\S-[^-]*\\S-[^-]*\\S-)([^-]*)(-.*)} \""
         << font << "\" {\\1" << (slant ? "i" : "r") << "\\3} __temp__" 
         << ends;

  res = Tcl_GlobalEval(interp, regsub.str());
  regsub.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to regsub!");
    return 0;
    }
  if (atoi(Tcl_GetStringResult(interp)) == 1)
    {
    res = Tcl_GlobalEval(interp, "set __temp__");
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to replace result of regsub! ("
                             << Tcl_GetStringResult(interp) << ")");
      return 0;
      }
    strcpy(new_font, Tcl_GetStringResult(interp));
    return 1;
    }

  // Otherwise replace the -slant parameter with either bold or normal

  ostrstream regsub2;
  regsub2 << "regsub -- {(.* -slant )(\\w*\\M)(.*)} [font actual \"" 
          << font << "\"] {\\1" << (slant ? "italic" : "roman") 
          << "\\3} __temp__" << ends;
  res = Tcl_GlobalEval(interp, regsub2.str());
  regsub2.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to regsub (2)!");
    return 0;
    }
  if (atoi(Tcl_GetStringResult(interp)) == 1)
    {
    res = Tcl_GlobalEval(interp, "set __temp__");
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to replace result of regsub! (2) ("
                             << Tcl_GetStringResult(interp) << ")");
      return 0;
      }
    strcpy(new_font, Tcl_GetStringResult(interp));
    return 1;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlantToItalic(Tcl_Interp *interp,
                                              const char *font, 
                                              char *new_font)
{
  return vtkKWTkUtilities::ChangeFontSlant(interp, font, new_font, 1);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlantToRoman(Tcl_Interp *interp,
                                             const char *font, 
                                             char *new_font)
{
  return vtkKWTkUtilities::ChangeFontSlant(interp, font, new_font, 0);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlant(Tcl_Interp *interp,
                                      const char *widget,
                                      int slant)
{
  char font[1024], new_font[1024];
  
  int res;

  // Get the font

  ostrstream getfont;
  getfont << widget << " cget -font" << ends;
  res = Tcl_GlobalEval(interp, getfont.str());
  getfont.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to getfont!");
    return 0;
    }
  strcpy(font, Tcl_GetStringResult(interp));

  // Change the font slant

  if (!vtkKWTkUtilities::ChangeFontSlant(interp, font, new_font, slant))
    {
    return 0;
    }

  // Set the font

  ostrstream setfont;
  setfont << widget << " configure -font \"" << new_font << "\"" << ends;
  res = Tcl_GlobalEval(interp, setfont.str());
  setfont.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to replace font ! ("
                           << Tcl_GetStringResult(interp) << ")");
    return 0;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlantToItalic(Tcl_Interp *interp,
                                              const char *widget)
{
  return vtkKWTkUtilities::ChangeFontSlant(interp, widget, 1);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlantToItalic(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::ChangeFontSlantToItalic(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlantToRoman(Tcl_Interp *interp,
                                             const char *widget)
{
  return vtkKWTkUtilities::ChangeFontSlant(interp, widget, 0);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::ChangeFontSlantToRoman(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::ChangeFontSlantToRoman(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetGridSize(Tcl_Interp *interp,
                                  const char *widget,
                                  int *nb_of_cols,
                                  int *nb_of_rows)
{
  ostrstream size;
  size << "grid size " << widget << ends;
  int res = Tcl_GlobalEval(interp, size.str());
  size.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to query grid size!");
    return 0;
    }
  if (sscanf(
        Tcl_GetStringResult(interp), "%d %d", nb_of_cols, nb_of_rows) != 2)
    {
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetGridSize(vtkKWWidget *widget, 
                                   int *nb_of_cols,
                                   int *nb_of_rows)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::GetGridSize(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    nb_of_cols, nb_of_rows);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetWidgetPositionInGrid(Tcl_Interp *interp,
                                      const char *widget,
                                      int *col,
                                      int *row)
{
  ostrstream info;
  info << "grid info " << widget << ends;
  int res = Tcl_GlobalEval(interp, info.str());
  info.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to query grid info!");
    return 0;
    }
  
  const char *pos;
  int ok = 1;

  const char *result = Tcl_GetStringResult(interp);

  pos = strstr(result, "-column ");
  if (pos)
    {
    if (sscanf(pos, "-column %d", col) != 1)
      {
      ok = 0;
      }
    }

  pos = strstr(result, "-row ");
  if (pos)
    {
    if (sscanf(pos, "-row %d", row) != 1)
      {
      ok = 0;
      }
    }

  return ok;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetWidgetPositionInGrid(vtkKWWidget *widget, 
                                       int *col,
                                       int *row)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::GetWidgetPositionInGrid(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    col, row);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetWidgetPaddingInPack(Tcl_Interp *interp,
                                          const char *widget,
                                          int *ipadx,
                                          int *ipady,
                                          int *padx,
                                          int *pady)
{
  ostrstream packinfo;
  packinfo << "pack info " << widget << ends;
  int res = Tcl_GlobalEval(interp, packinfo.str());
  packinfo.rdbuf()->freeze(0);
  const char *result = Tcl_GetStringResult(interp);
  if (res != TCL_OK || !result || !result[0])
    {
    vtkGenericWarningMacro(<< "Unable to get pack info!");
    return 0;
    }
  
  // Parse (ex: -ipadx 0 -ipady 0 -padx 0 -pady 0)

  int ok = 1;

  const char *ptr;
  if (ipadx)
    {
    ptr = strstr(result, "-ipadx ");
    if (ptr)
      {
      if (sscanf(ptr + 7, "%d", ipadx) != 1)
        {
        ok = 0;
        }
      }
    }

  if (ipady)
    {
    ptr = strstr(result, "-ipady ");
    if (ptr)
      {
      if (sscanf(ptr + 7, "%d", ipady) != 1)
        {
        ok = 0;
        }
      }
    }

  if (padx)
    {
    ptr = strstr(result, "-padx ");
    if (ptr)
      {
      if (sscanf(ptr + 6, "%d", padx) != 1)
        {
        ok = 0;
        }
      }
    }

  if (pady)
    {
    ptr = strstr(result, "-pady ");
    if (ptr)
      {
      if (sscanf(ptr + 6, "%d", pady) != 1)
        {
        ok = 0;
        }
      }
    }

  return ok;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetMasterInPack(Tcl_Interp *interp,
                                     const char *widget,
                                     ostream &in)
{
  ostrstream packinfo;
  packinfo << "pack info " << widget << ends;
  int res = Tcl_GlobalEval(interp, packinfo.str());
  packinfo.rdbuf()->freeze(0);
  const char *result = Tcl_GetStringResult(interp);
  if (res != TCL_OK || !result || !result[0])
    {
    vtkGenericWarningMacro(<< "Unable to get pack info!");
    return 0;
    }
  
  // Parse for -in

  const char *pack_in = strstr(result, "-in ");
  if (!pack_in)
    {
    return 0;
    }

  pack_in += 4;
  const char *pack_in_end = strchr(pack_in, ' ');

  if (pack_in_end)
    {
    char *pack_in_buffer = new char [strlen(pack_in) + 1];
    strncpy(pack_in_buffer, pack_in, pack_in_end - pack_in);
    pack_in_buffer[pack_in_end - pack_in] = '\0';
    in << pack_in_buffer;
    delete [] pack_in_buffer;
    }
  else
    {
    in << pack_in;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetMasterInPack(vtkKWWidget *widget, 
                                      ostream &in)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }

  return vtkKWTkUtilities::GetMasterInPack(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    in);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetSlavesBoundingBoxInPack(Tcl_Interp *interp,
                                        const char *widget,
                                        int *width,
                                        int *height)
{
  ostrstream slaves;
  slaves << "pack slaves " << widget << ends;
  int res = Tcl_GlobalEval(interp, slaves.str());
  slaves.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to get pack slaves!");
    return 0;
    }
  
  // No slaves
  
  const char *result = Tcl_GetStringResult(interp);
  if (!result || !result[0])
    {
    return 1;
    }
  
  // Browse each slave for reqwidth, reqheight

  int buffer_length = strlen(result);
  char *buffer = new char [buffer_length + 1];
  strcpy(buffer, result);

  char *buffer_end = buffer + buffer_length;
  char *ptr = buffer, *word_end;

  while (ptr < buffer_end)
    {
    // Get the slave name

    word_end = strchr(ptr + 1, ' ');
    if (word_end == NULL)
      {
      word_end = buffer_end;
      }
    else
      {
      *word_end = 0;
      }

    // Get width / height

    ostrstream geometry;
    geometry << "concat [winfo reqwidth " << ptr << "] [winfo reqheight " 
             << ptr << "]"<< ends;
    res = Tcl_GlobalEval(interp, geometry.str());
    geometry.rdbuf()->freeze(0);
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to query slave geometry!");
      }
    else
      {
      int w, h;
      sscanf(Tcl_GetStringResult(interp), "%d %d", &w, &h);

      // If w == h == 1 then again it might not have been packed, so call
      // recursively

      if (w == 1 && h == 1)
        {
        vtkKWTkUtilities::GetSlavesBoundingBoxInPack(interp, ptr, &w, &h);
        }

      // Don't forget the padding

      int ipadx = 0, ipady = 0, padx = 0, pady = 0;
      vtkKWTkUtilities::GetWidgetPaddingInPack(interp, ptr, 
                                            &ipadx, &ipady, &padx, &pady);

      w += 2 * (padx + ipadx);
      h += 2 * (pady + ipady);

      if (w > *width)
        {
        *width = w;
        }
      if (h > *height)
        {
        *height = h;
        }
      }
    
    ptr = word_end + 1;
    }

  delete [] buffer;

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetSlavesBoundingBoxInPack(vtkKWWidget *widget,
                                        int *width,
                                        int *height)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }

  return vtkKWTkUtilities::GetSlavesBoundingBoxInPack(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    width, height);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetSlaveHorizontalPositionInPack(Tcl_Interp *interp,
                                                     const char *widget,
                                                     const char *slave,
                                                     int *x)
{
  ostrstream slaves;
  slaves << "pack slaves " << widget << ends;
  int res = Tcl_GlobalEval(interp, slaves.str());
  slaves.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to get pack slaves!");
    return 0;
    }
  
  // No slaves
  
  const char *result = Tcl_GetStringResult(interp);
  if (!result || !result[0])
    {
    vtkGenericWarningMacro(<< "Unable to find slaves!");
    return 0;
    }
  
  // Browse each slave until the right one if found

  int buffer_length = strlen(result);
  char *buffer = new char [buffer_length + 1];
  strcpy(buffer, result);

  char *buffer_end = buffer + buffer_length;
  char *ptr = buffer, *word_end;

  int pos = 0;

  while (ptr < buffer_end)
    {
    // Get the slave name

    word_end = strchr(ptr + 1, ' ');
    if (word_end == NULL)
      {
      word_end = buffer_end;
      }
    else
      {
      *word_end = 0;
      }

    // If slave found, add one padx and leave
    
    if (!strcmp(ptr, slave))
      {
      int padx = 0;
      vtkKWTkUtilities::GetWidgetPaddingInPack(interp, ptr, 0, 0, &padx, 0);
      pos += padx;
      break;
      }

    // Get width

    ostrstream geometry;
    geometry << "winfo reqwidth " << ptr << ends;
    res = Tcl_GlobalEval(interp, geometry.str());
    geometry.rdbuf()->freeze(0);
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to query slave geometry!");
      }
    else
      {
      int w = atoi(Tcl_GetStringResult(interp));
      
      // If w == 1 then again it might not have been packed, so get bbox

      if (w == 1)
        {
        int h = 0;
        vtkKWTkUtilities::GetSlavesBoundingBoxInPack(interp, ptr, &w, &h);
        }

      // Don't forget the padding

      int ipadx = 0, padx = 0;
      vtkKWTkUtilities::GetWidgetPaddingInPack(interp, ptr, &ipadx, 0, &padx, 0);
      
      pos += w + 2 * (padx + ipadx);
      }
    
    ptr = word_end + 1;
    }

  delete [] buffer;

  *x = pos;

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetSlaveHorizontalPositionInPack(vtkKWWidget *widget, 
                                                     vtkKWWidget *slave, 
                                                     int *x)
{
  if (!widget || !widget->IsCreated() ||
      !slave || !slave->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::GetSlaveHorizontalPositionInPack(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    slave->GetWidgetName(),
    x);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetGridColumnWidths(Tcl_Interp *interp,
                                          const char *widget,
                                          int *nb_of_cols,
                                          int **col_widths,
                                          int allocate)
{
  // First get grid size

  int nb_of_rows;
  if (!vtkKWTkUtilities::GetGridSize(interp, widget, nb_of_cols, &nb_of_rows))
    {
    vtkGenericWarningMacro(<< "Unable to query grid size!");
    return 0;
    }

  // Iterate over the columns and get the largest widget
  // (I'm expecting only one widget per cell here)

  if (allocate)
    {
    *col_widths = new int[*nb_of_cols];
    }

  int col, row;
  for (col = 0; col < *nb_of_cols; col++)
    {
    (*col_widths)[col] = 0;
    for (row = 0; row < nb_of_rows; row++)
      {
      // Get the slave

      ostrstream slave;
      slave << "grid slaves " << widget << " -column " << col 
            << " -row " << row << ends;
      int res = Tcl_GlobalEval(interp, slave.str());
      slave.rdbuf()->freeze(0);
      if (res != TCL_OK)
        {
        vtkGenericWarningMacro(<< "Unable to get grid slave!");
        continue;
        }

      // No slave, let's process the next row

      const char *result = Tcl_GetStringResult(interp);
      if (!result || !result[0])
        {
        continue;
        }

      // Get the slave reqwidth

      ostrstream reqwidth;
      reqwidth << "winfo reqwidth " << result << ends;
      res = Tcl_GlobalEval(interp, reqwidth.str());
      reqwidth.rdbuf()->freeze(0);
      if (res != TCL_OK)
        {
        vtkGenericWarningMacro(<< "Unable to query slave width!");
        continue;
        }
      int width = 0;
      if (sscanf(Tcl_GetStringResult(interp), "%d", &width) == 1 && 
          width > (*col_widths)[col])
        {
        (*col_widths)[col] = width;
        }
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::SynchroniseGridsColumnMinimumSize(
  Tcl_Interp *interp,
  int nb_of_widgets,
  const char **widgets,
  const float *factors,
  const int *weights)
{
  // Allocate mem for nb of colums and widths

  int *nb_of_cols = new int [nb_of_widgets];
  int **col_widths = new int* [nb_of_widgets];
  int widget;

  // Collect column widths

  int min_nb_of_cols = 10000;
  for (widget = 0; widget < nb_of_widgets; widget++)
    {
    if (!vtkKWTkUtilities::GetGridColumnWidths(
      interp, widgets[widget], &nb_of_cols[widget], &col_widths[widget], 1))
      {
      nb_of_cols[widget] = 0;
      col_widths[widget] = NULL;
      }
    if (nb_of_cols[widget] < min_nb_of_cols)
      {
      min_nb_of_cols = nb_of_cols[widget];
      }
    }

  // Synchronize columns (for each column, configure -minsize to the largest
  // column width for all grids)

  ostrstream minsize;
  for (int col = 0; col < min_nb_of_cols; col++)
    {
    int col_width_max = 0;
    for (widget = 0; widget < nb_of_widgets; widget++)
      {
      if (col_widths[widget][col] > col_width_max)
        {
        col_width_max = col_widths[widget][col];
        }
      }
    if (factors)
      {
      col_width_max = (int)((float)col_width_max * factors[col]);
      }
    for (widget = 0; widget < nb_of_widgets; widget++)
      {
      minsize << "grid columnconfigure " << widgets[widget] << " " << col 
              << " -minsize " << col_width_max;
      if (weights)
        {
        minsize << " -weight " << weights[col];
        }
      minsize << endl;
      }
    }
  minsize << ends;

  int ok = 1;
  if (Tcl_GlobalEval(interp, minsize.str()) != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to synchronize grid columns!");
    ok = 0;
    }
  minsize.rdbuf()->freeze(0);

  // Free mem

  delete [] nb_of_cols;
  for (widget = 0; widget < nb_of_widgets; widget++)
    {
    delete [] col_widths[widget];
    }
  delete [] col_widths;

  return ok;
}


//----------------------------------------------------------------------------
int vtkKWTkUtilities::SynchroniseLabelsMaximumWidth(
  Tcl_Interp *interp,
  int nb_of_widgets,
  const char **widgets,
  const char *options)
{
  // Get the maximum width

  int width, length, maxwidth = 0;

  int widget;
  for (widget = 0; widget < nb_of_widgets; widget++)
    {
    // Get the -width

    ostrstream getwidth;
    getwidth << widgets[widget] << " cget -width" << ends;
    int res = Tcl_GlobalEval(interp, getwidth.str());
    getwidth.rdbuf()->freeze(0);
    const char *result = Tcl_GetStringResult(interp);
    if (res != TCL_OK || !result || !result[0])
      {
      vtkGenericWarningMacro(<< "Unable to get label -width! " 
                             <<result);
      continue;
      }
    width = atoi(result);

    // Get the -text length

    ostrstream getlength;
    getlength << widgets[widget] << " cget -text" << ends;
    res = Tcl_GlobalEval(interp, getlength.str());
    getlength.rdbuf()->freeze(0);
    if (res != TCL_OK)
      {
      vtkGenericWarningMacro(<< "Unable to get label -text! " 
                             << Tcl_GetStringResult(interp));
      continue;
      }
    result = Tcl_GetStringResult(interp);
    length = result ? strlen(result) : 0;

    // Store the max

    if (width > maxwidth)
      {
      maxwidth = width;
      }
    if (length > maxwidth)
      {
      maxwidth = length;
      }
    }

  // Synchronize labels

  ostrstream setwidth;
  for (widget = 0; widget < nb_of_widgets; widget++)
    {
    setwidth << widgets[widget] << " configure -width " << maxwidth;
    if (options)
      {
      setwidth << " " << options;
      }
    setwidth << endl;
    }
  setwidth << ends;
  int res = Tcl_GlobalEval(interp, setwidth.str());
  setwidth.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to synchronize labels width! " 
                           << Tcl_GetStringResult(interp));
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::SynchroniseLabelsMaximumWidth(vtkKWApplication *app,
                                                    int nb_of_widgets,
                                                    const char **widgets,
                                                    const char *options)
{
  if (!app)
    {
    return 0;
    }
  return vtkKWTkUtilities::SynchroniseLabelsMaximumWidth(
    app->GetMainInterp(), nb_of_widgets, widgets, options);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetSlavesInPack(
  Tcl_Interp *interp,
  const char *widget,
  char ***slaves)
{
  int res;

  // Get number of slaves

  ostrstream nb_slaves_str;
  nb_slaves_str << "llength [pack slaves " << widget << "]" << ends;
  res = Tcl_GlobalEval(interp, nb_slaves_str.str());
  nb_slaves_str.rdbuf()->freeze(0);
  const char *result = Tcl_GetStringResult(interp);
  if (res != TCL_OK || !result || !result[0])
    {
    vtkGenericWarningMacro(<< "Unable to get number of packed slaves!");
    return 0;
    }

  int nb_slaves = atoi(result);
  if (!nb_slaves)
    {
    return 0;
    }

  // Get the slaves as a space-separated list

  ostrstream slaves_str;
  slaves_str << "pack slaves " << widget << ends;
  res = Tcl_GlobalEval(interp, slaves_str.str());
  slaves_str.rdbuf()->freeze(0);
  result = Tcl_GetStringResult(interp);
  if (res != TCL_OK || !result || !result[0])
    {
    vtkGenericWarningMacro(<< "Unable to get packed slaves!");
    return 0;
    }
  
  // Allocate slaves

  *slaves = new char* [nb_slaves];
  
  // Browse each slave and store it

  int buffer_length = strlen(result);
  char *buffer = new char [buffer_length + 1];
  strcpy(buffer, result);

  char *buffer_end = buffer + buffer_length;
  char *ptr = buffer, *word_end;
  int i = 0;

  while (ptr < buffer_end && i < nb_slaves)
    {
    word_end = strchr(ptr + 1, ' ');
    if (word_end == NULL)
      {
      word_end = buffer_end;
      }
    else
      {
      *word_end = 0;
      }

    (*slaves)[i] = new char [strlen(ptr) + 1];
    strcpy((*slaves)[i], ptr);

    i++;
    ptr = word_end + 1;
    }

  delete [] buffer;

  return nb_slaves;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetSlavesInPack(vtkKWWidget *widget, 
                                char ***slaves)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::GetSlavesInPack(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    slaves);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetPreviousAndNextSlaveInPack(
  Tcl_Interp *interp,
  const char *widget,
  const char *slave,
  ostream &previous_slave,
  ostream &next_slave)
{
  // Search (and allocate) the slaves

  char **slaves = 0;
  int nb_slaves = vtkKWTkUtilities::GetSlavesInPack(interp, widget, &slaves);
  if (!nb_slaves)
    {
    return 0;
    }

  // Browse each of them

  int i, found = 0;
  for (i = 0; i < nb_slaves; i++)
    {
    if (!strcmp(slaves[i], slave))
      {
      if (i > 0)
        {
        previous_slave << slaves[i - 1];
        }
      if (i < nb_slaves - 1)
        {
        next_slave << slaves[i + 1];
        }
      found = 1;
      break;
      }
    }

  // Deallocate slaves

  for (i = 0 ; i < nb_slaves; i++)
    {
    delete [] slaves[i];
    }
  delete [] slaves;

  return found;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::GetPreviousAndNextSlaveInPack(
  vtkKWWidget *widget, 
  vtkKWWidget *slave, 
  ostream &previous_slave,
  ostream &next_slave)
{
  if (!widget || !widget->IsCreated() ||
      !slave || !slave->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::GetPreviousAndNextSlaveInPack(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    slave->GetWidgetName(),
    previous_slave, next_slave);
}

/*
 *--------------------------------------------------------------
 *
 * TkImageGetColor --
 *
 *  This procedure converts a pixel value to three floating
 *      point numbers, representing the amount of red, green, and 
 *      blue in that pixel on the screen.  It makes use of colormap
 *      data passed as an argument, and should work for all Visual
 *      types.
 *
 *  This implementation is bogus on Windows because the colormap
 *  data is never filled in.  Instead all postscript generated
 *  data coming through here is expected to be RGB color data.
 *  To handle lower bit-depth images properly, XQueryColors
 *  must be implemented for Windows.
 *
 * Results:
 *  Returns red, green, and blue color values in the range 
 *      0 to 1.  There are no error returns.
 *
 * Side effects:
 *  None.
 *
 *--------------------------------------------------------------
 */

/*
 * The following definition is used in generating postscript for images
 * and windows.
 */

struct vtkKWTkUtilitiesTkColormapData {  /* Hold color information for a window */
  int separated;    /* Whether to use separate color bands */
  int color;      /* Whether window is color or black/white */
  int ncolors;    /* Number of color values stored */
  XColor *colors;    /* Pixel value -> RGB mappings */
  int red_mask, green_mask, blue_mask;  /* Masks and shifts for each */
  int red_shift, green_shift, blue_shift;  /* color band */
};

static void
vtkKWTkUtilitiesTkImageGetColor(vtkKWTkUtilitiesTkColormapData* 
#ifdef WIN32
#else
                           cdata
#endif
                           ,
                           unsigned long pixel, 
                           double *red, 
                           double *green, 
                           double *blue)
#ifdef WIN32

/*
 * We could just define these instead of pulling in windows.h.
#define GetRValue(rgb)  ((BYTE)(rgb))
#define GetGValue(rgb)  ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)  ((BYTE)((rgb)>>16))
*/

{
  *red   = (double) GetRValue(pixel) / 255.0;
  *green = (double) GetGValue(pixel) / 255.0;
  *blue  = (double) GetBValue(pixel) / 255.0;
}
#else
{
  if (cdata->separated) {
    int r = (pixel & cdata->red_mask) >> cdata->red_shift;
    int g = (pixel & cdata->green_mask) >> cdata->green_shift;
    int b = (pixel & cdata->blue_mask) >> cdata->blue_shift;
    *red   = cdata->colors[r].red / 65535.0;
    *green = cdata->colors[g].green / 65535.0;
    *blue  = cdata->colors[b].blue / 65535.0;
  } else {
    *red   = cdata->colors[pixel].red / 65535.0;
    *green = cdata->colors[pixel].green / 65535.0;
    *blue  = cdata->colors[pixel].blue / 65535.0;
  }
}
#endif

//----------------------------------------------------------------------------
int vtkKWTkUtilities::TakeScreenDump(Tcl_Interp *interp,
                                     const char* widget, 
                                     const char* fname,
                                     int top, int bottom, int left, int right)
{
  if (!interp || !fname || !widget)
    {
    return 0;
    }

  ostrstream geometry;
  geometry << "concat [winfo rootx " << widget << "] [winfo rooty "
           << widget << "] [winfo width " << widget << "] [winfo height "
           << widget << "]" << ends;
  int res = Tcl_GlobalEval(interp, geometry.str());
  geometry.rdbuf()->freeze(0);
  if (res != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to query widget geometry! " << widget);
    return 0;
    }
  
  int xx, yy, ww, hh;
  xx = yy = ww = hh = 0;
  if (sscanf(
        Tcl_GetStringResult(interp), "%d %d %d %d", &xx, &yy, &ww, &hh) != 4)
    {
    return 0;
    }

  xx -= left;
  yy -= top;
  ww += left + right;
  hh += top + bottom;

  Tk_Window image_window;

  image_window = Tk_MainWindow(interp);
  Display *dpy = Tk_Display(image_window);
  int screen = DefaultScreen(dpy);
  Window win=RootWindow(dpy, screen);

  XImage *ximage = XGetImage(dpy, win, xx, yy,
    (unsigned int)ww, (unsigned int)hh, AllPlanes, XYPixmap);
  if ( !ximage )
    {
    return 0;
    }
  unsigned int buffer_size = ximage->bytes_per_line * ximage->height;
  if (ximage->format != ZPixmap)
    {
    buffer_size = ximage->bytes_per_line * ximage->height * ximage->depth;
    }

  vtkKWTkUtilitiesTkColormapData cdata;
  Colormap cmap;
  Visual *visual;
  int i, ncolors;
  cmap = Tk_Colormap(image_window);
  visual = Tk_Visual(image_window);

  /*
   * Obtain information about the colormap, ie the mapping between
   * pixel values and RGB values.  The code below should work
   * for all Visual types.
   */

  ncolors = visual->map_entries;
  cdata.colors = (XColor *) ckalloc(sizeof(XColor) * ncolors);
  cdata.ncolors = ncolors;

  if (visual->c_class == DirectColor || visual->c_class == TrueColor) 
    {
    cdata.separated = 1;
    cdata.red_mask = visual->red_mask;
    cdata.green_mask = visual->green_mask;
    cdata.blue_mask = visual->blue_mask;
    cdata.red_shift = 0;
    cdata.green_shift = 0;
    cdata.blue_shift = 0;
    while ((0x0001 & (cdata.red_mask >> cdata.red_shift)) == 0)
      cdata.red_shift ++;
    while ((0x0001 & (cdata.green_mask >> cdata.green_shift)) == 0)
      cdata.green_shift ++;
    while ((0x0001 & (cdata.blue_mask >> cdata.blue_shift)) == 0)
      cdata.blue_shift ++;
    for (i = 0; i < ncolors; i ++)
      cdata.colors[i].pixel =
        ((i << cdata.red_shift) & cdata.red_mask) |
        ((i << cdata.green_shift) & cdata.green_mask) |
        ((i << cdata.blue_shift) & cdata.blue_mask);
    } 
  else 
    {
    cdata.separated=0;
    for (i = 0; i < ncolors; i ++)
      cdata.colors[i].pixel = i;
    }
  if (visual->c_class == StaticGray || visual->c_class == GrayScale)
    {
    cdata.color = 0;
    }
  else
    {
    cdata.color = 1;
    }

  XQueryColors(Tk_Display(image_window), cmap, cdata.colors, ncolors);

  /*
   * Figure out which color level to use (possibly lower than the 
   * one specified by the user).  For example, if the user specifies
   * color with monochrome screen, use gray or monochrome mode instead. 
   */

  int level = 2;
  if (!cdata.color && level == 2) 
    {
    level = 1;
    }

  if (!cdata.color && cdata.ncolors == 2) 
    {
    level = 0;
    }


  unsigned long stride = ww * 3;
  unsigned long buffer_length = stride * hh;
  unsigned char *buffer = new unsigned char [buffer_length];
  unsigned char *ptr = buffer + buffer_length - stride;

  int x, y;
  for (y = 0; y < hh; y++) 
    {
    for (x = 0; x < ww; x++) 
      {
      double red, green, blue;
      vtkKWTkUtilitiesTkImageGetColor(
        &cdata, XGetPixel(ximage, x, hh - y - 1), &red, &green, &blue);
      *ptr++ = (unsigned char)(255 * red);
      *ptr++ = (unsigned char)(255 * green);
      *ptr++ = (unsigned char)(255 * blue);
      }
    ptr -= stride;
    ptr -= stride;
    }

  vtkKWResourceUtilities::WritePNGImage(fname, ww, hh, 3, buffer);

  delete [] buffer;

  XDestroyImage(ximage);

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::TakeScreenDump(vtkKWWidget *widget, 
                                     const char* fname,
                                     int top, int bottom, int left, int right)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::TakeScreenDump(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    fname,
    top, bottom, left, right);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::SetTopLevelMouseCursor(Tcl_Interp *interp,
                                             const char* widget, 
                                             const char *cursor)
{
  if (!interp || !widget)
    {
    return 0;
    }

  vtksys_stl::string cmd;
  cmd = "[winfo toplevel ";
  cmd += widget;
  cmd += "] configure -cursor {";
  if (cursor)
    {
    cmd += cursor;
    }
  cmd += "}";

  if (Tcl_GlobalEval(interp, cmd.c_str()) != TCL_OK)
    {
    vtkGenericWarningMacro(
      << "Unable to change toplevel mouse cursor: " 
      << Tcl_GetStringResult(interp));
    return 0;
    }
 
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::SetTopLevelMouseCursor(vtkKWWidget *widget, 
                                             const char *cursor)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::SetTopLevelMouseCursor(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName(),
    cursor);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::IsTopLevel(Tcl_Interp *interp,
                                 const char* widget)
{
  if (!interp || !widget)
    {
    return 0;
    }

  vtksys_stl::string cmd("winfo toplevel ");
  cmd += widget;

  if (Tcl_GlobalEval(interp, cmd.c_str()) != TCL_OK)
    {
    vtkGenericWarningMacro(
      << "Unable to query toplevel: " << Tcl_GetStringResult(interp));
    return 0;
    }
 
  return (!strcmp(Tcl_GetStringResult(interp), widget) ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWTkUtilities::IsTopLevel(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return 0;
    }
  
  return vtkKWTkUtilities::IsTopLevel(
    widget->GetApplication()->GetMainInterp(),
    widget->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWTkUtilities::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

