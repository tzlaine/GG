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
#include "GGTexture.h"
#include "GGDrawUtil.h"
#include "GGApp.h"

#include <cmath>

namespace GG {

namespace {
const double DEFAULT_FPS = 15.0;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int margin, const Texture* texture,
                               int frames/* = -1*/, Uint32 flags/* = 0*/) :
        Control(x, y, w, h, flags),
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
        m_margin(margin)
{
    SetColor(CLR_WHITE);
    AddFrames(texture, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int margin, const shared_ptr<Texture>& texture,
                               int frames/* = -1*/, Uint32 flags/* = 0*/) :
        Control(x, y, w, h, flags),
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
        m_margin(margin)
{
    SetColor(CLR_WHITE);
    AddFrames(texture, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(int x, int y, int w, int h, bool loop, int margin, const vector<shared_ptr<Texture> >& textures,
                               int frames/* = -1*/, Uint32 flags/* = 0*/) :
        Control(x, y, w, h, flags),
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
        m_margin(margin)
{
    SetColor(CLR_WHITE);
    AddFrames(textures, frames);
    m_last_frame_idx = m_frames - 1;
}

DynamicGraphic::DynamicGraphic(const XMLElement& elem) :
        Control(elem.Child("GG::Control")),
        m_first_frame_time(-1),
        m_last_frame_time(-1),
        m_margin(boost::lexical_cast<int>(elem.Child("m_margin").Attribute("value")))
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
        fs.frames = lexical_cast<int>(curr_elem->Child(i + 1).Attribute("value"));
        m_textures.push_back(fs);
    }

    curr_elem = &elem.Child("m_FPS");
    m_FPS = lexical_cast<double>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_playing");
    m_playing = lexical_cast<bool>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_looping");
    m_looping = lexical_cast<bool>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_frames");
    m_frames = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_curr_frame");
    m_curr_frame = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_first_frame_idx");
    m_first_frame_idx = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_last_frame_idx");
    m_last_frame_idx = lexical_cast<int>(curr_elem->Attribute("value"));

    SetFrameIndex(m_curr_frame);
}

XMLElement DynamicGraphic::XMLEncode() const
{
    XMLElement retval("GG::DynamicGraphic");
    retval.AppendChild(Control::XMLEncode());

    XMLElement temp;

    temp = XMLElement("m_textures");
    for (unsigned int i = 0; i < m_textures.size(); ++i) {
        XMLElement local_temp;

        local_temp = XMLElement("texture");
        local_temp.AppendChild(m_textures[i].texture->XMLEncode());
        temp.AppendChild(local_temp);

        local_temp = XMLElement("frames");
        local_temp.SetAttribute("value", lexical_cast<string>(m_textures[i].frames));
        temp.AppendChild(local_temp);
    }
    retval.AppendChild(temp);

    temp = XMLElement("m_FPS");
    temp.SetAttribute("value", lexical_cast<string>(m_FPS));
    retval.AppendChild(temp);

    temp = XMLElement("m_playing");
    temp.SetAttribute("value", lexical_cast<string>(m_playing));
    retval.AppendChild(temp);

    temp = XMLElement("m_looping");
    temp.SetAttribute("value", lexical_cast<string>(m_looping));
    retval.AppendChild(temp);

    temp = XMLElement("m_frames");
    temp.SetAttribute("value", lexical_cast<string>(m_frames));
    retval.AppendChild(temp);

    temp = XMLElement("m_curr_frame");
    temp.SetAttribute("value", lexical_cast<string>(m_curr_frame));
    retval.AppendChild(temp);

    temp = XMLElement("m_first_frame_idx");
    temp.SetAttribute("value", lexical_cast<string>(m_first_frame_idx));
    retval.AppendChild(temp);

    temp = XMLElement("m_last_frame_idx");
    temp.SetAttribute("value", lexical_cast<string>(m_last_frame_idx));
    retval.AppendChild(temp);

    temp = XMLElement("m_margin");
    temp.SetAttribute("value", lexical_cast<string>(m_margin));
    retval.AppendChild(temp);

    return retval;
}

int DynamicGraphic::Render()
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

        int cols = m_textures[m_curr_texture].texture->DefaultWidth() / (Width() + m_margin);
        int x = (m_curr_subtexture % cols) * (Width() + m_margin) + m_margin;
        int y = (m_curr_subtexture / cols) * (Height() + m_margin) + m_margin;
        SubTexture st(m_textures[m_curr_texture].texture, x, y, x + Width(), y + Height());
        st.OrthoBlit(UpperLeft(), false);

        if (send_end_frame_signal)
            m_end_frame_sig(final_frame_idx);
        if (send_stopped_signal)
            m_stopped_sig(m_curr_frame);
    }
    return 1;
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
            for (unsigned int i = 0; i < m_textures.size(); ++i) {
                if (0 < idx - m_textures[i].frames) {
                    idx -= m_textures[i].frames;
                } else {
                    m_curr_texture = i;
                    m_curr_subtexture = idx;
                    break;
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

int DynamicGraphic::FramesInTexture(const Texture* t)
{
    int cols = t->DefaultWidth() / (Width() + m_margin);
    int rows = t->DefaultHeight() / (Height() + m_margin);
    return cols * rows;
}

} // namespace GG

