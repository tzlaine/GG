/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_CURSOR_HPP
#define ADOBE_CURSOR_HPP

#include <boost/shared_ptr.hpp>


namespace GG {
    class Cursor;
}

/****************************************************************************************************/

typedef boost::shared_ptr<GG::Cursor> adobe_cursor_t;

// TODO: Move to StyleFactory.
adobe_cursor_t pointer_cursor();
adobe_cursor_t help_cursor();
adobe_cursor_t crosshair_cursor();
adobe_cursor_t move_cursor();
adobe_cursor_t link_cursor();
adobe_cursor_t grabable_cursor();
adobe_cursor_t grabbing_cursor();
adobe_cursor_t text_cursor();
adobe_cursor_t resize_left_right_cursor();
adobe_cursor_t resize_up_down_cursor();
adobe_cursor_t resize_ul_lr_cursor();
adobe_cursor_t resize_ll_ur_cursor();
adobe_cursor_t zoom_in_cursor();
adobe_cursor_t zoom_out_cursor();
adobe_cursor_t drop_cursor();
adobe_cursor_t disallow_cursor();


/****************************************************************************************************/

// ADOBE_CURSOR_HPP
#endif

/****************************************************************************************************/
