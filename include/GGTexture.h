// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@hotmail.com */

/* $Header$ */

#ifndef _GGTexture_h_
#define _GGTexture_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

namespace GG {

/** This class encapsulates OpenGL texture objects.  Thanks to the SDL_image library, it is extremely easy to load an image of 
    virtually any format and create an SDL surface out of it in one step.  When initialized with Load(), Texture objects 
    create such an SDL surface from a file, then convert it to an OpenGL texture, then free the surface.  If the dimensions of the 
    file image were not both powers of two, the created OpenGL texture is created with dimensions to the next largest powers
    of two; the original image size and corresponding texture coords are saved in m_default_width, m_default_height, and
    m_tex_coords, respectively.  These are kept so that only the originally-loaded-image part of the texture can be used, if desired.  
    The same thing happens when the texture is initailized using Init(SDL_Surface*...).  However, when using Init(int width...), 
    the user must ensure that the image provided has power-of-two dimensions; no default parameters are recorded (other than the 
    default texture coords (0,0) to (1,1).  All five initialization functions first free the OpenGL texture currently in use (if 
    any) and create a new one.  All initialization functions fail silently, performing no initialization and allocating no 
    memory or OpenGL texture when the load filename is "" or the surf or image parameters are 0.  
    \note It is important to remember that OpenGL does 
    not support the alteration of textures once loaded.  Texture therefore also does not provide any such support.  Also,
    since a texture saved as an XMLElement is recreated from the filename associated with the texture, creating a texture from
    a programmatically created or altered image requires that the recreation of the Texture be handled by the user explicitly. */
class Texture
{
public:
    GGEXCEPTION(TextureException);   ///< exception class \see GG::GGEXCEPTION

    /** \name Structors */ //@{
    Texture();                       ///< ctor
    Texture(const XMLElement& elem); ///< ctor that constructs a Texture object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Texture object
    virtual ~Texture();              ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    inline GLenum           WrapS() const     {return m_wrap_s;}      ///< returns S-wrap mode associated with this opengl texture
    inline GLenum           WrapT() const     {return m_wrap_t;}      ///< returns T-wrap mode associated with this opengl texture
    inline GLenum           MinFilter() const {return m_min_filter;}  ///< returns minimization filter modes associated with this opengl texture
    inline GLenum           MagFilter() const {return m_mag_filter;}  ///< returns maximization filter modes associated with this opengl texture
    inline int              BytesPP() const   {return m_bytes_pp;}    ///< returns the image's color depth in bytes
    inline int              Width() const     {return m_width;}       ///< returns width of entire texture
    inline int              Height() const    {return m_height;}      ///< returns height of entire texture
    inline bool             MipMapped() const {return m_mipmaps;}     ///< returns true if the texture has mipmaps
    inline GLuint           OpenGLId() const  {return m_opengl_id;}   ///< GLuint "name" of the opengl texture object associated with this object
    inline const GLfloat*   DefaultTexCoords() const   {return m_tex_coords;}     ///< texture coordinates to use by default when blitting this texture
    inline int              DefaultWidth() const       {return m_default_width;}  ///< returns width in pixels, based on initial surface (0 if texture was not loaded)
    inline int              DefaultHeight() const      {return m_default_height;} ///< returns height in pixels, based on initial surface (0 if texture was not loaded)
   
    /** blit any portion of texture to any place on screen, scaling as necessary*/
    void OrthoBlit(const Pt& pt1, const Pt& pt2, const GLfloat* tex_coords = 0, bool enter_2d_mode = true) const {OrthoBlit(pt1.x, pt1.y, pt2.x, pt2.y, tex_coords, enter_2d_mode);}
   
    /** blit any portion of texture to any place on screen, scaling as necessary*/
    void OrthoBlit(int x1, int y1, int x2, int y2, const GLfloat* tex_coords = 0, bool enter_2d_mode = true) const; 
   
    /** blit default portion of texture unscaled to \a pt (upper left corner)*/
    void OrthoBlit(const Pt& pt, bool enter_2d_mode = true) const {OrthoBlit(pt.x, pt.y, enter_2d_mode);}
   
    /** blit default portion of texture unscaled to \a x,\a y (upper left corner)*/
    void OrthoBlit(int x, int y, bool enter_2d_mode = true) const; 
   
    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a Texture object
    //@}

    /** \name Mutators */ //@{
    // intialization functions
    void Load(const string& filename, bool mipmap = false) {Load(filename.c_str(), mipmap);} ///< frees any currently-held memory and loads a texture from file \a filename.  \throw TextureException May throw if the texture creation fails.
    void Load(const char* filename, bool mipmap = false); ///< frees any currently-held memory and loads a texture from file \a filename.  \throw TextureException May throw if the texture creation fails.
    void Init(int width, int height, const unsigned char* image, Uint32 channels, bool mipmap = false); ///< frees any currently-held memory and creates a texture from supplied array \a image.  \throw TextureException May throw if the texture creation fails.
    void Init(int x, int y, int width, int height, int image_width, const unsigned char* image, int channels, bool mipmap = false); ///< frees any currently-held memory and creates a texture from subarea of supplied array \a image.  \throw TextureException May throw if the texture creation fails.
   
    void SetWrap(GLenum s, GLenum t);         ///< sets the opengl texture wrap modes associated with opengl texture m_opengl_id
    void SetFilters(GLenum min, GLenum mag);  ///< sets the opengl min/mag filter modes associated with opengl texture m_opengl_id
    void Clear();  ///< frees the opengl texture object associated with this object
    //@}

private:
    Texture(const Texture& rhs);             ///< disabled
    Texture& operator=(const Texture& rhs);  ///< disabled
   
    string   m_filename;   ///< filename from which this Texture was constructed ("" if not loaded from a file)

    int      m_bytes_pp;
    int      m_width;
    int      m_height;

    GLenum   m_wrap_s, m_wrap_t;
    GLenum   m_min_filter, m_mag_filter;
   
    bool     m_mipmaps;

    GLuint   m_opengl_id;   ///< OpenGL texture ID

    /// each of these is used for a non-power-of-two-sized graphic loaded into a power-of-two-sized texture
    GLfloat  m_tex_coords[4];  ///< the texture coords used to blit from this texture by default (reflecting original image width and height)
    int      m_default_width;  ///< the original width and height of this texture to be used in blitting 
    int      m_default_height;  
};

/** This class is a convenient way to store the info needed to use a portion of an OpenGL texture.*/
class SubTexture
{
public:
    GGEXCEPTION(SubTextureException);   ///< exception class \see GG::GGEXCEPTION

    /** \name Structors */ //@{
    SubTexture(); ///< default ctor
   
    /** creates a SubTexture from a GG::Texture and coordinates into it. \warning Calling code <b>must not</b> delete 
        \a texture; \a texture becomes the property of a shared_ptr inside the SubTexture. \throw SubTextureException 
        May throw. */
    SubTexture(const Texture* texture, int x1, int y1, int x2, int y2); 
   
    /** creates a SubTexture from a GG::Texture and coordinates into it \throw SubTextureException May throw. */
    SubTexture(const shared_ptr<const Texture>& texture, int x1, int y1, int x2, int y2);  
   
    SubTexture(const SubTexture& rhs); ///< copy ctor
    const SubTexture& operator=(const SubTexture& rhs); ///< assignment operator
    SubTexture(const XMLElement& elem); ///< ctor that constructs a SubTexture object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a SubTexture object
    virtual ~SubTexture() {} ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    inline bool             Empty() const     {return !m_texture;}     ///< returns true if this object has no associated GG::Texture
    inline const GLfloat*   TexCoords() const {return m_tex_coords;}   ///< texture coordinates to use when blitting this sub-texture
    inline int              Width() const     {return m_width;}        ///< width of sub-texture in pixels
    inline int              Height() const    {return m_height;}       ///< height of sub-texture in pixels
    inline const Texture*   GetTexture()const {return m_texture.get();}///< returns the texture the SubTexture is a part of
   
    /** blit sub-texture to any place on screen, scaling as necessary \see GG::Texture::OrthoBlit*/
    void OrthoBlit(const Pt& pt1, const Pt& pt2, bool enter_2d_mode = true) const {OrthoBlit(pt1.x, pt1.y, pt2.x, pt2.y, enter_2d_mode);}
   
    /** blit sub-texture to any place on screen, scaling as necessary \see GG::Texture::OrthoBlit*/
    void OrthoBlit(int x1, int y1, int x2, int y2, bool enter_2d_mode = true) const; 
   
    /** blit sub-texture unscaled to \a pt (upper left corner) \see GG::Texture::OrthoBlit*/
    void OrthoBlit(const Pt& pt, bool enter_2d_mode = true) const {OrthoBlit(pt.x, pt.y, enter_2d_mode);}
   
    /** blit sub-texture unscaled to \a x, \a y (upper left corner) \see GG::Texture::OrthoBlit*/
    void OrthoBlit(int x, int y, bool enter_2d_mode = true) const;
   
    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a SubTexture object
    //@}

private:
    shared_ptr<const Texture>  m_texture;        ///< shared_ptr to texture object with entire image
    int                        m_width;
    int                        m_height;
    GLfloat                    m_tex_coords[4];  ///< position of element within containing texture 
};

/** This singleton class is essentially a very thin wrapper around a map of Texture smart pointers, keyed on std::string texture names.  
    The user need only request a texture through GetTexture(); if the texture is not already resident, it will be loaded.  If the user would 
    like to create her own images and store them in the manager, that can be accomplished via StoreTexture() calls.*/
class TextureManager
{
public:
    GGEXCEPTION(TextureManagerException);   ///< exception class \see GG::GGEXCEPTION

    /** \name Structors */ //@{
    TextureManager(); ///< ctor
    //@}

    /** \name Mutators */ //@{
    shared_ptr<Texture> StoreTexture(Texture* texture, const string& texture_name); ///< stores a pre-existing GG::Texture in the manager's texture pool, and returns a shared_ptr to it. \warning Calling code <b>must not</b> delete \a texture; \a texture becomes the property of the manager, which will eventually delete it.
    shared_ptr<Texture> StoreTexture(shared_ptr<Texture> texture, const string& texture_name); ///< stores a pre-existing GG::Texture in the manager's texture pool, and returns a shared_ptr to it. \warning Calling code <b>must not</b> delete \a texture; \a texture becomes the property of the manager, which will eventually delete it.
    shared_ptr<Texture> GetTexture(const string& name, bool mipmap = false);  ///< returns a shared_ptr to the texture created from image file \a name; mipmapped textures are generated if \a mimap is true.  If the texture is not present in the manager's pool, it will be loaded from disk.
    void                FreeTexture(const string& name); ///< removes the manager's shared_ptr to the texture created from image file \a name, if it exists.  \note Due to shared_ptr semantics, the texture may not be deleted until much later.
    //@}

   static void         InitDevIL(); ///< initializes DevIL image library, if it is not already initialized

private:
    shared_ptr<Texture> LoadTexture(const string& filename, bool mipmap);

    static bool                         s_created;
    static bool                         s_il_initialized;
    map<string, shared_ptr<Texture> >   m_textures;
};

} // namespace GG

#endif // _GGTexture_h_

