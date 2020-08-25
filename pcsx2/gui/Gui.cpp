#include "Gui.h"

bool Gui::Initalize()
{
    try
    {
        config = new wxConfig("PCSX2");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

void Gui::Load()
{

}

void Gui::Save()
{

}