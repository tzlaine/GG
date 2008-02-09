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

#include <GG/Texture.h>

#include <GG/GUI.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include <iostream>
#include <iomanip>


using namespace GG;

namespace {
    const bool VERBOSE_DEVIL_ERROR_REPORTING = true;

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
            if (VERBOSE_DEVIL_ERROR_REPORTING) {
                std::cerr << "IL call \"" << function_call << "\" failed with IL error \"" << iluErrorString(error)
                          << "\" (code " << error << ")\n";
            }
        }
    }

    std::string ILenumToString(ILenum il_enum)
    {
#define ENUM_CASE(x) if (il_enum == x) return #x
        ENUM_CASE(IL_COLOR_INDEX);
        ENUM_CASE(IL_RGB);
        ENUM_CASE(IL_RGBA);
        ENUM_CASE(IL_BGR);
        ENUM_CASE(IL_BGRA);
        ENUM_CASE(IL_LUMINANCE);
        ENUM_CASE(IL_LUMINANCE_ALPHA);
        ENUM_CASE(IL_BYTE);
        ENUM_CASE(IL_UNSIGNED_BYTE);
        ENUM_CASE(IL_SHORT);
        ENUM_CASE(IL_UNSIGNED_SHORT);
        ENUM_CASE(IL_INT);
        ENUM_CASE(IL_UNSIGNED_INT);
        ENUM_CASE(IL_FLOAT);
        ENUM_CASE(IL_DOUBLE);
#undef ENUM_CASE
        return "UNKNOWN";
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
    m_min_filter(GL_LINEAR_MIPMAP_LINEAR),
    m_mag_filter(GL_LINEAR),
    m_mipmaps(false),
    m_opengl_id(0),
    m_format(GL_INVALID_ENUM),
    m_type(GL_INVALID_ENUM),
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

GLint Texture::Width() const
{
    return m_width;
}

GLint Texture::Height() const
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

GLint Texture::DefaultWidth() const
{
    return m_default_width;
}

GLint Texture::DefaultHeight() const
{
    return m_default_height;
}

void Texture::OrthoBlit(const Pt& pt1, const Pt& pt2, const GLfloat* tex_coords/* = 0*/) const
{
    if (m_opengl_id) {
        if (!tex_coords) // use default texture coords when not given any others
            tex_coords = m_tex_coords;

        glBindTexture(GL_TEXTURE_2D, m_opengl_id);

        // HACK! This code ensures that unscaled textures are reproduced exactly, even
        // though they theoretically should be even when using non-GL_NEAREST* scaling.
        bool render_scaled = (pt2.x - pt1.x) != m_default_width || (pt2.y - pt1.y) != m_default_height;
        bool need_min_filter_change = !render_scaled && m_min_filter != GL_NEAREST;
        bool need_mag_filter_change = !render_scaled && m_mag_filter != GL_NEAREST;
        if (need_min_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (need_mag_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // render texture
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(tex_coords[0], tex_coords[1]); glVertex2i(pt1.x, pt1.y);
        glTexCoord2f(tex_coords[2], tex_coords[1]); glVertex2i(pt2.x, pt1.y);
        glTexCoord2f(tex_coords[0], tex_coords[3]); glVertex2i(pt1.x, pt2.y);
        glTexCoord2f(tex_coords[2], tex_coords[3]); glVertex2i(pt2.x, pt2.y);
        glEnd();

        if (need_min_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        if (need_mag_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    }
}

void Texture::OrthoBlit(const Pt& pt) const
{
    OrthoBlit(pt, pt + Pt(m_default_width, m_default_height), m_tex_coords);
}

void Texture::Load(const std::string& filename, bool mipmap/* = false*/)
{
    if (m_opengl_id)
        Clear();

    TextureManager::InitDevIL();

    ILuint id, error;
    ilGenImages(1, &id);
    CheckILErrors("ilGenImages(1, &id)");
    ilBindImage(id);
    CheckILErrors("ilBindImage(id)");
    ilLoadImage(const_cast<char*>(filename.c_str()));
    if ((error = ilGetError()) != IL_NO_ERROR) {
        std::string call = "ilLoadImage(\"" + filename + "\")";
        if (VERBOSE_DEVIL_ERROR_REPORTING) {
            std::cerr << "IL call \"" << call << "\" failed with IL error \"" << iluErrorString(error)
                      << "\" (code " << error << ")\n";
        }
        CheckILErrors(call);
        throw BadFile("Could not load temporary DevIL image from file \'" + filename + "\'");
    }

    m_filename = filename;
    m_default_width = ilGetInteger(IL_IMAGE_WIDTH);
    CheckILErrors("ilGetInteger(IL_IMAGE_WIDTH)");
    m_default_height = ilGetInteger(IL_IMAGE_HEIGHT);
    CheckILErrors("ilGetInteger(IL_IMAGE_HEIGHT)");
    m_bytes_pp = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
    CheckILErrors("ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL)");
    m_format = ilGetInteger(IL_IMAGE_FORMAT);
    CheckILErrors("ilGetInteger(IL_IMAGE_FORMAT)");
    if (m_format == IL_COLOR_INDEX) {
        m_format = IL_RGBA;
        m_type = IL_UNSIGNED_BYTE;
        m_bytes_pp = 4;
        ilConvertImage(m_format, m_type);
        CheckILErrors("ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)");
    } else {
        m_type = ilGetInteger(IL_IMAGE_TYPE);
        CheckILErrors("ilGetInteger(IL_IMAGE_TYPE)");
    }
    ILubyte* image_data = ilGetData();
    CheckILErrors("ilGetData()");
    Init(m_default_width, m_default_height, image_data, m_format, m_type, m_bytes_pp, mipmap);

    ilDeleteImages(1, &id);
    CheckILErrors("ilDeleteImages(1, &id)");
}

void Texture::Init(int x, int y, int width, int height, int image_width, const unsigned char* image, GLenum format, GLenum type, int bytes_per_pixel, bool mipmap/* = false*/)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, false);
    glPixelStorei(GL_UNPACK_LSB_FIRST, false);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image_width);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    try {
        InitFromRawData(width, height, image, format, type, bytes_per_pixel, mipmap);
    } catch (...) {
        glPopClientAttrib();
        throw;
    }

    glPopClientAttrib();
}

void Texture::Init(int width, int height, const unsigned char* image, GLenum format, GLenum type, int bytes_per_pixel, bool mipmap/* = false*/)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, false);
    glPixelStorei(GL_UNPACK_LSB_FIRST, false);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    try {
        InitFromRawData(width, height, image, format, type, bytes_per_pixel, mipmap);
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
    m_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    m_mag_filter = GL_LINEAR;

    m_mipmaps = false;
    m_opengl_id = 0;
    m_format = GL_INVALID_ENUM;
    m_type = GL_INVALID_ENUM;

    m_tex_coords[0] = m_tex_coords[1] = 0.0f;   // min x, y
    m_tex_coords[2] = m_tex_coords[3] = 1.0f;   // max x, y
}

void Texture::InitFromRawData(int width, int height, const unsigned char* image, GLenum format, GLenum type, int bytes_per_pixel, bool mipmap)
{
    if (!image)
        return;

    if (m_opengl_id)
        Clear();

    int GL_texture_width = PowerOfTwo(width);
    int GL_texture_height = PowerOfTwo(height);

    glGenTextures(1, &m_opengl_id);
    glBindTexture(GL_TEXTURE_2D, m_opengl_id);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);

    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, format, GL_texture_width, GL_texture_height, 0, format, type, image);
    GLint checked_format;
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &checked_format);
    if (!checked_format)
        throw InsufficientResources("Insufficient resources to create requested OpenGL texture");
    bool image_is_power_of_two = width == GL_texture_width && height == GL_texture_height;
    if (image_is_power_of_two) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, image);
    } else {
        std::vector<unsigned char> zero_data(bytes_per_pixel * GL_texture_width * GL_texture_height);
        glTexImage2D(GL_TEXTURE_2D, 0, format, GL_texture_width, GL_texture_height, 0, format, type, &zero_data[0]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, image);
    }

    m_mipmaps = mipmap;
    m_default_width = width;
    m_default_height = height;
    m_bytes_pp = bytes_per_pixel;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_height);
    m_tex_coords[2] = m_default_width / double(m_width);
    m_tex_coords[3] = m_default_height / double(m_height);

    if (mipmap) {
        std::auto_ptr<unsigned char> image_copy;
        if (!image_is_power_of_two)
            image_copy.reset(GetRawBytes());
        unsigned char* image_to_use = image_copy.get() ? image_copy.get() : const_cast<unsigned char*>(image);
        gluBuild2DMipmaps(GL_PROXY_TEXTURE_2D, format, GL_texture_width, GL_texture_height, format, type, image_to_use);
        GLint checked_format;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &checked_format);
        if (!checked_format)
            throw InsufficientResources("Insufficient resources to create requested mipmapped OpenGL texture");
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, GL_texture_width, GL_texture_height, format, type, image_to_use);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }
}

unsigned char* Texture::GetRawBytes()
{
    unsigned char* retval = 0;
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
    glGetTexImage(GL_TEXTURE_2D, 0, m_format, m_type, retval);
    glPopClientAttrib();
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

SubTexture::SubTexture(const boost::shared_ptr<const Texture>& texture, int x1, int y1, int x2, int y2) :
    m_texture(texture),
    m_width(x2 - x1),
    m_height(y2 - y1),
    m_tex_coords()
{
    if (!m_texture) throw BadTexture("Attempted to contruct subtexture from invalid texture");
    if (x2 < x1 || y2 < y1) throw InvalidTextureCoordinates("Attempted to contruct subtexture from invalid coordinates");

    m_tex_coords[0] = static_cast<double>(x1) / texture->Width();
    m_tex_coords[1] = static_cast<double>(y1) / texture->Height();
    m_tex_coords[2] = static_cast<double>(x2) / texture->Width();
    m_tex_coords[3] = static_cast<double>(y2) / texture->Height();
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

GLint SubTexture::Width() const
{
    return m_width;
}

GLint SubTexture::Height() const
{
    return m_height;
}

const Texture* SubTexture::GetTexture() const
{
    return m_texture.get();
}

void SubTexture::OrthoBlit(const Pt& pt1, const Pt& pt2) const
{
    if (m_texture) m_texture->OrthoBlit(pt1, pt2, m_tex_coords);
}

void SubTexture::OrthoBlit(const Pt& pt) const
{
    if (m_texture) m_texture->OrthoBlit(pt, pt + Pt(m_width, m_height), m_tex_coords);
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

boost::shared_ptr<Texture> TextureManager::StoreTexture(const boost::shared_ptr<Texture>& texture, const std::string& texture_name)
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
        s_il_initialized = true;
    }
}

boost::shared_ptr<Texture> TextureManager::LoadTexture(const std::string& filename, bool mipmap/* = false*/)
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
