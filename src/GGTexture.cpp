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

#include "GGTexture.h"

#include <GGApp.h>

#include <IL/il.h>

#include <iostream>

/* some versions of libDevIL are linked with Allegro, which requires
   _mangled_main_address to be defined. Futhermore, ilut.h includes
   allegro.h as extern "C", which is wrong, because allegro defines
   some C++-classes if it detects a C++ compiler. So, we need to
   include allegro before including ilut!
 
   If GG_NO_ALLEGRO_HACK is not defined, we include a
   _mangled_main_address variable with a dummy value; this would cause
   * Allegro programs to crash! Therefore, if GG_NO_ALLEGRO_HACK is
   defined, the developer has to use Allegro's END_OF_MAIN macro. */

#ifndef _GGConfig_h_
# include "GGConfig.h"
#endif

#ifdef GG_DEVIL_WITH_ALLEGRO
# include <allegro.h>
# ifndef GG_NO_ALLEGRO_HACK
   /* This "hexspeak" address should stand out in an debugger,
      reminding the developer what went wrong */
   void * _mangled_main_address = (void*) 0xdeadbabe;
# endif
#endif

#include <IL/ilut.h>

#include <iomanip>

#define VERBOSE_DEVIL_ERROR_REPORTING 0

using namespace GG;

namespace {
    int PowerOfTwo(int input)
    {
        int value = 1;
        while (value < input)
            value <<= 1;
        return value;
    }

    void CheckILErrors(const std::string& function_call)
    {
        ILuint error;
        while ((error = ilGetError()) != IL_NO_ERROR) {
#if VERBOSE_DEVIL_ERROR_REPORTING
            std::cerr << "IL call \"" << function_call << "\" failed with IL error \"" << iluErrorString(error)
                      << "\" (code " << error << ")\n";
#endif
        }
    }
}

///////////////////////////////////////
// class GG::Texture
///////////////////////////////////////
Texture::Texture() :
    m_bytes_pp(0),
    m_width(0),
    m_height(0),
    m_wrap_s(GL_REPEAT),
    m_wrap_t(GL_REPEAT),
    m_min_filter(GL_NEAREST_MIPMAP_LINEAR),
    m_mag_filter(GL_LINEAR),
    m_mipmaps(false),
    m_opengl_id(0),
    m_tex_coords(),
    m_default_width(0),
    m_default_height(0)
{
    Clear();
}

Texture::~Texture()
{
    Clear();
}

std::string Texture::Filename() const
{
    return m_filename;
}

GLenum Texture::WrapS() const
{
    return m_wrap_s;
}

GLenum Texture::WrapT() const
{
    return m_wrap_t;
}

GLenum Texture::MinFilter() const
{
return m_min_filter;
}

GLenum Texture::MagFilter() const
{
    return m_mag_filter;
}

int Texture::BytesPP() const
{
    return m_bytes_pp;
}

int Texture::Width() const
{
    return m_width;
}

int Texture::Height() const
{
    return m_height;
}

bool Texture::MipMapped() const
{
    return m_mipmaps;
}

GLuint Texture::OpenGLId() const
{
    return m_opengl_id;
}

const GLfloat* Texture::DefaultTexCoords() const
{
    return m_tex_coords;
}

int Texture::DefaultWidth() const
{
    return m_default_width;
}

int Texture::DefaultHeight() const
{
    return m_default_height;
}

void Texture::OrthoBlit(const Pt& pt1, const Pt& pt2, const GLfloat* tex_coords/* = 0*/, bool enter_2d_mode/* = true*/) const
{
    OrthoBlit(pt1.x, pt1.y, pt2.x, pt2.y, tex_coords, enter_2d_mode);
}

void Texture::OrthoBlit(int x1, int y1, int x2, int y2, const GLfloat* tex_coords/* = 0*/, bool enter_2d_mode/* = true*/) const
{
    if (m_opengl_id) {
        if (!tex_coords) // use default texture coords when not given any others
            tex_coords = m_tex_coords;

        if (enter_2d_mode)
            App::GetApp()->Enter2DMode(); // enter 2D mode, if needed

        glBindTexture(GL_TEXTURE_2D, m_opengl_id);

        // HACK! This code ensures that unscaled textures are reproduced exactly, even
        // though they theoretically should be even when using non-GL_NEAREST* scaling.
        bool render_scaled = (x2 - x1) != m_default_width || (y2 - y1) != m_default_height;
        bool need_min_filter_change = !render_scaled && m_min_filter != (m_mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
        bool need_mag_filter_change = !render_scaled && m_mag_filter != GL_NEAREST;
        if (need_min_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
        if (need_mag_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // render texture
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(tex_coords[0], tex_coords[1]); glVertex2i(x1, y1);
        glTexCoord2f(tex_coords[2], tex_coords[1]); glVertex2i(x2, y1);
        glTexCoord2f(tex_coords[0], tex_coords[3]); glVertex2i(x1, y2);
        glTexCoord2f(tex_coords[2], tex_coords[3]); glVertex2i(x2, y2);
        glEnd();

        if (need_min_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        if (need_mag_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);

        if (enter_2d_mode)
            App::GetApp()->Exit2DMode(); // exit 2D mode, if needed
    }
}

void Texture::OrthoBlit(const Pt& pt, bool enter_2d_mode/* = true*/) const
{
    OrthoBlit(pt.x, pt.y, enter_2d_mode);
}

void Texture::OrthoBlit(int x, int y, bool enter_2d_mode/* = true*/) const
{
    OrthoBlit(x, y, x + m_default_width, y + m_default_height, m_tex_coords, enter_2d_mode);
}

void Texture::Load(const std::string& filename, bool mipmap/* = false*/)
{
    Load(filename.c_str(), mipmap);
}

void Texture::Load(const char* filename, bool mipmap/* = false*/)
{
    if (m_opengl_id)
        Clear();

    TextureManager::InitDevIL();

    ILuint id, error;
    ilGenImages(1, &id);
    CheckILErrors("ilGenImages(1, &id)");
    ilBindImage(id);
    CheckILErrors("ilBindImage(id)");
    ilLoadImage(const_cast<char*>(filename));
    CheckILErrors("ilLoadImage(const_cast<char*>(filename))");
    if ((error = ilGetError()) != IL_NO_ERROR)
        throw BadFile((std::string("Could not load temporary DevIL image from file \'") + filename) + "\'");
   
    if (mipmap) {
        m_opengl_id = ilutGLBindMipmaps();
        CheckILErrors("ilutGLBindMipmaps()");
    } else {
        m_opengl_id = ilutGLBindTexImage();
        CheckILErrors("ilutGLBindTexImage()");
    }
    if (!m_opengl_id || (error = ilGetError()) != IL_NO_ERROR)
        throw BadFile((std::string("Could not create OpenGL texture object from file \'") + filename) + "\'");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);

    // be sure to record these
    m_filename = filename;
    m_mipmaps = mipmap;
    m_bytes_pp = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
    CheckILErrors("ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL)");
    m_default_width = ilGetInteger(IL_IMAGE_WIDTH);
    CheckILErrors("ilGetInteger(IL_IMAGE_WIDTH)");
    m_default_height = ilGetInteger(IL_IMAGE_HEIGHT);
    CheckILErrors("ilGetInteger(IL_IMAGE_HEIGHT)");
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_height);
    m_tex_coords[2] = m_default_width / double(m_width);
    m_tex_coords[3] = m_default_height / double(m_height);
    
    ilDeleteImages(1, &id);
    CheckILErrors("ilDeleteImages(1, &id)");
}

void Texture::Init(int x, int y, int width, int height, int image_width, const unsigned char* image, int channels, bool mipmap/* = false*/)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, false);
    glPixelStorei(GL_UNPACK_LSB_FIRST, false);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image_width);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    try {
        InitFromRawData(width, height, image, channels, mipmap);
    } catch (...) {
        glPopClientAttrib();
        throw;
    }

    glPopClientAttrib();
}

void Texture::Init(int width, int height, const unsigned char* image, Uint32 channels, bool mipmap/* = false*/)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, false);
    glPixelStorei(GL_UNPACK_LSB_FIRST, false);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    try {
        InitFromRawData(width, height, image, channels, mipmap);
    } catch (...) {
        glPopClientAttrib();
        throw;
    }

    glPopClientAttrib();
}

void Texture::SetWrap(GLenum s, GLenum t)
{
    m_wrap_s = s;
    m_wrap_t = t;
    if (m_opengl_id) {
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);
    }
}

void Texture::SetFilters(GLenum min, GLenum mag)
{
    m_min_filter = min;
    m_mag_filter = mag;
    if (m_opengl_id) {
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    }
}

void Texture::Clear()
{
    if (m_opengl_id)
        glDeleteTextures(1, &m_opengl_id);

    m_filename = "";

    m_bytes_pp = 4;
    m_default_width = m_width = 0;
    m_default_height = m_height = 0;

    m_wrap_s = m_wrap_t = GL_REPEAT;
    m_min_filter = m_mag_filter = GL_NEAREST;

    m_mipmaps = false;

    m_opengl_id = 0;

    m_tex_coords[0] = m_tex_coords[1] = 0.0f;   // min x, y
    m_tex_coords[2] = m_tex_coords[3] = 1.0f;   // max x, y
}

void Texture::InitFromRawData(int width, int height, const unsigned char* image, Uint32 channels, bool mipmap)
{
    if (m_opengl_id)
        Clear();

    if (image) {
        GLenum mode = 0;
        switch (channels) {
        case 1:   mode = GL_LUMINANCE;       break;
        case 2:   mode = GL_LUMINANCE_ALPHA; break;
        case 3:   mode = GL_RGB;             break;
        case 4:   mode = GL_RGBA;            break;
        default: throw InvalidColorChannels("Attempted to initialize a GG::Texture with an invalid number of color channels");
        }

        int GL_texture_width = PowerOfTwo(width);
        int GL_texture_height = PowerOfTwo(height);
        if (width != GL_texture_width || height != GL_texture_height)
            throw BadSize("Attempted to create a texture whose sides are not powers of two");

        glGenTextures(1, &m_opengl_id);
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);                     // set this texture as the current opengl texture
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);   // set some relevant opengl texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);

        if (mipmap) { // creating mipmapped texture
            // check to see if this texture can be created with available resources
            gluBuild2DMipmaps(GL_PROXY_TEXTURE_2D, channels, GL_texture_width, GL_texture_height, mode, GL_UNSIGNED_BYTE, image);
            GLint format;
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
            if (format) // if yes, build it
                gluBuild2DMipmaps(GL_TEXTURE_2D, channels, GL_texture_width, GL_texture_height, mode, GL_UNSIGNED_BYTE, image);
            else // if no, throw
                throw InsufficientResources("Insufficient resources to create requested mipmapped OpenGL texture");
        } else { // creating non-mipmapped texture
            glTexImage2D(GL_PROXY_TEXTURE_2D, 0, channels, GL_texture_width, GL_texture_height, 0, mode, GL_UNSIGNED_BYTE, image);
            GLint format;
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
            if (format)
                glTexImage2D(GL_TEXTURE_2D, 0, channels, GL_texture_width, GL_texture_height, 0, mode, GL_UNSIGNED_BYTE, image);
            else
                throw InsufficientResources("Insufficient resources to create requested OpenGL texture");
        }

        // be sure to record these
        m_mipmaps = mipmap;
        m_bytes_pp = channels;
        m_default_width = width;
        m_default_height = height;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_height);
    }
}

unsigned char* Texture::GetRawBytes()
{
    unsigned char* retval = 0;
    if (m_filename == "" && m_opengl_id) {
        glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
        glPixelStorei(GL_PACK_SWAP_BYTES, false);
        glPixelStorei(GL_PACK_LSB_FIRST, false);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        // get pixel data
        typedef unsigned char uchar;
        retval = new uchar[m_width * m_height * m_bytes_pp];
        GLenum mode = 0;
        switch (m_bytes_pp) {
        case 1:   mode = GL_LUMINANCE;       break;
        case 2:   mode = GL_LUMINANCE_ALPHA; break;
        case 3:   mode = GL_RGB;             break;
        case 4:   mode = GL_RGBA;            break;
        default: throw InvalidColorChannels("Attempted to encode a GG::Texture with an invalid number of bytes per pixel");
        }
        glGetTexImage(GL_TEXTURE_2D, 0, mode, GL_UNSIGNED_BYTE, retval);
        glPopClientAttrib();
    }
    return retval;
}



///////////////////////////////////////
// class GG::SubTexture
///////////////////////////////////////
SubTexture::SubTexture() :
    m_width(0),
    m_height(0),
    m_tex_coords()
{
}

SubTexture::SubTexture(const Texture* texture, int x1, int y1, int x2, int y2) :
    m_texture(boost::shared_ptr<const Texture>(texture)),
    m_width(x2 - x1),
    m_height(y2 - y1),
    m_tex_coords()
{
    if (!m_texture) throw BadTexture("Attempted to contruct subtexture from invalid texture");
    if (x2 < x1 || y2 < y1) throw InvalidTextureCoordinates("Attempted to contruct subtexture from invalid coordinates");

    m_tex_coords[0] = static_cast<double>(x1) / texture->DefaultWidth();
    m_tex_coords[1] = static_cast<double>(y1) / texture->DefaultHeight();
    m_tex_coords[2] = static_cast<double>(x2) / texture->DefaultWidth();
    m_tex_coords[3] = static_cast<double>(y2) / texture->DefaultHeight();
}

SubTexture::SubTexture(const boost::shared_ptr<const Texture>& texture, int x1, int y1, int x2, int y2) :
    m_texture(texture),
    m_width(x2 - x1),
    m_height(y2 - y1),
    m_tex_coords()
{
    if (!m_texture) throw BadTexture("Attempted to contruct subtexture from invalid texture");
    if (x2 < x1 || y2 < y1) throw InvalidTextureCoordinates("Attempted to contruct subtexture from invalid coordinates");

    m_tex_coords[0] = static_cast<double>(x1) / texture->DefaultWidth();
    m_tex_coords[1] = static_cast<double>(y1) / texture->DefaultHeight();
    m_tex_coords[2] = static_cast<double>(x2) / texture->DefaultWidth();
    m_tex_coords[3] = static_cast<double>(y2) / texture->DefaultHeight();
}

SubTexture::~SubTexture()
{
}

SubTexture::SubTexture(const SubTexture& rhs)
{
    *this = rhs;
}

const SubTexture& SubTexture::operator=(const SubTexture& rhs)
{
    if (this != &rhs) {
        m_texture = rhs.m_texture;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_tex_coords[0] = rhs.m_tex_coords[0];
        m_tex_coords[1] = rhs.m_tex_coords[1];
        m_tex_coords[2] = rhs.m_tex_coords[2];
        m_tex_coords[3] = rhs.m_tex_coords[3];
    }
    return *this;
}

bool SubTexture::Empty() const
{
    return !m_texture;
}

const GLfloat* SubTexture::TexCoords() const
{
    return m_tex_coords;
}

int SubTexture::Width() const
{
    return m_width;
}

int SubTexture::Height() const
{
    return m_height;
}

const Texture* SubTexture::GetTexture()const
{
    return m_texture.get();
}

void SubTexture::OrthoBlit(const Pt& pt1, const Pt& pt2, bool enter_2d_mode/* = true*/) const
{
    OrthoBlit(pt1.x, pt1.y, pt2.x, pt2.y, enter_2d_mode);
}

void SubTexture::OrthoBlit(int x1, int y1, int x2, int y2, bool enter_2d_mode/* = true*/) const
{
    if (m_texture) m_texture->OrthoBlit(x1, y1, x2, y2, m_tex_coords, enter_2d_mode);
}

void SubTexture::OrthoBlit(const Pt& pt, bool enter_2d_mode/* = true*/) const
{
    OrthoBlit(pt.x, pt.y, enter_2d_mode);
}

void SubTexture::OrthoBlit(int x, int y, bool enter_2d_mode/* = true*/) const
{
    if (m_texture) m_texture->OrthoBlit(x, y, x + m_width, y + m_height, m_tex_coords, enter_2d_mode);
}


///////////////////////////////////////
// class GG::TextureManager
///////////////////////////////////////
// static member(s)
bool TextureManager::s_il_initialized = false;

TextureManager::TextureManager()
{}

boost::shared_ptr<Texture> TextureManager::StoreTexture(Texture* texture, const std::string& texture_name)
{
    boost::shared_ptr<Texture> temp(texture);
    return StoreTexture(temp, texture_name);
}

boost::shared_ptr<Texture> TextureManager::StoreTexture(boost::shared_ptr<Texture> texture, const std::string& texture_name)
{
    return (m_textures[texture_name] = texture);
}

boost::shared_ptr<Texture> TextureManager::GetTexture(const std::string& name, bool mipmap/* = false*/)
{
    std::map<std::string, boost::shared_ptr<Texture> >::iterator it = m_textures.find(name);
    if (it == m_textures.end()) { // if no such texture was found, attempt to load it now, using name as the filename
        return (m_textures[name] = LoadTexture(name, mipmap));
    } else { // otherwise, just return the texture we found
        return it->second;
    }
}

void TextureManager::FreeTexture(const std::string& name)
{
    std::map<std::string, boost::shared_ptr<Texture> >::iterator it = m_textures.find(name);
    if (it != m_textures.end())
        m_textures.erase(it);
}

void TextureManager::InitDevIL()
{
    if (!s_il_initialized) {
        // ensure we're starting with an empty error stack
        while (ilGetError() != IL_NO_ERROR) ;
        ilInit();
        CheckILErrors("ilInit()");
        iluInit();
        CheckILErrors("iluInit()");
        ilutInit();
        CheckILErrors("ilutInit()");
        ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
        CheckILErrors("ilOriginFunc()");
        ilutRenderer(ILUT_OPENGL);
        CheckILErrors("ilutRenderer()");
        GLint max_size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
        ilutSetInteger(ILUT_MAXTEX_WIDTH, max_size);
        CheckILErrors("ilutSetInteger(ILUT_MAXTEX_WIDTH, max_size)");
        ilutSetInteger(ILUT_MAXTEX_HEIGHT, max_size);
        CheckILErrors("ilutSetInteger(ILUT_MAXTEX_HEIGHT, max_size)");
        s_il_initialized = true;
    }
}

boost::shared_ptr<Texture> TextureManager::LoadTexture(const std::string& filename, bool mipmap)
{
    boost::shared_ptr<Texture> temp(new Texture());
    temp->Load(filename, mipmap);
    return (m_textures[filename] = temp);
}

TextureManager& GG::GetTextureManager()
{
    static TextureManager manager;
    return manager;
}
