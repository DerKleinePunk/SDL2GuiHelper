#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIProgressbar"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIElement.h"
#include "GUIProgressbar.h"
#include "GUIRenderer.h"
#include "GUITexture.h"
#include "GUIImageManager.h"


GUIProgressbar::GUIProgressbar(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor)
    : GUIElement(position, size, name), GUIOnClickDecorator(static_cast<GUIElement*>(this))
{
    logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = transparent_color;
    _background = background;
    _textcolor = textcolor;
}
    
GUIProgressbar::~GUIProgressbar()
{

}

void GUIProgressbar::Init()
{
    //Things after Control is Created
}

void GUIProgressbar::Draw()
{

}

void GUIProgressbar::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}
	
void GUIProgressbar::UpdateAnimation()
{

}

void GUIProgressbar::Close()
{

}

void GUIProgressbar::ButtonDownUpdate(Uint8 button)
{
    UNUSED(button);
}

void GUIProgressbar::ButtonUpUpdate(Uint8 button)
{
    UNUSED(button);
}