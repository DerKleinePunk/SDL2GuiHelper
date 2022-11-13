#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUISvgView"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include <iostream>
#include <fstream>
#include <string>

#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/commonutils.h"
#include "GUISvgView.hpp"


void GUISvgView::DrawIntern()
{
#ifdef ENABLECAIRO
    //auto image = _cairoTexture->GetCairo();
    std::ifstream myFile_Handler;
    std::string myLine;
    std::string svgBuffer;

    // File Open in the Read Mode
    myFile_Handler.open(_fileName);

    if(myFile_Handler.is_open())
    {
        while(getline(myFile_Handler, myLine))
        {
            utils::replaceAll(myLine, "{{roll}}", "0");
            utils::replaceAll(myLine, "{{pitch}}", "0");

            std::cout << myLine << std::endl;
            svgBuffer += myLine;
        }
        myFile_Handler.close();
    }

    _cairoTexture->LoadSvg(reinterpret_cast<const unsigned char*>(svgBuffer.c_str()), svgBuffer.length(), GUIRect(0, 0, Size()));

    _cairoTexture->PaintDone();
#endif
}

GUISvgView::GUISvgView(GUIPoint position, GUISize size, const std::string& name):
    GUIElement(position, size, name)
{
    _logger = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = transparent_color;
    _fileName = "artificial-horizon.svg";
}

GUISvgView::~GUISvgView()
{
#ifdef ENABLECAIRO
    if(_cairoTexture != nullptr) {
        delete _cairoTexture;
    }
#endif
}


void GUISvgView::Init()
{
    // Things after Control is Created
#ifdef ENABLECAIRO
    _cairoTexture = new GUICairoTexture(renderer_, Size());
    DrawIntern();
#endif
}

void GUISvgView::Draw()
{
#ifdef ENABLECAIRO
    auto texture = _cairoTexture->GetTexture();
    if(texture != nullptr) {
        auto drawTextPosition = GUIPoint(0, 0);
        renderer_->RenderCopy(texture, drawTextPosition);
    }
#endif

    needRedraw_ = false;
}

void GUISvgView::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUISvgView::UpdateAnimation()
{
}

void GUISvgView::Close()
{
}