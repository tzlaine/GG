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

/** \file GGWnd.h
    Contains the Wnd class, upon which all GG GUI elements are based. */

#ifndef _GGColorDlg_h_
#define _GGColorDlg_h_

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

namespace GG {

class Font;
class Slider;
    
/** contains the necessary data to represent a color in HSV space, with an alpha value thrown in to make conversions to
    and from GG::Clr possible. */
struct GG_API HSVClr
{
    HSVClr(); ///< ctor
    HSVClr(double h_, double s_, double v_, Uint8 a_ = 255); ///< ctor
    double h;   ///< hue
    double s;   ///< saturation
    double v;   ///< value
    Uint8  a;   ///< alpha
};

/** a control specifically designed for ColorDlg that allows the user to select a point in the Hue-Saturation subspace
    of the HSV color space. */
class GG_API HueSaturationPicker : public Control
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (double, double)> ChangedSignalType; ///< emitted whenever the hue or saturation in the picker changes
    //@}

    /** \name Slot Types */ //@{
    typedef ChangedSignalType::slot_type         ChangedSlotType;   ///< type of functor(s) invoked on a ChangedSignalType
    //@}

    /** \name Structors */ //@{
    HueSaturationPicker(int x, int y, int w, int h); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LButtonDown(const Pt& pt, Uint32 keys);
    virtual void LDrag(const Pt& pt, const Pt& move, Uint32 keys);

    void SetHueSaturation(double hue, double saturation); ///< sets the current hue and saturation.  Note that this does not cause a signal to be emitted.
    //@}

    mutable ChangedSignalType ChangedSignal;

private:
    void SetHueSaturationFromPt(Pt pt);

    double m_hue;
    double m_saturation;
    std::vector<std::vector<std::pair<double, double> > >  m_vertices;
    std::vector<std::vector<Clr> > m_colors;
};


/** a control specifically designed for ColorDlg that allows the user to select a point in the Value subspace of the HSV
    color space. */
class GG_API ValuePicker : public Control
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (double)> ChangedSignalType; ///< emitted whenever the hue or saturation in the picker changes
    //@}

    /** \name Slot Types */ //@{
    typedef ChangedSignalType::slot_type ChangedSlotType;   ///< type of functor(s) invoked on a ChangedSignalType
    //@}

    /** \name Structors */ //@{
    ValuePicker(int x, int y, int w, int h, Clr arrow_color); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LButtonDown(const Pt& pt, Uint32 keys);
    virtual void LDrag(const Pt& pt, const Pt& move, Uint32 keys);

    /** sets the current hue and saturation.  These are only used to render the control, and do not otherwise influence
        its operation. */
    void SetHueSaturation(double hue, double saturation);
    void SetValue(double value); ///< sets the current value.  Note that this does not cause a signal to be emitted.
    //@}

    mutable ChangedSignalType ChangedSignal;

private:
    void SetValueFromPt(Pt pt);

    double  m_hue;
    double  m_saturation;
    double  m_value;

    Clr m_arrow_color;
};


/** a dialog box used to get a color selection from the user.  The user may select a certain number of custom colors,
    which will remain available for the duration of that run of the application in the ColorDlg's static space.  If
    desired, an optional previous color can be provided to the ColorDlg ctor, which will cause this previous color to be
    shown next to the new color for comparison purposes. */
class GG_API ColorDlg : public Wnd
{
public:
    /** \name Structors */ //@{
    /** ctor */
    ColorDlg(int x, int y, const boost::shared_ptr<Font>& font,
             Clr dialog_color, Clr border_color, Clr text_color = CLR_BLACK);

    /** ctor */
    ColorDlg(int x, int y, Clr original_color, const boost::shared_ptr<Font>& font,
             Clr dialog_color, Clr border_color, Clr text_color = CLR_BLACK);
    //@}

    /** \name Accessors */ //@{
    /** returns true iff the user selected a color and then clicked the "Ok" button.  Otherwise, the color returned by
        Result() will be the original color if one was selected, or undefined if one was not. */
    bool ColorWasSelected() const;

    /** returns the color selected by the user, if the "Ok" button was used to close the dialog. */
    Clr Result() const;
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void Keypress(Key key, Uint32 key_mods);
    //@}

protected:
    /** the button used to select the custom colors in ColorDlg. */
    class GG_API ColorButton : public Button
    {
    public:
        /** \name Structors */ //@{
        ColorButton(const Clr& color);
        //@}

        /** \name Accessors */ //@{
        /** returns the custom color represented by the button */
        Clr RepresentedColor() const;
        //@}
        
        /** \name Mutators */ //@{
        /** sets the custom color represented by the button */
        void SetRepresentedColor(const Clr& color);
        //@}

    protected:
        /** \name Mutators */ //@{
        virtual void RenderUnpressed();
        virtual void RenderPressed();
        virtual void RenderRollover();
        //@}

    private:
        Clr m_represented_color;
    };

    /** a simple control that only displays a rectangle filled with the given color.  The color is shown in full alpha
        in the upper-left portion of the rectangle, and the color is shown in its given alpha in the lower-left of the
        rectangle. */
    class GG_API ColorDisplay : public Control
    {
    public:
        /** \name Structors */ //@{
        ColorDisplay(Clr color); ///< ctor.
        //@}

        /** \name Accessors */ //@{
        virtual void Render();
        //@}
    };

private:
    struct ColorButtonClickFunctor
    {
        ColorButtonClickFunctor(int id, ColorDlg* picker_);
        void operator()();
        const int button_id;
        ColorDlg* picker;
    };

    enum {R, G, B, A, H, S, V};

    void Init(const boost::shared_ptr<Font>& font);
    void ColorChanged(HSVClr color);
    void HueSaturationPickerChanged(double hue, double saturation);
    void ValuePickerChanged(double value);
    void UpdateRGBSliders();
    void UpdateHSVSliders();
    void ColorChangeFromRGBSlider();
    void ColorButtonClicked(int i);
    void RedSliderChanged(int value, int low, int high);
    void GreenSliderChanged(int value, int low, int high);
    void BlueSliderChanged(int value, int low, int high);
    void AlphaSliderChanged(int value, int low, int high);
    void HueSliderChanged(int value, int low, int high);
    void SaturationSliderChanged(int value, int low, int high);
    void ValueSliderChanged(int value, int low, int high);
    void OkClicked();
    void CancelClicked();

    HSVClr                    m_current_color;
    Clr                       m_original_color;
    bool                      m_original_color_specified;
    bool                      m_color_was_picked;

    HueSaturationPicker*      m_hue_saturation_picker;
    ValuePicker*              m_value_picker;
    Layout*                   m_pickers_layout;
    ColorDisplay*             m_new_color_square;
    ColorDisplay*             m_old_color_square;
    Layout*                   m_color_squares_layout;
    std::vector<ColorButton*> m_color_buttons;
    Layout*                   m_color_buttons_layout;
    int                       m_current_color_button;
    std::vector<TextControl*> m_slider_labels;
    std::vector<TextControl*> m_slider_values;
    std::vector<Slider*>      m_sliders;
    bool                      m_ignore_sliders;
    Button*                   m_ok;
    Button*                   m_cancel;
    Layout*                   m_sliders_ok_cancel_layout;

    Clr                       m_color;
    Clr                       m_border_color;
    Clr                       m_text_color;

    static std::vector<Clr>   s_custom_colors;

    friend struct ColorButtonClickFunctor;
};

} // namespace GG

#endif // _GGColorDlg_h_
