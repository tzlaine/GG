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
};


void Test::Initialize()
{
    Status = SDL_Init(SDL_INIT_NOPARACHUTE);
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

    


