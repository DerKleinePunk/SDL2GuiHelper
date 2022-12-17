#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIArtificialHorizon"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include <iostream>
#include <fstream>
#include <string>

#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/commonutils.h"
#include "GUIArtificialHorizon.hpp"

void GUIArtificialHorizon::DrawIntern()
{
#ifdef ENABLECAIRO
    auto image = _cairoTexture->GetCairo();

    /*cairo_set_operator (image, CAIRO_OPERATOR_CLEAR);
    cairo_paint (image);*/

    double sizeImage = Size().width;
    if(sizeImage > Size().height) {
        sizeImage = Size().height;
    }
    sizeImage = sizeImage / 2;

    /*double linesize = 2.0;
    cairo_set_source_rgb(image, 0.757, 0.757, 0.059);
    cairo_set_line_width(image, linesize);
    cairo_move_to(image, Size().width / 2, Size().height / 2);
    cairo_line_to(image, Size().width / 2 - sizeImage, Size().height / 2);
    cairo_move_to(image, Size().width / 2, Size().height / 2);
    cairo_line_to(image, Size().width / 2 + sizeImage, Size().height / 2);
    cairo_move_to(image, Size().width / 2, Size().height / 2);
    cairo_line_to(image, Size().width / 2, Size().height / 2 - sizeImage);
    cairo_move_to(image, Size().width / 2, Size().height / 2);
    cairo_line_to(image, Size().width / 2, Size().height / 2 + sizeImage);

    cairo_stroke (image);

    cairo_set_source_rgb(image, 0.047, 0.796, 0.047);
    double pixelDistance = sizeImage / 90;
    for(auto i = 5; i < 21; i = i + 5)
    {
        if(i % 10 == 0) {
            cairo_move_to(image, Size().width / 2 - 20, Size().height / 2 - pixelDistance * i);
            cairo_line_to(image, Size().width / 2 + 20, Size().height / 2 - pixelDistance * i);
            cairo_move_to(image, Size().width / 2 - 20, Size().height / 2 + pixelDistance * i);
            cairo_line_to(image, Size().width / 2 + 20, Size().height / 2 + pixelDistance * i);
        }
        else{
            cairo_move_to(image, Size().width / 2 - 10, Size().height / 2 - pixelDistance * i);
            cairo_line_to(image, Size().width / 2 + 10, Size().height / 2 - pixelDistance * i);
            cairo_move_to(image, Size().width / 2 - 10, Size().height / 2 + pixelDistance * i);
            cairo_line_to(image, Size().width / 2 + 10, Size().height / 2 + pixelDistance * i);
        }
    }
    cairo_stroke (image);

    //cairo_translate(image,Size().width/2,Size().height/2); // translate origin to the center
    //cairo_rotate(image, _rolling * GradToRadians);
    //cairo_translate(image,Size().width/2,Size().height/2); // translate origin back
   

    double circlelinesize = 24.0;
    cairo_set_source_rgb(image, 0.725, 0.725, 0.725);
    cairo_set_line_width(image, circlelinesize);
    cairo_arc(image, Size().width / 2, Size().height / 2, sizeImage - (circlelinesize / 2), 180 * GradToRadians, 0 * GradToRadians);
    cairo_stroke (image);

    cairo_set_source_rgb(image, 0.314, 0.314, 0.314);
    cairo_arc(image, Size().width / 2, Size().height / 2, sizeImage - (circlelinesize / 2), 0 * GradToRadians, 180 * GradToRadians);
    cairo_stroke (image);

    linesize = 2.0;
    double angle1 = 60  * GradToRadians;  // angles are specified in radians 
    double angle2 = 30 * GradToRadians;

    cairo_set_source_rgb(image, 1, 1, 1);
    cairo_set_line_width(image, linesize);
    cairo_move_to(image, Size().width / 2 - (cos(angle1) * (sizeImage - circlelinesize)), sizeImage - sin(angle1) * (sizeImage - circlelinesize));
    cairo_line_to(image, Size().width / 2 - (cos(angle1) * sizeImage), sizeImage - sin(angle1) * sizeImage);

    cairo_move_to(image, Size().width / 2 - (cos(angle2) * (sizeImage - circlelinesize)), sizeImage - sin(angle2) * (sizeImage - circlelinesize));
    cairo_line_to(image, Size().width / 2 - (cos(angle2) * sizeImage), sizeImage - sin(angle2) * sizeImage);
    
    cairo_move_to(image, Size().width / 2 + (cos(angle1) * (sizeImage - circlelinesize)), sizeImage - sin(angle1) * (sizeImage - circlelinesize));
    cairo_line_to(image, Size().width / 2 + (cos(angle1) * sizeImage), sizeImage - sin(angle1) * sizeImage);

    cairo_move_to(image, Size().width / 2 + (cos(angle2) * (sizeImage - circlelinesize)), sizeImage - sin(angle2) * (sizeImage - circlelinesize));
    cairo_line_to(image, Size().width / 2 + (cos(angle2) * sizeImage), sizeImage - sin(angle2) * sizeImage);

    cairo_stroke (image);

    // Draw a triangle.
    double angle3 = 88 * GradToRadians;
    cairo_move_to(image, Size().width / 2, circlelinesize);
    cairo_line_to(image, Size().width / 2 - (cos(angle3) * (sizeImage - 5.0)), sizeImage - sin(angle3) * (sizeImage -5.0));
    cairo_line_to(image, Size().width / 2 + (cos(angle3) * (sizeImage - 5.0)), sizeImage - sin(angle3) * (sizeImage -5.0));
    cairo_close_path(image);
    cairo_fill(image);

    angle3 = 45 * GradToRadians;
    cairo_move_to(image, Size().width / 2 - (cos(angle3) * (sizeImage - circlelinesize)), sizeImage - sin(angle3) * (sizeImage - circlelinesize));
    angle3 = 43 * GradToRadians;
    cairo_line_to(image, Size().width / 2 - (cos(angle3) * (sizeImage - 10.0)), sizeImage - sin(angle3) * (sizeImage - 10.0));
    angle3 = 47 * GradToRadians;
    cairo_line_to(image, Size().width / 2 - (cos(angle3) * (sizeImage - 10.0)), sizeImage - sin(angle3) * (sizeImage - 10.0));
    cairo_close_path(image);
    cairo_fill(image);

    angle3 = 45 * GradToRadians;
    cairo_move_to(image, Size().width / 2 + (cos(angle3) * (sizeImage - circlelinesize)), sizeImage - sin(angle3) * (sizeImage - circlelinesize));
    angle3 = 43 * GradToRadians;
    cairo_line_to(image, Size().width / 2 + (cos(angle3) * (sizeImage - 10.0)), sizeImage - sin(angle3) * (sizeImage - 10.0));
    angle3 = 47 * GradToRadians;
    cairo_line_to(image, Size().width / 2 + (cos(angle3) * (sizeImage - 10.0)), sizeImage - sin(angle3) * (sizeImage - 10.0));
    cairo_close_path(image);
    cairo_fill(image);*/

    cairo_translate(image,Size().width/2,Size().height/2); // translate origin to the center
    cairo_rotate(image, _rolling * GradToRadians);// 20 degrees
    cairo_translate(image, -Size().width/2, -Size().height/2);  
    cairo_rectangle(image, Size().width/2 - 50.0, Size().height/2 - 25.5, 100.0, 50.0);
    cairo_set_source_rgb(image, 0, 0, 1);
    cairo_stroke(image);
   

    _cairoTexture->PaintDone();
#endif
}

GUIArtificialHorizon::GUIArtificialHorizon(GUIPoint position, GUISize size, const std::string& name):
    GUIElement(position, size, name)
{
    _logger = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = transparent_color;
    _rolling = 0.0;
}

GUIArtificialHorizon::~GUIArtificialHorizon()
{
#ifdef ENABLECAIRO
    if(_cairoTexture != nullptr) {
        delete _cairoTexture;
    }
#endif
}

void GUIArtificialHorizon::Init()
{
    // Things after Control is Created
#ifdef ENABLECAIRO
    _cairoTexture = new GUICairoTexture(renderer_, Size());
    DrawIntern();
#endif
}

void GUIArtificialHorizon::Draw()
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

void GUIArtificialHorizon::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUIArtificialHorizon::UpdateAnimation()
{
}

void GUIArtificialHorizon::Close()
{
}

void GUIArtificialHorizon::SetRolling(double value)
{
    _rolling = value;
    delete _cairoTexture;
    _cairoTexture = new GUICairoTexture(renderer_, Size());
    DrawIntern();
    SetRedraw();
}