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

/* This class is based on earlier work with GG by Tony Casale.  Thanks, Tony.*/

#include "GGDynamicGraphic.h"

#include <GGTexture.h>
#include <GGDrawUtil.h>
#include <GGApp.h>
#include <XMLValidators.h>

#include <cmath>

namespace GG {

namespace {
const double DEFAULT_FPS = 15.0;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int margin, const Texture* texture,
                               Uint32 style/* = 0*/, int frames/* = -1*/, Uint32 flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_margin(margin),
    m_frame_width(w),
    m_frame_height(h),
    m_FPS(DEFAULT_FPS),
    m_playing(true),
    m_looping(loop),
    m_curr_texture(0),
    m_curr_subtexture(0),
    m_frames(0),
    m_curr_frame(0),
    m_first_frame_time(-1),
    m_last_frame_time(-1),
    m_first_frame_idx(0),
    m_style(style)
{
    ValidateStyle();
    SetColor(CLR_WHITE);
    AddFrames(texture, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int margin, const shared_ptr<Texture>& texture,
                               Uint32 style/* = 0*/, int frames/* = -1*/, Uint32 flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_margin(margin),
    m_frame_width(w),
    m_frame_height(h),
    m_FPS(DEFAULT_FPS),
    m_playing(true),
    m_looping(loop),
    m_curr_texture(0),
    m_curr_subtexture(0),
    m_frames(0),
    m_curr_frame(0),
    m_first_frame_time(-1),
    m_last_frame_time(-1),
    m_first_frame_idx(0),
    m_style(style)
{
    ValidateStyle();
    SetColor(CLR_WHITE);
    AddFrames(texture, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int margin, const vector<shared_ptr<Texture> >& textures,
                               Uint32 style/* = 0*/, int frames/* = -1*/, Uint32 flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_margin(margin),
    m_frame_width(w),
    m_frame_height(h),
    m_FPS(DEFAULT_FPS),
    m_playing(true),
    m_looping(loop),
    m_curr_texture(0),
    m_curr_subtexture(0),
    m_frames(0),
    m_curr_frame(0),
    m_first_frame_time(-1),
    m_last_frame_time(-1),
    m_first_frame_idx(0),
    m_style(style)
{
    ValidateStyle();
    SetColor(CLR_WHITE);
    AddFrames(textures, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int frame_width, int frame_height, int margin, const Texture* texture,
                               Uint32 style/* = 0*/, int frames/* = -1*/, Uint32 flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_margin(margin),
    m_frame_width(frame_width),
    m_frame_height(frame_height),
    m_FPS(DEFAULT_FPS),
    m_playing(true),
    m_looping(loop),
    m_curr_texture(0),
    m_curr_subtexture(0),
    m_frames(0),
    m_curr_frame(0),
    m_first_frame_time(-1),
    m_last_frame_time(-1),
    m_first_frame_idx(0),
    m_style(style)
{
    ValidateStyle();
    SetColor(CLR_WHITE);
    AddFrames(texture, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int frame_width, int frame_height, int margin, const shared_ptr<Texture>& texture,
                               Uint32 style/* = 0*/, int frames/* = -1*/, Uint32 flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_margin(margin),
    m_frame_width(frame_width),
    m_frame_height(frame_height),
    m_FPS(DEFAULT_FPS),
    m_playing(true),
    m_looping(loop),
    m_curr_texture(0),
    m_curr_subtexture(0),
    m_frames(0),
    m_curr_frame(0),
    m_first_frame_time(-1),
    m_last_frame_time(-1),
    m_first_frame_idx(0),
    m_style(style)
{
    ValidateStyle();
    SetColor(CLR_WHITE);
    AddFrames(texture, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int frame_width, int frame_height, int margin, 
                               const vector<shared_ptr<Texture> >& textures, Uint32 style/* = 0*/, int frames/* = -1*/, 
                               Uint32 flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_margin(margin),
    m_frame_width(frame_width),
    m_frame_height(frame_height),
    m_FPS(DEFAULT_FPS),
    m_playing(true),
    m_looping(loop),
    m_curr_texture(0),
    m_curr_subtexture(0),
    m_frames(0),
    m_curr_frame(0),
    m_first_frame_time(-1),
    m_last_frame_time(-1),
    m_first_frame_idx(0),
    m_style(style)
{
    ValidateStyle();
    SetColor(CLR_WHITE);
    AddFrames(textures, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(const XMLElement& elem) :
    Control(elem.Child("GG::Control")),
    m_margin(boost::lexical_cast<int>(elem.Child("m_margin").Text())),
    m_frame_width(boost::lexical_cast<int>(elem.Child("m_frame_width").Text())),
    m_frame_height(boost::lexical_cast<int>(elem.Child("m_frame_height").Text())),
    m_first_frame_time(-1),
    m_last_frame_time(-1),
    m_style(0)
{
    if (elem.Tag() != "GG::DynamicGraphic")
        throw std::invalid_argument("Attempted to construct a GG::DynamicGraphic from an XMLElement that had a tag other than \"GG::DynamicGraphic\"");

    SetColor(CLR_WHITE);

    const XMLElement* curr_elem = &elem.Child("m_textures");
    for (int i = 0; i < curr_elem->NumChildren(); i += 2) {
        FrameSet fs;
        // (borrowed from GG::SubTexture)
        string texture_filename = curr_elem->Child(i).Child("GG::Texture").Child("m_filename").Text();
        // we need to ensure that the settings in the XML-encoded texture are preserved in the loaded texture; to do this:
        shared_ptr<Texture> temp_texture = App::GetApp()->GetTexture(texture_filename);
        // if no copy of this texture exists in the manager, GetTexture() will load it with default settings, and the
        // use_count will be 2: one for the shared_ptr in the manager, one for temp_texture.
        // if this is the case, dump the texture, reload it from the XML definition (which may have non-default settings),
        // and store the XML-loaded Texture in the manager.
        if (temp_texture.use_count() == 2) {
            temp_texture.reset(new Texture(curr_elem->Child(i).Child("GG::Texture")));
            App::GetApp()->FreeTexture(texture_filename);
            App::GetApp()->StoreTexture(temp_texture, texture_filename);
            fs.texture = temp_texture;
        } else { // if this is not the case, we're not the first ones to load the texture, so just keep it.
            fs.texture = temp_texture;
        }
        fs.frames = lexical_cast<int>(curr_elem->Child(i + 1).Text());
        m_textures.push_back(fs);
    }

    m_FPS = lexical_cast<double>(elem.Child("m_FPS").Text());
    m_playing = lexical_cast<bool>(elem.Child("m_playing").Text());
    m_looping = lexical_cast<bool>(elem.Child("m_looping").Text());
    m_frames = lexical_cast<int>(elem.Child("m_frames").Text());
    m_curr_frame = lexical_cast<int>(elem.Child("m_curr_frame").Text());
    m_first_frame_idx = lexical_cast<int>(elem.Child("m_first_frame_idx").Text());
    m_last_frame_idx = lexical_cast<int>(elem.Child("m_last_frame_idx").Text());

    m_style = FlagsFromString<GraphicStyle>(elem.Child("m_style").Text());
    ValidateStyle();

    SetFrameIndex(m_curr_frame);
}

XMLElement DynamicGraphic::XMLEncode() const
{
    XMLElement retval("GG::DynamicGraphic");
    retval.AppendChild(Control::XMLEncode());
    retval.AppendChild(XMLElement("m_margin", lexical_cast<string>(m_margin)));
    retval.AppendChild(XMLElement("m_frame_width", lexical_cast<string>(m_frame_width)));
    retval.AppendChild(XMLElement("m_frame_height", lexical_cast<string>(m_frame_height)));

    XMLElement temp("m_textures");
    for (unsigned int i = 0; i < m_textures.size(); ++i) {
        temp.AppendChild(XMLElement("texture", m_textures[i].texture->XMLEncode()));
        temp.AppendChild(XMLElement("frames", lexical_cast<string>(m_textures[i].frames)));
    }
    retval.AppendChild(temp);

    retval.AppendChild(XMLElement("m_FPS", lexical_cast<string>(m_FPS)));
    retval.AppendChild(XMLElement("m_playing", lexical_cast<string>(m_playing)));
    retval.AppendChild(XMLElement("m_looping", lexical_cast<string>(m_looping)));
    retval.AppendChild(XMLElement("m_frames", lexical_cast<string>(m_frames)));
    retval.AppendChild(XMLElement("m_curr_frame", lexical_cast<string>(m_curr_frame)));
    retval.AppendChild(XMLElement("m_first_frame_idx", lexical_cast<string>(m_first_frame_idx)));
    retval.AppendChild(XMLElement("m_last_frame_idx", lexical_cast<string>(m_last_frame_idx)));
    retval.AppendChild(XMLElement("m_style", StringFromFlags<GraphicStyle>(m_style)));

    return retval;
}

XMLElementValidator DynamicGraphic::XMLValidator() const
{
    XMLElementValidator retval("GG::DynamicGraphic");
    retval.AppendChild(Control::XMLValidator());
    retval.AppendChild(XMLElementValidator("m_margin", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_frame_width", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_frame_height", new Validator<int>()));

    XMLElementValidator temp("m_textures");
    for (unsigned int i = 0; i < m_textures.size(); ++i) {
        temp.AppendChild(XMLElementValidator("texture", m_textures[i].texture->XMLValidator()));
        temp.AppendChild(XMLElementValidator("frames", new Validator<int>()));
    }
    retval.AppendChild(temp);

    retval.AppendChild(XMLElementValidator("m_FPS", new Validator<double>()));
    retval.AppendChild(XMLElementValidator("m_playing", new Validator<bool>()));
    retval.AppendChild(XMLElementValidator("m_looping", new Validator<bool>()));
    retval.AppendChild(XMLElementValidator("m_frames", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_curr_frame", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_first_frame_idx", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_last_frame_idx", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_style", new ListValidator<GraphicStyle>()));
    return retval;
}

bool DynamicGraphic::Render()
{
    if (0 <= m_curr_texture && m_curr_texture < static_cast<int>(m_textures.size()) &&
        0 <= m_curr_subtexture && m_curr_subtexture < m_textures[m_curr_texture].frames) {
        bool send_stopped_signal = false;
        bool send_end_frame_signal = false;

        // advance frames
        int initial_frame_idx = (0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx);
        int final_frame_idx =   (0.0 <= m_FPS ? m_last_frame_idx : m_first_frame_idx);
        if (m_playing) {
            if (m_first_frame_time == -1) {
                m_last_frame_time = m_first_frame_time = App::GetApp()->Ticks();
                if (0.0 != m_FPS) // needed if a start index was set
                    m_first_frame_time -= 1000.0 / m_FPS * m_curr_frame;
            } else {
                int old_frame = m_curr_frame;
                int curr_time = App::GetApp()->Ticks();
                SetFrameIndex(initial_frame_idx + static_cast<int>((curr_time - m_first_frame_time) / 1000.0 * m_FPS) % (m_last_frame_idx - m_first_frame_idx + 1));

                // determine whether the final frame was passed
                int frames_passed = static_cast<int>((curr_time - m_last_frame_time) / 1000.0 * m_FPS);
                if (m_frames <= frames_passed || (0.0 <= m_FPS ? m_curr_frame < old_frame : m_curr_frame > old_frame)) { // if we passed the final frame
                    send_end_frame_signal = true;
                    if (!m_looping) { // if looping isn't allowed, stop at the last frame
                        m_playing = false;
                        m_first_frame_time = -1;
                        SetFrameIndex(final_frame_idx);
                        send_stopped_signal = true;
                    }
                }
                m_last_frame_time = curr_time;
            }
        }

        // render current frame
        Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        glColor4ubv(color_to_use.v);

        int cols = m_textures[m_curr_texture].texture->DefaultWidth() / (m_frame_width + m_margin);
        int x = (m_curr_subtexture % cols) * (m_frame_width + m_margin) + m_margin;
        int y = (m_curr_subtexture / cols) * (m_frame_height + m_margin) + m_margin;
        SubTexture st(m_textures[m_curr_texture].texture, x, y, x + m_frame_width, y + m_frame_height);

        Pt ul = UpperLeft(), lr = LowerRight();
        Pt window_sz(lr - ul);
        Pt graphic_sz(m_frame_width, m_frame_height);
        Pt pt1, pt2(graphic_sz); // (unscaled) default graphic size
        if (m_style & GR_FITGRAPHIC) {
            if (m_style & GR_PROPSCALE) {
                double scale_x = window_sz.x / double(graphic_sz.x),
                    scale_y = window_sz.y / double(graphic_sz.y);
                double scale = (scale_x < scale_y) ? scale_x : scale_y;
                pt2.x = int(graphic_sz.x * scale);
                pt2.y = int(graphic_sz.y * scale);
            } else {
                pt2 = window_sz;
            }
        } else if (m_style & GR_SHRINKFIT) {
            if (m_style & GR_PROPSCALE) {
                double scale_x = (graphic_sz.x > window_sz.x) ? window_sz.x / double(graphic_sz.x) : 1.0,
                    scale_y = (graphic_sz.y > window_sz.y) ? window_sz.y / double(graphic_sz.y) : 1.0;
                double scale = (scale_x < scale_y) ? scale_x : scale_y;
                pt2.x = int(graphic_sz.x * scale);
                pt2.y = int(graphic_sz.y * scale);
            } else {
                pt2 = window_sz;
            }
        }

        int shift = 0;
        if (m_style & GR_LEFT) {
            shift = ul.x;
        } else if (m_style & GR_CENTER) {
            shift = ul.x + (window_sz.x - (pt2.x - pt1.x)) / 2;
        } else { // m_style & GR_RIGHT
            shift = lr.x - (pt2.x - pt1.x);
        }
        pt1.x += shift;
        pt2.x += shift;

        if (m_style & GR_TOP) {
            shift = ul.y;
        } else if (m_style & GR_VCENTER) {
            shift = ul.y + (window_sz.y - (pt2.y - pt1.y)) / 2;
        } else { // m_style & GR_BOTTOM
            shift = lr.y - (pt2.y - pt1.y);
        }
        pt1.y += shift;
        pt2.y += shift;

        st.OrthoBlit(pt1, pt2, false);

        if (send_end_frame_signal)
            m_end_frame_sig(final_frame_idx);
        if (send_stopped_signal)
            m_stopped_sig(m_curr_frame);
    }
    return true;
}

void DynamicGraphic::AddFrames(const Texture* texture, int frames/* = -1*/)
{
    int frames_in_texture = FramesInTexture(texture);
    if (!frames_in_texture)
        throw std::invalid_argument("DynamicGraphic::AddFrames : attempted to add frames from a Texture not even large enough for one frame");

    FrameSet fs;
    fs.texture.reset(texture);
    fs.frames = frames < 0 ? frames_in_texture : std::min(frames_in_texture, std::max(frames, 1));
    m_textures.push_back(fs);
    m_frames += fs.frames;
}

void DynamicGraphic::AddFrames(const shared_ptr<Texture>& texture, int frames/* = -1*/)
{
    int frames_in_texture = FramesInTexture(texture.get());
    if (!frames_in_texture)
        throw std::invalid_argument("DynamicGraphic::AddFrames : attempted to add frames from a Texture not even large enough for one frame");

    FrameSet fs;
    fs.texture = texture;
    fs.frames = frames < 0 ? frames_in_texture : std::min(frames_in_texture, std::max(frames, 1));
    m_textures.push_back(fs);
    m_frames += fs.frames;
}

void DynamicGraphic::AddFrames(const vector<shared_ptr<Texture> >& textures, int frames/* = -1*/)
{
    if (!textures.empty()) {
        int old_frames = m_frames;
        for (unsigned int i = 0; i < textures.size() - 1; ++i)
            AddFrames(textures[i], -1);
        AddFrames(textures.back(), m_frames - old_frames);
    }
}

void DynamicGraphic::Play()
{
    // if we're at the end of a previous playback and looping is disabled, reset the frame index to the initial frame
    if (!m_playing && !m_looping && m_curr_frame == (0.0 <= m_FPS ? m_last_frame_idx : m_first_frame_idx))
        SetFrameIndex(0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx);
    m_playing = true;
    if (m_FPS == 0.0)
        m_FPS = DEFAULT_FPS;
}

void DynamicGraphic::Pause()
{
    m_playing = false;
}

void DynamicGraphic::NextFrame()
{
    if (0 <= m_curr_texture && 0 <= m_curr_subtexture && m_textures.size()) { // if these are reasonable values
        m_playing = false;
        if (m_curr_frame == m_last_frame_idx) { // if this is the very last frame
            if (m_looping) // only wrap around if looping is turned on
                SetFrameIndex(m_first_frame_idx);
        } else if (m_curr_subtexture == m_textures[m_curr_texture].frames - 1) {
            ++m_curr_texture;
            m_curr_subtexture = 0;
            ++m_curr_frame;
        } else {
            ++m_curr_subtexture;
            ++m_curr_frame;
        }
    }
}

void DynamicGraphic::PrevFrame()
{
    if (0 <= m_curr_texture && 0 <= m_curr_subtexture && m_textures.size()) { // if these are reasonable values
        m_playing = false;
        if (m_curr_frame == m_first_frame_idx) { // if this is the very first frame
            if (m_looping) // only wrap around if looping is turned on
                SetFrameIndex(m_last_frame_idx);
        } else if (!m_curr_subtexture) { // if this is the first frame in its texture
            --m_curr_texture;
            m_curr_subtexture = m_textures[m_curr_texture].frames - 1;
            --m_curr_frame;
        } else { // if this is some frame in the middle of its texture
            --m_curr_subtexture;
            --m_curr_frame;
        }
    }
}

void DynamicGraphic::Stop()
{
    m_playing = false;
    SetFrameIndex(0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx);
}

void DynamicGraphic::Loop(bool b/* = true*/)
{
    m_looping = b;
}

void DynamicGraphic::SetFPS(double fps)
{
    m_FPS = fps;
}

void DynamicGraphic::SetFrameIndex(int idx)
{
    if (m_textures.empty()) { // if there are no valid texture data
        m_curr_texture = -1;
        m_curr_subtexture = -1;
        m_curr_frame = -1;
    } else if (idx < 0) { // if idx is too low
        m_curr_texture = 0;
        m_curr_subtexture = 0;
        m_curr_frame = 0;
    } else if (m_frames <= idx) { // if idx is too high
        m_curr_texture = static_cast<int>(m_textures.size()) - 1;
        m_curr_subtexture = m_textures[m_curr_texture].frames - 1;
        m_curr_frame = m_frames - 1;
    } else {
        // try to use O(1) Prev- and NextFrame() if we can
        if (idx == m_curr_frame + 1 && m_curr_frame < m_last_frame_idx) {
            NextFrame();
            m_playing = true;
        } else if (idx == m_curr_frame - 1 && m_first_frame_idx < m_curr_frame) {
            PrevFrame();
            m_playing = true;
        } else { // use O(n) linear search if necessary
            m_curr_frame = idx;
            if (idx == 0) {
                m_curr_texture = 0;
                m_curr_subtexture = 0;
            } else {
                m_curr_texture = 0;
                for (unsigned int i = 0; i < m_textures.size(); ++i) {
                    if (0 <= idx - m_textures[i].frames) {
                        idx -= m_textures[i].frames;
                        m_curr_texture++;
                    } else {
                        m_curr_subtexture = idx;
                        break;
                    }
                }
            }
        }
    }
}

void DynamicGraphic::SetTimeIndex(int idx)
{
    int initial_frame_idx = 0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx;
    int final_frame_idx = 0.0 <= m_FPS ? m_last_frame_idx : m_first_frame_idx;
    int frames_in_sequence = (m_last_frame_idx - m_first_frame_idx + 1);
    if (idx < 0)
        SetFrameIndex(initial_frame_idx);
    else if (frames_in_sequence * m_FPS <= idx && !m_looping)
        SetFrameIndex(final_frame_idx);
    else
        SetFrameIndex(initial_frame_idx + static_cast<int>(idx / 1000.0 * m_FPS) % frames_in_sequence);
}

void DynamicGraphic::SetStartFrame(int idx)
{
    if (idx < 0)
        m_first_frame_idx = 0;
    else if (m_frames <= idx)
        m_first_frame_idx = m_frames - 1;
    else
        m_first_frame_idx = idx;

    if (m_curr_frame < m_first_frame_idx)
        SetFrameIndex(m_first_frame_idx);
}

void DynamicGraphic::SetEndFrame(int idx)
{
    if (idx < 0)
        m_last_frame_idx = 0;
    else if (m_frames <= idx)
        m_last_frame_idx = m_frames - 1;
    else
        m_last_frame_idx = idx;

    if (m_last_frame_idx < m_curr_frame)
        SetFrameIndex(m_last_frame_idx);
}

void DynamicGraphic::SetStyle(Uint32 style)
{
    m_style = style; ValidateStyle();
}

int DynamicGraphic::FramesInTexture(const Texture* t) const
{
    int cols = t->DefaultWidth() / (m_frame_width + m_margin);
    int rows = t->DefaultHeight() / (m_frame_height + m_margin);
    return cols * rows;
}

void DynamicGraphic::ValidateStyle()
{
    int dup_ct = 0;   // duplication count
    if (m_style & GR_LEFT) ++dup_ct;
    if (m_style & GR_RIGHT) ++dup_ct;
    if (m_style & GR_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GR_CENTER by default
        m_style &= ~(GR_RIGHT | GR_LEFT);
        m_style |= GR_CENTER;
    }
    dup_ct = 0;
    if (m_style & GR_TOP) ++dup_ct;
    if (m_style & GR_BOTTOM) ++dup_ct;
    if (m_style & GR_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GR_VCENTER by default
        m_style &= ~(GR_TOP | GR_BOTTOM);
        m_style |= GR_VCENTER;
    }
    dup_ct = 0;
    if (m_style & GR_FITGRAPHIC) ++dup_ct;
    if (m_style & GR_SHRINKFIT) ++dup_ct;
    if (dup_ct > 1) {   // mo more than one may be picked; when both are picked, use GR_SHRINKFIT by default
        m_style &= ~GR_FITGRAPHIC;
        m_style |= GR_SHRINKFIT;
    }
}

} // namespace GG
