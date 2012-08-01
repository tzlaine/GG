#include <GG/PtRect.h>

#include <string>


struct Action
{
    enum Semantic
    {
        LeftClick,
        RightClick,
        DoubleClick,
        Drag
    };

    Action();
    Action(int x, int y, Semantic semantic);
    Action(int x1, int y1, int x2, int y2);
    Action(const std::vector<GG::Key>& keys);

    GG::Pt m_pt;
    GG::Pt m_pt_2;
    Semantic m_semantic;
    std::vector<GG::Key> m_keys;
    bool m_pressed_for_drag;
    bool m_moved_for_drag;
    bool m_released_for_drag;
};

typedef std::vector<Action> Actions;

Actions ParseActions(const std::string& str);
