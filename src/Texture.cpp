/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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
   whatwasthataddress@gmail.com */

#include <GG/Texture.h>

#include <GG/GUI.h>
#include <GG/Config.h>
#include <GG/DrawUtil.h>

#if GG_USE_DEVIL_IMAGE_LOAD_LIBRARY
# include <IL/il.h>
# include <IL/ilu.h>
#else
# include <boost/filesystem/operations.hpp>
# include "GIL/extension/dynamic_image/any_image.hpp"
# if GG_HAVE_LIBJPEG
#  include "GIL/extension/io/jpeg_dynamic_io.hpp"
# endif
# if GG_HAVE_LIBPNG
#  include "GIL/extension/io/png_dynamic_io.hpp"
# endif
# if GG_HAVE_LIBTIFF
#  include "GIL/extension/io/tiff_dynamic_io.hpp"
# endif
# include <boost/algorithm/string/case_conv.hpp>
#endif

#include <iostream>
#include <iomanip>


using namespace GG;

namespace {
    const bool VERBOSE_DEVIL_ERROR_REPORTING = true;

    template <class T>
    T PowerOfTwo(T input)
    {
        T value(1);
        while (value < input)
            value *= 2;
        return value;
    }

#if GG_USE_DEVIL_IMAGE_LOAD_LIBRARY
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

    bool g_il_initialized = false;
    void InitDevIL()
    {
        if (!g_il_initialized) {
            // ensure we're starting with an empty error stack
            while (ilGetError() != IL_NO_ERROR) ;
            ilInit();
            CheckILErrors("ilInit()");
            iluInit();
            CheckILErrors("iluInit()");
            g_il_initialized = true;
        }
    }
#endif
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
{ Clear(); }

Texture::~Texture()
{ Clear(); }

std::string Texture::Filename() const
{ return m_filename; }

GLenum Texture::WrapS() const
{ return m_wrap_s; }

GLenum Texture::WrapT() const
{ return m_wrap_t; }

GLenum Texture::MinFilter() const
{ return m_min_filter; }

GLenum Texture::MagFilter() const
{ return m_mag_filter; }

unsigned int Texture::BytesPP() const
{ return m_bytes_pp; }

X Texture::Width() const
{ return m_width; }

Y Texture::Height() const
{ return m_height; }

bool Texture::MipMapped() const
{ return m_mipmaps; }

GLuint Texture::OpenGLId() const
{ return m_opengl_id; }

const GLfloat* Texture::DefaultTexCoords() const
{ return m_tex_coords; }

X Texture::DefaultWidth() const
{ return m_default_width; }

Y Texture::DefaultHeight() const
{ return m_default_height; }

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
        glTexCoord2f(tex_coords[0], tex_coords[1]);
        glVertex(pt1.x, pt1.y);
        glTexCoord2f(tex_coords[2], tex_coords[1]);
        glVertex(pt2.x, pt1.y);
        glTexCoord2f(tex_coords[0], tex_coords[3]);
        glVertex(pt1.x, pt2.y);
        glTexCoord2f(tex_coords[2], tex_coords[3]);
        glVertex(pt2.x, pt2.y);
        glEnd();

        if (need_min_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        if (need_mag_filter_change)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    }
}

void Texture::OrthoBlit(const Pt& pt) const
{ OrthoBlit(pt, pt + Pt(m_default_width, m_default_height), m_tex_coords); }

void Texture::Load(const std::string& filename, bool mipmap/* = false*/)
{
    if (m_opengl_id)
        Clear();

#if GG_USE_DEVIL_IMAGE_LOAD_LIBRARY

    InitDevIL();

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
        throw BadFile("Could not load temporary DevIL image from file \"" + filename + "\"");
    }

    m_filename = filename;
    m_default_width = X(ilGetInteger(IL_IMAGE_WIDTH));
    CheckILErrors("ilGetInteger(IL_IMAGE_WIDTH)");
    m_default_height = Y(ilGetInteger(IL_IMAGE_HEIGHT));
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

#else

    namespace gil = boost::gil;
    namespace fs = boost::filesystem;

    BOOST_STATIC_ASSERT((sizeof(gil::gray8_pixel_t) == 1));
    BOOST_STATIC_ASSERT((sizeof(gil::gray_alpha8_pixel_t) == 2));
    BOOST_STATIC_ASSERT((sizeof(gil::rgb8_pixel_t) == 3));
    BOOST_STATIC_ASSERT((sizeof(gil::rgba8_pixel_t) == 4));

    typedef boost::mpl::vector4<
        gil::gray8_image_t,
        gil::gray_alpha8_image_t,
        gil::rgb8_image_t,
        gil::rgba8_image_t
    > ImageTypes;
    typedef gil::any_image<ImageTypes> ImageType;

    fs::path path(filename);

    if (!fs::exists(path))
        throw BadFile("Texture file \"" + filename + "\" does not exist");

    if (!fs::is_regular_file(path))
        throw BadFile("Texture \"file\" \"" + filename + "\" is not a file");

#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    std::string extension = boost::algorithm::to_lower_copy(path.extension().string());
#else
    std::string extension = boost::algorithm::to_lower_copy(path.extension());
#endif

    ImageType image;
    try {
        // First attempt -- try just to read the file in one of the default
        // formats above.
#if GG_HAVE_LIBJPEG
        if (extension == ".jpg" || extension == ".jpe" || extension == ".jpeg")
            gil::jpeg_read_image(filename, image);
        else
#endif
#if GG_HAVE_LIBPNG
        if (extension == ".png")
            gil::png_read_image(filename, image);
        else
#endif
#if GG_HAVE_LIBTIFF
        if (extension == ".tif" || extension == ".tiff")
            gil::tiff_read_image(filename, image);
        else
#endif
            throw BadFile("Texture file \"" + filename + "\" does not have a supported file extension");
    } catch (const std::ios_base::failure &) {
        // Second attempt -- If *_read_image() throws, see if we can convert
        // the image to RGBA.  This is needed for color-indexed images.
#if GG_HAVE_LIBJPEG
        if (extension == ".jpg" || extension == ".jpe" || extension == ".jpeg") {
            gil::rgba8_image_t rgba_image;
            gil::jpeg_read_and_convert_image(filename, rgba_image);
            image.move_in(rgba_image);
        }
#endif
#if GG_HAVE_LIBPNG
        if (extension == ".png") {
            gil::rgba8_image_t rgba_image;
            gil::png_read_and_convert_image(filename, rgba_image);
            image.move_in(rgba_image);
        }
#endif
#if GG_HAVE_LIBTIFF
        if (extension == ".tif" || extension == ".tiff") {
            gil::rgba8_image_t rgba_image;
            gil::tiff_read_and_convert_image(filename, rgba_image);
            image.move_in(rgba_image);
        }
#endif
    }

    m_filename = filename;
    m_default_width = X(image.width());
    m_default_height = Y(image.height());
    m_type = GL_UNSIGNED_BYTE;

#define IF_IMAGE_TYPE_IS(image_prefix)                                  \
    if (image.current_type_is<image_prefix ## _image_t>()) {            \
        m_bytes_pp = sizeof(image_prefix ## _pixel_t);                  \
        image_data = interleaved_view_get_raw_data(                     \
            const_view(image._dynamic_cast<image_prefix ## _image_t>())); \
    }

    const unsigned char* image_data = 0;

    IF_IMAGE_TYPE_IS(gil::gray8)
    else IF_IMAGE_TYPE_IS(gil::gray_alpha8)
    else IF_IMAGE_TYPE_IS(gil::rgb8)
    else IF_IMAGE_TYPE_IS(gil::rgba8)

#undef IF_IMAGE_TYPE_IS

    switch (m_bytes_pp) {
    case 1:  m_format = GL_LUMINANCE; break;
    case 2:  m_format = GL_LUMINANCE_ALPHA; break;
    case 3:  m_format = GL_RGB; break;
    case 4:  m_format = GL_RGBA; break;
    default: throw BadFile("Texture file \"" + filename + "\" does not have a supported number of color channels (1-4)");
    }

    assert(image_data);
    Init(m_default_width, m_default_height, image_data, m_format, m_type, m_bytes_pp, mipmap);

#endif
}

void Texture::Init(X x, Y y, X width, Y height, X image_width, const unsigned char* image,
                   GLenum format, GLenum type, unsigned int bytes_per_pixel, bool mipmap/* = false*/)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, false);
    glPixelStorei(GL_UNPACK_LSB_FIRST, false);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, Value(image_width));
    glPixelStorei(GL_UNPACK_SKIP_ROWS, Value(y));
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, Value(x));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    try {
        InitFromRawData(width, height, image, format, type, bytes_per_pixel, mipmap);
    } catch (...) {
        glPopClientAttrib();
        throw;
    }

    glPopClientAttrib();
}

void Texture::Init(X width, Y height, const unsigned char* image, GLenum format, GLenum type,
                   unsigned int bytes_per_pixel, bool mipmap/* = false*/)
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
    m_default_width = m_width = X0;
    m_default_height = m_height = Y0;

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

void Texture::InitFromRawData(X width, Y height, const unsigned char* image, GLenum format, GLenum type,
                              unsigned int bytes_per_pixel, bool mipmap)
{
    if (!image)
        return;

    if (m_opengl_id)
        Clear();

    X GL_texture_width = PowerOfTwo(width);
    Y GL_texture_height = PowerOfTwo(height);

    glGenTextures(1, &m_opengl_id);
    glBindTexture(GL_TEXTURE_2D, m_opengl_id);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);

    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, format, Value(GL_texture_width), Value(GL_texture_height), 0, format, type, image);
    GLint checked_format;
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &checked_format);
    if (!checked_format)
        throw InsufficientResources("Insufficient resources to create requested OpenGL texture");
    bool image_is_power_of_two = width == GL_texture_width && height == GL_texture_height;
    if (image_is_power_of_two) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, Value(width), Value(height), 0, format, type, image);
    } else {
        std::vector<unsigned char> zero_data(bytes_per_pixel * Value(GL_texture_width) * Value(GL_texture_height));
        glTexImage2D(GL_TEXTURE_2D, 0, format, Value(GL_texture_width), Value(GL_texture_height), 0, format, type, &zero_data[0]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Value(width), Value(height), format, type, image);
    }

    m_mipmaps = mipmap;
    m_default_width = width;
    m_default_height = height;
    m_bytes_pp = bytes_per_pixel;
    {
        GLint w, h;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        m_width = X(w);
        m_height = Y(h);
    }
    m_tex_coords[2] = Value(1.0 * m_default_width / m_width);
    m_tex_coords[3] = Value(1.0 * m_default_height / m_height);

    if (mipmap) {
        boost::scoped_array<unsigned char> image_copy;
        if (!image_is_power_of_two)
            image_copy.reset(GetRawBytes());
        unsigned char* image_to_use = image_copy ? image_copy.get() : const_cast<unsigned char*>(image);
        gluBuild2DMipmaps(GL_PROXY_TEXTURE_2D, format, Value(GL_texture_width), Value(GL_texture_height), format, type, image_to_use);
        GLint checked_format;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &checked_format);
        if (!checked_format)
            throw InsufficientResources("Insufficient resources to create requested mipmapped OpenGL texture");
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, Value(GL_texture_width), Value(GL_texture_height), format, type, image_to_use);
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
    retval = new uchar[Value(m_width) * Value(m_height) * m_bytes_pp];
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
{}

SubTexture::SubTexture(const boost::shared_ptr<const Texture>& texture, X x1, Y y1, X x2, Y y2) :
    m_texture(texture),
    m_width(x2 - x1),
    m_height(y2 - y1),
    m_tex_coords()
{
    if (!m_texture) throw BadTexture("Attempted to contruct subtexture from invalid texture");
    if (x2 < x1 || y2 < y1) throw InvalidTextureCoordinates("Attempted to contruct subtexture from invalid coordinates");

    m_tex_coords[0] = Value(x1 * 1.0 / texture->Width());
    m_tex_coords[1] = Value(y1 * 1.0 / texture->Height());
    m_tex_coords[2] = Value(x2 * 1.0 / texture->Width());
    m_tex_coords[3] = Value(y2 * 1.0 / texture->Height());
}

SubTexture::~SubTexture()
{}

SubTexture::SubTexture(const SubTexture& rhs)
{ *this = rhs; }

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
{ return !m_texture; }

const GLfloat* SubTexture::TexCoords() const
{ return m_tex_coords; }

X SubTexture::Width() const
{ return m_width; }

Y SubTexture::Height() const
{ return m_height; }

const Texture* SubTexture::GetTexture() const
{ return m_texture.get(); }

void SubTexture::OrthoBlit(const Pt& pt1, const Pt& pt2) const
{ if (m_texture) m_texture->OrthoBlit(pt1, pt2, m_tex_coords); }

void SubTexture::OrthoBlit(const Pt& pt) const
{ if (m_texture) m_texture->OrthoBlit(pt, pt + Pt(m_width, m_height), m_tex_coords); }

bool SubTexture::operator==(const SubTexture& rhs) const
{
    return
        m_texture == rhs.m_texture &&
        m_width == rhs.m_width &&
        m_height == rhs.m_height &&
        m_tex_coords[0] == rhs.m_tex_coords[0] &&
        m_tex_coords[1] == rhs.m_tex_coords[1] &&
        m_tex_coords[2] == rhs.m_tex_coords[2] &&
        m_tex_coords[3] == rhs.m_tex_coords[3];
}

bool SubTexture::operator!=(const SubTexture& rhs) const
{ return !(*this == rhs); }


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
{ return (m_textures[texture_name] = texture); }

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
