#pragma once
#include <SDL.h>
#include <string>
#include <functional>
#define SDL2_GFXPRIMITIVES_SCOPE
#include "../../common/SDL2_gfx/SDL2_gfxPrimitives.h"

enum class textAlign : unsigned char {
	center,
	left,
	right
};

enum class blendMode : unsigned char {
	none,
	blend,
	add,
	mod
};

enum class sizeType : unsigned char {
	absolute,
	relative
};

enum class AnchorFlags : unsigned char{
	None = 0,
	Top = 1 << 0,
	Buttom = 1 << 1,
	Right = 1 << 2,
	Left = 1 << 3,
};

constexpr AnchorFlags operator |(AnchorFlags X, AnchorFlags Y) {
	return static_cast<AnchorFlags>(
		static_cast<unsigned char>(X) | static_cast<unsigned char>(Y));
}

constexpr AnchorFlags operator &(AnchorFlags X, AnchorFlags Y) {
	return static_cast<AnchorFlags>(
		static_cast<unsigned char>(X) & static_cast<unsigned char>(Y));
}

constexpr AnchorFlags operator ^(AnchorFlags X, AnchorFlags Y) {
	return static_cast<AnchorFlags>(
		static_cast<unsigned char>(X) ^ static_cast<unsigned char>(Y));
}

constexpr AnchorFlags operator ~(AnchorFlags X) {
	return static_cast<AnchorFlags>(
		~static_cast<unsigned char>(X));
}

inline AnchorFlags& operator |=(AnchorFlags& X, AnchorFlags Y) {
	X = X | Y; return X;
}

inline AnchorFlags& operator &=(AnchorFlags& X, AnchorFlags Y) {
	X = X & Y; return X;
}

inline AnchorFlags& operator ^=(AnchorFlags& X, AnchorFlags Y) {
	X = X ^ Y; return X;
}

struct GUIPoint :SDL_Point {
	GUIPoint();
	GUIPoint(int x, int y);
	GUIPoint(const GUIPoint& that);//copy constructor
	GUIPoint& operator=(const GUIPoint& that);//copy assignment operator
	void set(int x, int y);
	std::string ToString() const;
};

struct GUISize {
	GUISize();
	GUISize(int with, int height);
	GUISize(int with1, int height1, sizeType type1);
	void set(int with, int height);
	GUISize(const GUISize& that);//copy constructor
	GUISize& operator=(const GUISize& that);//copy assignment operator
	int width;
	int height;
	sizeType type;
};

struct GUIRect :SDL_Rect {
	GUIRect();
	GUIRect(int x, int y, int with, int height);
	GUIRect(GUIPoint point, GUISize size);
	GUIRect(int x, int y, GUISize size);
	GUIRect(const GUIRect& that);//copy constructor
	GUIRect& operator=(const GUIRect& that);//copy assignment operator
	void set(int x, int y, int with, int height);
};

struct GUIEvent
{
	Uint32 Type;
	bool Handled;
	SDL_Event Event;
	explicit GUIEvent(const SDL_Event* event);
	explicit GUIEvent();
};

//https://www.w3schools.com/colors/colors_rgb.asp
const SDL_Color black_color = { 0, 0, 0, 0xff };
const SDL_Color white_color = { 0xff, 0xff, 0xff, 0xff };
const SDL_Color lightblack_color = { 25, 25, 26, 0xff };
const SDL_Color black_t_color = { 0xFF, 0xFF, 0xFF, 0x4C }; // ~30%
const SDL_Color red_color = { 0xff, 0, 0, 0xff };
const SDL_Color green_color = { 0, 0xff, 0, 0xff };
const SDL_Color blue_color = { 0, 0, 0xff, 0xff };
const SDL_Color yellow_color = { 0xff, 0xff, 0, 0xff };
const SDL_Color lightgray_t_color = { 0xCC, 0xCC, 0xCC, 0x80 };
const SDL_Color lightgray_color = { 0xCC, 0xCC, 0xCC, 0xff };
const SDL_Color accent_color = { 0x11, 0x9E, 0xDA, 0x99 };
const SDL_Color transparent_color = { 0x00, 0x00, 0x00, 0x00 };
const SDL_Color transparent_white_color = { 0xFF, 0xFF, 0xFF, 0x00 };
const SDL_Color own_blue_color = { 0x28, 0x5B, 0x78, 0xFF }; //285B78
const SDL_Color own_red_color = { 0x84, 0x14, 0x28, 0xFF }; //841428
const SDL_Color buttondown_green_color = { 0x00, 0xAF, 0x64, 0xFF }; //#00af64
const Uint32 long_click_time = 500;

double frac(double value);

typedef std::function<void(Uint8 button, Uint8 clicks, const GUIPoint& point)> ButtonDelegate;
typedef std::function<void(Uint8 button, const GUIPoint& point)> MouseDelegate;
typedef std::function<void(const GUIPoint& point)> MousePositionDelegate;

#if (!SDL_VERSION_ATLEAST(2,0,4))
SDL_FORCE_INLINE SDL_bool SDL_PointInRect(const SDL_Point *p, const SDL_Rect *r)
{
	return ((p->x >= r->x) && (p->x < (r->x + r->w)) &&
		(p->y >= r->y) && (p->y < (r->y + r->h))) ? SDL_TRUE : SDL_FALSE;
}
#endif

//Todo: I am not happy with this
#if defined(__GNUC__)
    #define UNUSED(x) ((void)(x))
#else
    #define UNUSED(x) UNREFERENCED_PARAMETER(x) 
#endif

#define byteswap32 __builtin_bswap32