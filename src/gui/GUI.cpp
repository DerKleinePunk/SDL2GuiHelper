#include "GUI.h"

GUIPoint::GUIPoint(int x1, int y1) { x = x1; y = y1; }

GUIPoint::GUIPoint(const GUIPoint& that)
{
	x = that.x;
	y = that.y;
}

GUIPoint& GUIPoint::operator=(const GUIPoint& that)
{
	if (this != &that)
	{
		x = that.x;
		y = that.y;
	}
	return *this;
}

GUIPoint::GUIPoint() { x = y = 0; }
void GUIPoint::set(int x1, int y1) { x = x1; y = y1; }

std::string GUIPoint::ToString() const {
	return std::to_string(x) + " x " + std::to_string(y);
}

GUISize::GUISize(int with1, int height1):
    type(sizeType::absolute) {
	width = with1; 
	height = height1; 
}

GUISize::GUISize(int with1, int height1, sizeType type1):
    type(type1) {
    width = with1;
    height = height1;
}

GUISize::GUISize(): 
    type(sizeType::absolute) {
	width = height = 0; 
}

void GUISize::set(int with1, int height1){
	width = with1; 
	height = height1;
	type = sizeType::absolute;
}

GUISize::GUISize(const GUISize& that):
    type(that.type) {
    width = that.width;
    height = that.height;
	
}

GUISize& GUISize::operator=(const GUISize& that)
{
	if (this != &that)
	{
		width = that.width;
		height = that.height;
		type = that.type;
	}
	return *this;
}

GUIRect::GUIRect() { x = y = w = h = 0; }
GUIRect::GUIRect(int _x, int _y, int _dx, int _dy) { x = _x; y = _y; w = _dx; h = _dy; }

GUIRect::GUIRect(GUIPoint point, GUISize size)
{
	x = point.x;
	y = point.y;
	w = size.width;
	h = size.height;
}

GUIRect::GUIRect(int _x, int _y, GUISize size)
{
	x = _x;
	y = _y;
	w = size.width;
	h = size.height;
}

GUIRect::GUIRect(const GUIRect& that)
{
	x = that.x;
	y = that.y;
	w = that.w;
	h = that.h;
}

GUIRect& GUIRect::operator=(const GUIRect& that)
{
	if (this != &that)
	{
		x = that.x;
		y = that.y;
		w = that.w;
		h = that.h;
	}
	return *this;
}

void GUIRect::set(int _x, int _y, int _dx, int _dy) { x = _x; y = _y; w = _dx; h = _dy; }

GUIEvent::GUIEvent()
{
	Event.type = 0xFFFFF;
	Type = Event.type;
	Handled = false;
	DisplayWidth = -1;
	DisplayHeight = -1;	
}

GUIEvent::GUIEvent(const SDL_Event* base, int displayWidth, int displayHeight):
    Event(*base) {
    Type = Event.type;
    Handled = false;
	DisplayWidth = displayWidth;
	DisplayHeight = displayHeight;	
}

double frac(double value)
{
	return value - static_cast<long>(value);
}
