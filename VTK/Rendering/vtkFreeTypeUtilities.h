/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFreeTypeUtilities - FreeType library support
// .SECTION Description
// vtkFreeTypeUtilities provides low-level interface to the FreeType library,
// including font-cache, rasterizing and vectorizing. Internal use only.
// EXPERIMENTAL for the moment. Also include the old cache.

#ifndef __vtkFreeTypeUtilities_h
#define __vtkFreeTypeUtilities_h

#define VTK_FTFC_CACHE_CAPACITY 150

#include "vtkObject.h"

class vtkImageData;
class vtkTextProperty;

// FreeType

#include "vtk_freetype.h"  //since ft2build.h could be in the path
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_IMAGE_H
#include FT_CACHE_CHARMAP_H

// FTGL

class FTFont;

//----------------------------------------------------------------------------
// Singleton cleanup

class VTK_RENDERING_EXPORT vtkFreeTypeUtilitiesCleanup
{
public:
  vtkFreeTypeUtilitiesCleanup();
  ~vtkFreeTypeUtilitiesCleanup();
};

//----------------------------------------------------------------------------
// Singleton font cache

class VTK_RENDERING_EXPORT vtkFreeTypeUtilities : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkFreeTypeUtilities, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This is a singleton pattern New. There will be only ONE reference 
  // to a vtkFreeTypeUtilities object per process.  Clients that
  // call this method must use Delete() on the object so that reference
  // counting will work. The single instance will be unreferenced when
  // the program exits. You should just use the static GetInstance() method
  // anyway to get the singleton.
  static vtkFreeTypeUtilities *New();

  // Description:
  // Return the singleton instance with no reference counting.
  static vtkFreeTypeUtilities* GetInstance();

  // Description:
  // Supply a user defined instance. Call Delete() on the supplied
  // instance after setting it to fix the reference count.
  static void SetInstance(vtkFreeTypeUtilities *instance);

  // Description:
  // Get the FreeType library singleton.
  FT_Library* GetLibrary();

  // Description:
  // Set/Get the maximum number of faces (FT_Face), sizes (FT_Size) and 
  // bytes used by the cache. These settings can be changed as long as
  // it is done prior to accessing any of the caches or the cache manager.
  vtkSetClampMacro(MaximumNumberOfFaces,unsigned int,1,VTK_UNSIGNED_INT_MAX);
  vtkGetMacro(MaximumNumberOfFaces, unsigned int);
  vtkSetClampMacro(MaximumNumberOfSizes,unsigned int,1,VTK_UNSIGNED_INT_MAX);
  vtkGetMacro(MaximumNumberOfSizes, unsigned int);
  vtkSetClampMacro(MaximumNumberOfBytes,unsigned long,1,VTK_UNSIGNED_LONG_MAX);
  vtkGetMacro(MaximumNumberOfBytes, unsigned long);

  // Description:
  // Get the FreeType cache manager, image cache and charmap cache
  FTC_Manager* GetCacheManager();
  FTC_ImageCache* GetImageCache();
  FTC_CMapCache* GetCMapCache();

  // Description:
  // Given a text property, get the corresponding FreeType size object
  // (a structure storing both a face and a specific size metric).
  // The size setting of the text property is used to set the size's face
  // to the corresponding size.
  // Return true on success, false otherwise
  int GetSize(vtkTextProperty *tprop, FT_Size *size);

  // Description:
  // Given a text property, get the corresponding FreeType face. 
  // The size parameter of the text property is ignored and a face with 
  // unknown current size is returned. Use GetSize() to get a specific size.
  // Return true on success, false otherwise
  int GetFace(vtkTextProperty *tprop, FT_Face *face);

  // Description:
  // Given a text property and a character, get the corresponding FreeType
  // glyph index.
  // Return true on success, false otherwise
  int GetGlyphIndex(vtkTextProperty *tprop, char c, FT_UInt *gindex);

  // Description:
  // Given a text property and a character, get the corresponding FreeType
  // glyph. The 'request' parameter can be used to request the glyph to be
  // in a specific format. If GLYPH_REQUEST_DEFAULT, the glyph might be either
  // an outline (most of the time) or a bitmap if the face includes a set of
  // pre-rendered glyphs (called "strikes") for a given size. 
  // If GLYPH_REQUEST_BITMAP, the glyph is rendered immediately and can
  // be safely cast to a FT_BitmapGlyph. If GLYPH_REQUEST_OUTLINE, no 
  // pre-rendered "strike" is considered, the glyph is an outline and can be
  // safely cast to a FT_OutlineGlyph.
  // Return true on success, false otherwise
  //BTX
  enum 
  {
    GLYPH_REQUEST_DEFAULT = 0,
    GLYPH_REQUEST_BITMAP  = 1,
    GLYPH_REQUEST_OUTLINE = 2
  };
  //ETX
  int GetGlyph(vtkTextProperty *tprop, 
               char c, 
               FT_Glyph *glyph, 
               int request = GLYPH_REQUEST_DEFAULT);

  // Description:
  // Given a text property and a string, get the bounding box [xmin, xmax] x 
  // [ymin, ymax]. Note that this is the bounding box of the area
  // where actual pixels will be written, given a text/pen/baseline location
  // of (0,0).
  // For example, if the string starts with a 'space', or depending on the 
  // orientation, you can end up with a [-20, -10] x [5, 10] bbox (the math
  // to get the real bbox is straightforward).
  // Return 1 on success, 0 otherwise.
  // You can use IsBoundingBoxValid() to test if the computed bbox
  // is valid (it may not if GetBoundingBox() failed or if the string 
  // was empty).
  int GetBoundingBox(vtkTextProperty *tprop, const char *str, int bbox[4]);
  int IsBoundingBoxValid(int bbox[4]);

  // Description:
  // Given a text property and a string, render it at coordinates (x, y) in
  // an vtkImageData. 
  // WARNING: writing to a L,A or RGB,A vtkImageData needs to be fixed.
  // Also, *no* bounds checking are performed for the moment, i.e. no
  // cropping, the text has to fit in the buffer (use GetBoundingBox()).
  int RenderString(vtkTextProperty *tprop, 
                   const char *str, 
                   int x, int y, 
                   vtkImageData *data);

  // Description:
  // For internal use only.
  // Given a text property 'tprop', get its unique ID in our cache framework.
  // In the same way, given a unique ID in our cache, retrieve the
  // corresponding text property and assign its parameters to 'tprop'.
  // Warning: there is no one to one mapping between a single text property
  // the corresponding ID, and vice-versa. The ID is just a fast hash, a 
  // binary mask concatenating the attributes of the text property that are
  // relevant to our cache (Color, Opacity, Justification setting are not
  // stored). 
  void MapTextPropertyToId(vtkTextProperty *tprop, unsigned long *tprop_cache_id);
  void MapIdToTextProperty(unsigned long tprop_cache_id, vtkTextProperty *tprop);

  // Description:
  // For internal use only.
  int GetSize(unsigned long tprop_cache_id, int font_size, FT_Size *size);
  int GetFace(unsigned long tprop_cache_id, FT_Face *face);
  int GetGlyphIndex(unsigned long tprop_cache_id, char c, FT_UInt *gindex);
  int GetGlyph(unsigned long tprop_cache_id, 
               int font_size, 
               FT_UInt gindex, 
               FT_Glyph *glyph, 
               int request = GLYPH_REQUEST_DEFAULT);

  // Old Code
  // Cache entry

  struct Entry
  {
    vtkTextProperty *TextProperty;
    FTFont *Font;
    float LargestAscender;
    float LargestDescender;
  };

  vtkFreeTypeUtilities::Entry* GetFont(vtkTextProperty *tprop, 
                                       double override_color[3] = NULL);

protected:
  vtkFreeTypeUtilities();
  virtual ~vtkFreeTypeUtilities();

private:
  vtkFreeTypeUtilities(const vtkFreeTypeUtilities&);  // Not implemented.
  void operator=(const vtkFreeTypeUtilities&);  // Not implemented.

  // The singleton instance and the singleton cleanup instance

  static vtkFreeTypeUtilities* Instance;
  static vtkFreeTypeUtilitiesCleanup Cleanup;

  // The cache manager, image cache and charmap cache

  FTC_Manager *CacheManager;
  FTC_ImageCache *ImageCache;
  FTC_CMapCache  *CMapCache;

  unsigned int MaximumNumberOfFaces;
  unsigned int MaximumNumberOfSizes;
  unsigned long MaximumNumberOfBytes;

  void InitializeCacheManager();
  void ReleaseCacheManager();

  // Old Code

  void PrintEntry(int i, char *msg = 0);
  void ReleaseEntry(int i);

  void InitializeCache();
  void ReleaseCache();

  Entry *Entries[VTK_FTFC_CACHE_CAPACITY];
  int NumberOfEntries;
};

#endif
