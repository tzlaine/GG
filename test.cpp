/*
 * Use 'make check' to see if GG is working correctly.
 */

#include "SDL.h"
#include <SDLGGApp.h>

class Test : public SDLGGApp
{
public:
    void Initialize();
    void Enter2DMode();
    void Exit2DMode();
    void Update();
    int Status;
    ~Test();
private:
    virtual void SDLInit();
};


void Test::Initialize()
{
//    Status = SDL_Init(SDL_INIT_NOPARACHUTE|SDL_INIT_VIDEO);
    SDLInit();
    
}

void Test::SDLInit()
{
   SDL_SetVideoMode(640,480,16,SDL_OPENGL);
//   GLInit();
}


void Test::Enter2DMode()
{
}

void Test::Exit2DMode()
{
}

void Test::Update()
{
}

Test::~Test()
{
    SDL_Quit();
}

extern "C" int main(int argc, char* argv[])
{
    Test a;
    a.Initialize();
    
    
    return a.Status;
    
}
