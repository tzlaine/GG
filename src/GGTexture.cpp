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
#include <XMLValidators.h>

#include <IL/il.h>

/* some versions of libDevIL are linked with Allegro, which requires
   _mangled_main_address to be defined. Futhermore, ilut.h includes
   allegro.h as extern "C", which is wrong, because allegro defines
   some C++-classes if it detects a C++ compiler. So, we need to
   include allegro before including ilut!
 
   If GG_NO_ALLEGRO_HACK is not defined, we include an
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

namespace GG {

///////////////////////////////////////
// class GG::Texture
///////////////////////////////////////
Texture::Texture() :
    m_opengl_id(0)
{
    Clear();
}

Texture::Texture(const XMLElement& elem) :
    m_opengl_id(0)
{
    if (elem.Tag() != "GG::Texture")
        throw std::invalid_argument("Attempted to construct a GG::Texture from an XMLElement that had a tag other than \"GG::Texture\"");

    Clear();

    m_filename = elem.Child("m_filename").Text();
    m_bytes_pp = lexical_cast<int>(elem.Child("m_bytes_pp").Text());
    m_width = lexical_cast<int>(elem.Child("m_width").Text());
    m_height = lexical_cast<int>(elem.Child("m_height").Text());
    m_wrap_s = lexical_cast<GLenum>(elem.Child("m_wrap_s").Text());
    m_wrap_t = lexical_cast<GLenum>(elem.Child("m_wrap_t").Text());
    m_min_filter = lexical_cast<GLenum>(elem.Child("m_min_filter").Text());
    m_mag_filter = lexical_cast<GLenum>(elem.Child("m_mag_filter").Text());
    m_mipmaps = lexical_cast<bool>(elem.Child("m_mipmaps").Text());

    const XMLElement* curr_elem = &elem.Child("m_tex_coords");
    m_tex_coords[0] = lexical_cast<GLfloat>(curr_elem->Attribute("u1"));
    m_tex_coords[1] = lexical_cast<GLfloat>(curr_elem->Attribute("v1"));
    m_tex_coords[2] = lexical_cast<GLfloat>(curr_elem->Attribute("u2"));
    m_tex_coords[3] = lexical_cast<GLfloat>(curr_elem->Attribute("v2"));

    m_default_width = lexical_cast<int>(elem.Child("m_default_width").Text());
    m_default_height = lexical_cast<int>(elem.Child("m_default_height").Text());

    if (m_filename != "")
        Load(m_filename, m_mipmaps);

    SetWrap(m_wrap_s, m_wrap_t);
    SetFilters(m_min_filter, m_mag_filter);
}

Texture::~Texture()
{
    Clear();
}

Texture::Texture(const Texture& rhs)
{
    throw TextureException("Attempted to copy-construct a GG::Texture object");
}

Texture& Texture::operator=(const Texture& rhs)
{
    throw TextureException("Attempted to assign a GG::Texture object");
    return *this;
}

void Texture::OrthoBlit(int x1, int y1, int x2, int y2, const GLfloat* tex_coords/* = 0*/, bool enter_2d_mode/* = true*/) const
{
    if (m_opengl_id) {
        if (!tex_coords) // use default texture coords when not given any others
            tex_coords = m_tex_coords;

        if (enter_2d_mode) App::GetApp()->Enter2DMode(); // enter 2D mode, if needed

        // render texture
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(tex_coords[0], tex_coords[1]); glVertex2i(x1, y1);
        glTexCoord2f(tex_coords[2], tex_coords[1]); glVertex2i(x2, y1);
        glTexCoord2f(tex_coords[0], tex_coords[3]); glVertex2i(x1, y2);
        glTexCoord2f(tex_coords[2], tex_coords[3]); glVertex2i(x2, y2);
        glEnd();

        if (enter_2d_mode) App::GetApp()->Exit2DMode(); // exit 2D mode, if needed
    }
}

void Texture::OrthoBlit(int x, int y, bool enter_2d_mode/* = true*/) const
{
    int w = m_default_width ? m_default_width : m_width;
    int h = m_default_height ? m_default_height : m_height;
    OrthoBlit(x, y, x + w, y + h, m_tex_coords, enter_2d_mode);
}

XMLElement Texture::XMLEncode() const
{
    XMLElement retval("GG::Texture");
    retval.AppendChild(XMLElement("m_filename", m_filename));
    retval.AppendChild(XMLElement("m_bytes_pp", lexical_cast<string>(m_bytes_pp)));
    retval.AppendChild(XMLElement("m_width", lexical_cast<string>(m_width)));
    retval.AppendChild(XMLElement("m_height", lexical_cast<string>(m_height)));
    retval.AppendChild(XMLElement("m_wrap_s", lexical_cast<string>(m_wrap_s)));
    retval.AppendChild(XMLElement("m_wrap_t", lexical_cast<string>(m_wrap_t)));
    retval.AppendChild(XMLElement("m_min_filter", lexical_cast<string>(m_min_filter)));
    retval.AppendChild(XMLElement("m_mag_filter", lexical_cast<string>(m_mag_filter)));
    retval.AppendChild(XMLElement("m_mipmaps", lexical_cast<string>(m_mipmaps)));

    XMLElement temp("m_tex_coords");
    temp.SetAttribute("u1", lexical_cast<string>(m_tex_coords[0]));
    temp.SetAttribute("v1", lexical_cast<string>(m_tex_coords[1]));
    temp.SetAttribute("u2", lexical_cast<string>(m_tex_coords[2]));
    temp.SetAttribute("v2", lexical_cast<string>(m_tex_coords[3]));
    retval.AppendChild(temp);

    retval.AppendChild(XMLElement("m_default_width", lexical_cast<string>(m_default_width)));
    retval.AppendChild(XMLElement("m_default_height", lexical_cast<string>(m_default_height)));
    return retval;
}

XMLElementValidator Texture::XMLValidator() const
{
    XMLElementValidator retval("GG::Texture");
    retval.AppendChild(XMLElementValidator("m_filename"));
    retval.AppendChild(XMLElementValidator("m_bytes_pp", new RangedValidator<int>(1, 4)));
    retval.AppendChild(XMLElementValidator("m_width", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_height", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_wrap_s", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_wrap_t", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_min_filter", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_mag_filter", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_mipmaps", new Validator<bool>()));

    XMLElementValidator temp("m_tex_coords");
    temp.SetAttribute("u1", new Validator<float>());
    temp.SetAttribute("v1", new Validator<float>());
    temp.SetAttribute("u2", new Validator<float>());
    temp.SetAttribute("v2", new Validator<float>());
    retval.AppendChild(temp);

    retval.AppendChild(XMLElementValidator("m_default_width", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_default_height", new Validator<int>()));
    return retval;
}

void Texture::Load(const char* filename, bool mipmap/* = false*/)
{
    if (m_opengl_id)
        Clear();

    TextureManager::InitDevIL();

    ILuint id, error;
    ilGenImages(1, &id);
    ilBindImage(id);
    ilLoadImage(const_cast<char*>(filename));
    if ((error = ilGetError()) != IL_NO_ERROR)
        throw TextureException((string("Could not load temporary DevIL image from file \'") + filename) + "\'");
   
    if (mipmap)
        m_opengl_id = ilutGLBindMipmaps();
    else
        m_opengl_id = ilutGLBindTexImage();
    if (!m_opengl_id || (error = ilGetError()) != IL_NO_ERROR)
        throw TextureException((string("Could not create OpenGL texture object from file \'") + filename) + "\'");
   
    // be sure to record these
    m_filename = filename;
    m_mipmaps = mipmap;
    m_bytes_pp = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
    m_default_width = m_width = ilGetInteger(IL_IMAGE_WIDTH);
    m_default_height = m_height = ilGetInteger(IL_IMAGE_HEIGHT);
   
    ilDeleteImages(1, &id);
}

void Texture::Init(int x, int y, int width, int height, int image_width, const unsigned char* image, int channels, bool mipmap/* = false*/)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image_width);
    Init(width, height, image, channels, mipmap);
    glPopClientAttrib();
}

void Texture::Init(int width, int height, const unsigned char* image, Uint32 channels, bool mipmap/* = false*/)
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
        default: throw TextureException("Attempted to initialize a GG::Texture with an invalid number of color channels");
        }

        glGenTextures(1, &m_opengl_id);
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);                     // set this texture as the current opengl texture
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);   // set some relevant opengl texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);

        if (mipmap) { // creating mipmapped texture
            // check to see if this texture can be created with available resources
            gluBuild2DMipmaps(GL_PROXY_TEXTURE_2D, channels, width, height, mode, GL_UNSIGNED_BYTE, image);
            GLint format;
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
            if (format) // if yes, build it
                gluBuild2DMipmaps(GL_TEXTURE_2D, channels, width, height, mode, GL_UNSIGNED_BYTE, image);
            else // if no, throw
                throw TextureException("Insufficient resources to create requested mipmapped OpenGL texture");
        } else { // creating non-mipmapped texture
            glTexImage2D(GL_PROXY_TEXTURE_2D, 0, channels, width, height, 0, mode, GL_UNSIGNED_BYTE, image);
            GLint format;
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
            if (format)
                glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, mode, GL_UNSIGNED_BYTE, image);
            else
                throw TextureException("Insufficient resources to create requested OpenGL texture");
        }

        // be sure to record these
        m_mipmaps = mipmap;
        m_bytes_pp = channels;
        m_width = width;
        m_height = height;
    }
}

void Texture::SetWrap(GLenum s, GLenum t)
{
    m_wrap_s = s;
    m_wrap_t = t;
    if (m_opengl_id) {
        glPushAttrib(GL_TEXTURE_BIT);
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);
        glPopAttrib();
    }
}

void Texture::SetFilters(GLenum min, GLenum mag)
{
    m_min_filter = min;
    m_mag_filter = mag;
    if (m_opengl_id) {
        glPushAttrib(GL_TEXTURE_BIT);
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
        glPopAttrib();
    }
}

void Texture::Clear()
{
    if (m_opengl_id)
        glDeleteTextures(1, &m_opengl_id);

    m_filename = "";

    m_bytes_pp = 4;
    m_width = 0;
    m_height = 0;

    m_wrap_s = m_wrap_t = GL_REPEAT;
    m_min_filter = m_mag_filter = GL_NEAREST;

    m_mipmaps = false;

    m_opengl_id = 0;

    m_tex_coords[0] = m_tex_coords[1] = 0.0f;   // min x, y
    m_tex_coords[2] = m_tex_coords[3] = 1.0f;   // max x, y

    m_default_width = 0;
    m_default_height = 0;
}


///////////////////////////////////////
// class GG::SubTexture
///////////////////////////////////////
SubTexture::SubTexture() :
    m_width(0),
    m_height(0)
{
}

SubTexture::SubTexture(const Texture* texture, int x1, int y1, int x2, int y2) :
    m_texture(shared_ptr<const Texture>(texture)),
    m_width(x2 - x1),
    m_height(y2 - y1)
{
    if (!m_texture) throw SubTextureException("Attempted to contruct subtexture from invalid texture");
    if (x2 < x1 || y2 < y1) throw SubTextureException("Attempted to contruct subtexture from invalid coordinates");

    m_tex_coords[0] = static_cast<double>(x1) / texture->Width();
    m_tex_coords[1] = static_cast<double>(y1) / texture->Height();
    m_tex_coords[2] = static_cast<double>(x2) / texture->Width();
    m_tex_coords[3] = static_cast<double>(y2) / texture->Height();
}

SubTexture::SubTexture(const shared_ptr<const Texture>& texture, int x1, int y1, int x2, int y2) :
    m_texture(texture),
    m_width(x2 - x1),
    m_height(y2 - y1)
{
    if (!m_texture) throw SubTextureException("Attempted to contruct subtexture from invalid texture");
    if (x2 < x1 || y2 < y1) throw SubTextureException("Attempted to contruct subtexture from invalid coordinates");

    m_tex_coords[0] = static_cast<double>(x1) / texture->Width();
    m_tex_coords[1] = static_cast<double>(y1) / texture->Height();
    m_tex_coords[2] = static_cast<double>(x2) / texture->Width();
    m_tex_coords[3] = static_cast<double>(y2) / texture->Height();
}

SubTexture::SubTexture(const XMLElement& elem) :
    m_width(0),
    m_height(0)
{
    if (elem.Tag() != "GG::SubTexture")
        throw std::invalid_argument("Attempted to construct a GG::SubTexture from an XMLElement that had a tag other than \"GG::SubTexture\"");

    const XMLElement* curr_elem = &elem.Child("m_texture");
    string texture_filename = curr_elem->NumChildren() ? curr_elem->Child("GG::Texture").Child("m_filename").Text() : "";
    if (texture_filename != "") {
        // we need to ensure that the settings in the XML-encoded texture are preserved in the loaded texture; to do this:
        shared_ptr<Texture> temp_texture = App::GetApp()->GetTexture(texture_filename);
        // if no copy of this texture exists in the manager, GetTexture() will load it with default settings, and the
        // use_count will be 2: one for the shared_ptr in the manager, one for temp_texture.
        // if this is the case, dump the texture, reload it from the XML definition (which may have non-default settings),
        // and store the XML-loaded Texture in the manager.
        if (temp_texture.use_count() == 2) {
            temp_texture = shared_ptr<Texture>(new Texture(curr_elem->Child("GG::Texture")));
            App::GetApp()->FreeTexture(texture_filename);
            App::GetApp()->StoreTexture(temp_texture, texture_filename);
            m_texture = temp_texture;
        } else { // if this is not the case, we're not the first ones to load the texture, so just keep it.
            m_texture = temp_texture;
        }
    }

    if (m_texture) {
        m_width = lexical_cast<int>(elem.Child("m_width").Text());
        m_height = lexical_cast<int>(elem.Child("m_height").Text());

        curr_elem = &elem.Child("m_tex_coords");
        m_tex_coords[0] = lexical_cast<GLfloat>(curr_elem->Attribute("u1"));
        m_tex_coords[1] = lexical_cast<GLfloat>(curr_elem->Attribute("v1"));
        m_tex_coords[2] = lexical_cast<GLfloat>(curr_elem->Attribute("u2"));
        m_tex_coords[3] = lexical_cast<GLfloat>(curr_elem->Attribute("v2"));
    }
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

void SubTexture::OrthoBlit(int x1, int y1, int x2, int y2, bool enter_2d_mode/* = true*/) const
{
    if (m_texture) m_texture->OrthoBlit(x1, y1, x2, y2, m_tex_coords, enter_2d_mode);
}

void SubTexture::OrthoBlit(int x, int y, bool enter_2d_mode/* = true*/) const
{
    if (m_texture) m_texture->OrthoBlit(x, y, x + m_width, y + m_height, m_tex_coords, enter_2d_mode);
}

XMLElement SubTexture::XMLEncode() const
{
    XMLElement retval("GG::SubTexture");
    XMLElement temp;

    temp = XMLElement("m_texture");
    if (m_texture)
        temp.AppendChild(m_texture->XMLEncode());
    retval.AppendChild(temp);

    retval.AppendChild(XMLElement("m_width", lexical_cast<string>(m_width)));
    retval.AppendChild(XMLElement("m_height", lexical_cast<string>(m_height)));

    temp = XMLElement("m_tex_coords");
    temp.SetAttribute("u1", lexical_cast<string>(m_tex_coords[0]));
    temp.SetAttribute("v1", lexical_cast<string>(m_tex_coords[1]));
    temp.SetAttribute("u2", lexical_cast<string>(m_tex_coords[2]));
    temp.SetAttribute("v2", lexical_cast<string>(m_tex_coords[3]));
    retval.AppendChild(temp);

    return retval;
}

XMLElementValidator SubTexture::XMLValidator() const
{
    XMLElementValidator retval("GG::SubTexture");

    XMLElementValidator temp("m_texture");
    if (m_texture)
	temp.AppendChild(m_texture->XMLValidator());
    retval.AppendChild(temp);

    retval.AppendChild(XMLElementValidator("m_width", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_height", new Validator<int>()));

    temp = XMLElementValidator("m_tex_coords");
    temp.SetAttribute("u1", new Validator<float>());
    temp.SetAttribute("v1", new Validator<float>());
    temp.SetAttribute("u2", new Validator<float>());
    temp.SetAttribute("v2", new Validator<float>());
    retval.AppendChild(temp);

    return retval;
}


///////////////////////////////////////
// class GG::TextureManager
///////////////////////////////////////
// static member(s)
bool TextureManager::s_created = false;
bool TextureManager::s_il_initialized = false;

TextureManager::TextureManager()
{
    if (s_created)
        throw TextureManagerException("Attempted to create a second instance of GG::TextureManager");
    s_created = true;
}

shared_ptr<Texture> TextureManager::StoreTexture(Texture* texture, const string& texture_name)
{
    shared_ptr<Texture> temp(texture);
    return StoreTexture(temp, texture_name);
}

shared_ptr<Texture> TextureManager::StoreTexture(shared_ptr<Texture> texture, const string& texture_name)
{
    return (m_textures[texture_name] = texture);
}

shared_ptr<Texture> TextureManager::GetTexture(const string& name, bool mipmap/* = false*/)
{
    std::map<string, shared_ptr<Texture> >::iterator it = m_textures.find(name);
    if (it == m_textures.end()) { // if no such texture was found, attempt to load it now, using name as the filename
        return (m_textures[name] = LoadTexture(name, mipmap));
    } else { // otherwise, just return the texture we found
        return it->second;
    }
}

void TextureManager::FreeTexture(const string& name)
{
    std::map<string, shared_ptr<Texture> >::iterator it = m_textures.find(name);
    if (it != m_textures.end())
        m_textures.erase(it);
}

void TextureManager::InitDevIL()
{
    if (!s_il_initialized) {
        ilInit();
        iluInit();
        ilutInit();
        ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
        ilutRenderer(ILUT_OPENGL);
        GLint max_size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
        ilutSetInteger(ILUT_MAXTEX_WIDTH, max_size);
        ilutSetInteger(ILUT_MAXTEX_HEIGHT, max_size);
        s_il_initialized = true;
    }
}

shared_ptr<Texture> TextureManager::LoadTexture(const string& filename, bool mipmap)
{
    shared_ptr<Texture> temp(new Texture());
    temp->Load(filename, mipmap);
    return (m_textures[filename] = temp);
}

} // namespace GG

