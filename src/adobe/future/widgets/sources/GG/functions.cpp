/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2011 T. Zachary Laine

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

#include <GG/EveGlue.h>
#include <GG/GUI.h>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>


namespace {

#define CASE(x) (name == adobe::static_name_t(#x)) retval = x

    GLenum name_to_min_filter(adobe::name_t name)
    {
        GLenum retval = GL_NEAREST_MIPMAP_LINEAR;
        if CASE(GL_NEAREST);
        else if CASE(GL_LINEAR);
        else if CASE(GL_NEAREST_MIPMAP_NEAREST);
        else if CASE(GL_LINEAR_MIPMAP_NEAREST);
        else if CASE(GL_NEAREST_MIPMAP_LINEAR);
        else if CASE(GL_LINEAR_MIPMAP_LINEAR);
        else if (name)
            throw std::runtime_error(adobe::make_string("Invalid OpenGL minification filter ", name.c_str()));
        return retval;
    }

    GLenum name_to_mag_filter(adobe::name_t name)
    {
        GLenum retval = GL_LINEAR;
        if CASE(GL_NEAREST);
        else if CASE(GL_LINEAR);
        else if (name)
            throw std::runtime_error(adobe::make_string("Invalid OpenGL magnification filter ", name.c_str()));
        return retval;
    }

    GLenum name_to_wrap(adobe::name_t name)
    {
        GLenum retval = GL_REPEAT;
        if CASE(GL_CLAMP);
        else if CASE(GL_CLAMP_TO_BORDER);
        else if CASE(GL_CLAMP_TO_EDGE);
        else if CASE(GL_MIRRORED_REPEAT);
        else if CASE(GL_REPEAT);
        else if (name)
            throw std::runtime_error(adobe::make_string("Invalid OpenGL wrap mode ", name.c_str()));
        return retval;
    }

#undef CASE

    adobe::any_regular_t texture(const adobe::dictionary_t& named_argument_set)
    {
        if (named_argument_set.empty())
            return adobe::any_regular_t(adobe::empty_t());

        std::string filename;
        adobe::dictionary_t wrap;
        adobe::dictionary_t filter;
        bool mipmap(false);

        get_value(named_argument_set, adobe::key_name, filename);
        get_value(named_argument_set, adobe::static_name_t("filter"), filter);
        get_value(named_argument_set, adobe::key_wrap, wrap);
        get_value(named_argument_set, adobe::static_name_t("mipmap"), mipmap);

        adobe::name_t wrap_s;
        adobe::name_t wrap_t;
        adobe::name_t min_filter;
        adobe::name_t mag_filter;

        get_value(wrap, adobe::static_name_t("s"), filename);
        get_value(wrap, adobe::static_name_t("t"), filename);
        get_value(filter, adobe::static_name_t("min"), filename);
        get_value(filter, adobe::static_name_t("mag"), filename);

        boost::shared_ptr<GG::Texture> texture;
        if (!filename.empty()) {
            try {
                texture = GG::GUI::GetGUI()->GetTexture(filename, mipmap);
                if (wrap_s || wrap_t)
                    texture->SetWrap(name_to_wrap(wrap_s), name_to_wrap(wrap_t));
                if (min_filter || mag_filter)
                    texture->SetFilters(name_to_min_filter(min_filter), name_to_mag_filter(mag_filter));
            } catch (...) {
                return adobe::any_regular_t(adobe::empty_t());
            }
        }

        return adobe::any_regular_t(texture);
    }

    adobe::any_regular_t subtexture(const adobe::dictionary_t& named_argument_set)
    {
        if (named_argument_set.empty())
            return adobe::any_regular_t(adobe::empty_t());

        std::string texture_name;
        boost::shared_ptr<GG::Texture> texture;
        int x1;
        int x2;
        int y1;
        int y2;

        bool all_needed_args = true;

        if (get_value(named_argument_set, adobe::static_name_t("texture"), texture_name)) {
            try {
                texture = GG::GUI::GetGUI()->GetTexture(texture_name);
            } catch (...) {}
        } else {
            get_value(named_argument_set, adobe::static_name_t("texture"), texture);
        }
        all_needed_args &= get_value(named_argument_set, adobe::static_name_t("x1"), x1);
        all_needed_args &= get_value(named_argument_set, adobe::static_name_t("y1"), y1);
        all_needed_args &= get_value(named_argument_set, adobe::static_name_t("x2"), x2);
        all_needed_args &= get_value(named_argument_set, adobe::static_name_t("y2"), y2);

        if (texture && all_needed_args)
            return adobe::any_regular_t(GG::SubTexture(texture, GG::X(x1), GG::Y(y1), GG::X(x2), GG::Y(y2)));
        else
            return adobe::any_regular_t(adobe::empty_t());
    }

    bool register_()
    {
        GG::RegisterDictionaryFunction(adobe::static_name_t("texture"), &texture);
        GG::RegisterDictionaryFunction(adobe::static_name_t("subtexture"), &subtexture);

        return true;
    }
    bool dummy = register_();

}
