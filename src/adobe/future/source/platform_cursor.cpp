/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/cursor.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/source/cursor_stack.hpp>
#include <GG/adobe/memory.hpp>

#include <GG/Cursor.h>
#include <GG/GUI.h>


namespace {
    adobe_cursor_t g_default_cursor;
    bool g_default_established = false;

    boost::shared_ptr<GG::Texture> default_cursor_texture()
    {
        // TODO: Create a uu-encocded array that can be used to construct a
        // cursors image, a la DefaultFont.cpp.
        return GG::GUI::GetGUI()->GetTexture("cursors.png");
    }

    adobe_cursor_t get_cursor(unsigned int row,
                              unsigned int column,
                              unsigned int hotspot_x,
                              unsigned int hotspot_y)
    {
        boost::shared_ptr<GG::Texture> texture = default_cursor_texture();
        const unsigned int cursor_size = 32u;
        return adobe_cursor_t(
            new GG::TextureCursor(
                GG::SubTexture(texture,
                               GG::X(row * cursor_size),
                               GG::Y(column * cursor_size),
                               GG::X((row + 1) * cursor_size),
                               GG::Y((column + 1) * cursor_size)),
                GG::Pt(GG::X(hotspot_x), GG::Y(hotspot_y))
            )
        );
    }
}

/****************************************************************************************************/

adobe_cursor_t make_cursor(const char* cursor_path,
                           float hot_spot_x,
                           float hot_spot_y)
{
    if (!g_default_cursor)
        g_default_cursor = GG::GUI::GetGUI()->GetCursor();
    boost::shared_ptr<GG::Texture> texture =
        GG::GUI::GetGUI()->GetTexture(cursor_path);
    GG::Pt hotspot(GG::X_d(hot_spot_x + 0.5), GG::Y_d(hot_spot_y + 0.5));
    return adobe_cursor_t(new GG::TextureCursor(texture, hotspot));
}

/****************************************************************************************************/

adobe_cursor_t make_cursor(const char* cursor_path,
                           unsigned int subtexture_ul_x,
                           unsigned int subtexture_ul_y,
                           unsigned int subtexture_lr_x,
                           unsigned int subtexture_lr_y,
                           float hot_spot_x,
                           float hot_spot_y)
{
    if (!g_default_cursor)
        g_default_cursor = GG::GUI::GetGUI()->GetCursor();
    boost::shared_ptr<GG::Texture> texture =
        GG::GUI::GetGUI()->GetTexture(cursor_path);
    GG::SubTexture subtexture(texture,
                              GG::X(subtexture_ul_x),
                              GG::Y(subtexture_ul_y),
                              GG::X(subtexture_lr_x),
                              GG::Y(subtexture_lr_y));
    GG::Pt hotspot(GG::X_d(hot_spot_x + 0.5), GG::Y_d(hot_spot_y + 0.5));
    return adobe_cursor_t(new GG::TextureCursor(subtexture, hotspot));
}

/****************************************************************************************************/

void delete_cursor(adobe_cursor_t cursor)
{}

/****************************************************************************************************/

void push_cursor(adobe_cursor_t cursor)
{
    if (!g_default_established) {
        g_default_cursor = GG::GUI::GetGUI()->GetCursor();
        g_default_established = true;
    }
    if (cursor) {
        cursor_stack_push(cursor);
        GG::GUI::GetGUI()->SetCursor(cursor);
    }
}

/****************************************************************************************************/

adobe_cursor_t pop_cursor()
{
    adobe_cursor_t old_cursor = cursor_stack_top();
    cursor_stack_pop();
    adobe_cursor_t new_cursor = cursor_stack_top();
    GG::GUI::GetGUI()->SetCursor(new_cursor ? new_cursor : g_default_cursor);
    return old_cursor;
}

/****************************************************************************************************/

void reset_cursor()
{
    cursor_stack_reset();
    GG::GUI::GetGUI()->SetCursor(g_default_cursor);
}

/*************************************************************************************************/

#define DEFINE_CURSOR_FUNCTION(name, row, column, hotspot_x, hotspot_y) \
    const adobe_cursor_t name()                                         \
    {                                                                   \
        static adobe_cursor_t retval;                                   \
        static bool inited(false);                                      \
        if (!inited) {                                                  \
            inited = true;                                              \
            retval = get_cursor(row, column, hotspot_x, hotspot_y);     \
        }                                                               \
        return retval;                                                  \
    }

DEFINE_CURSOR_FUNCTION(pointer_cursor, 0, 0, 3, 4)

DEFINE_CURSOR_FUNCTION(help_cursor, 0, 1, 3, 4)

DEFINE_CURSOR_FUNCTION(crosshair_cursor, 0, 2, 15, 15)

DEFINE_CURSOR_FUNCTION(move_cursor, 0, 3, 15, 15)

DEFINE_CURSOR_FUNCTION(link_cursor, 1, 0, 3, 11)

DEFINE_CURSOR_FUNCTION(grabable_cursor, 1, 1, 15, 12)

DEFINE_CURSOR_FUNCTION(grabbing_cursor, 1, 2, 14, 12)

DEFINE_CURSOR_FUNCTION(text_cursor, 1, 3, 12, 14)

DEFINE_CURSOR_FUNCTION(resize_left_right_cursor, 2, 0, 16, 16)

DEFINE_CURSOR_FUNCTION(resize_up_down_cursor, 2, 1, 16, 17)

DEFINE_CURSOR_FUNCTION(resize_ul_lr_cursor, 2, 2, 16, 16)

DEFINE_CURSOR_FUNCTION(resize_ll_ur_cursor, 2, 3, 16, 17)

DEFINE_CURSOR_FUNCTION(zoom_in_cursor, 3, 0, 14, 15)

DEFINE_CURSOR_FUNCTION(zoom_out_cursor, 3, 1, 14, 15)

DEFINE_CURSOR_FUNCTION(drop_cursor, 3, 2, 15, 15)

DEFINE_CURSOR_FUNCTION(disallow_cursor, 3, 3, 16, 16)

#undef DEFINE_CURSOR_FUNCTION
