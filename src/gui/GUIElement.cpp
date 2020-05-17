#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIElement"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../../common/easylogging/easylogging++.h"
#include "GUI.h"
#include "../SDLEventManager.h"
#include "GUIElement.h"
#include "../../common/exception/NullPointerException.h"
#include "GUIRenderer.h"
#include "GUITexture.h"
#include "../exception/GUIException.h"


bool GUIElement::PrepareDraw() const {
	if (hidden_)
		return false;
	if (!needRedraw_)
		return false;
	return true;
}

void GUIElement::Clear(GUIRect* rect) {
	if (rect == nullptr) {
		renderer_->Clear(backgroundColor_);
		VLOG(3) << ToString() << " Clear ";
		return;
	}
	needRedraw_ = true;
	
	VLOG(3) << ToString() << " Clear " << rect->x << " x "+ rect->y << " size " << rect->w << " x " << rect->h;

	if (backgroundColor_.a == 0) { //durchsichtig
        return;
	}
	renderer_->DrawFillRect(*rect, backgroundColor_);
}

void GUIElement::UpdateScreenArea()
{
	if (parent_ == nullptr) {
		screenArea_.x = topLeft_.x;
		screenArea_.y = topLeft_.y;
		screenArea_.w = size_.width;
		screenArea_.h = size_.height;
	}
	else
	{
		auto parentScreenPos = parent_->ScreenTopLeft();
		screenArea_.x = topLeft_.x + parentScreenPos.x;
		screenArea_.y = topLeft_.y + parentScreenPos.y;
		if (size_.type != sizeType::absolute)
		{
			screenArea_.w =  absoluteSize_.width;
			screenArea_.h = absoluteSize_.height;
		}
		else {
			screenArea_.w = size_.width;
			screenArea_.h = size_.height;
		}
	}

	VLOG(3) << ToString() << " ScreenArea "  << screenArea_.x << " " << screenArea_.y << " size " << screenArea_.w << " x " << screenArea_.h;
}

void GUIElement::MouseOverElement(const Uint8 button, const GUIPoint& point) {
	if (mouseMoveEvent_) {
		mouseMoveEvent_(button, point);
	}
	if(mouseOver_) return;
	VLOG(3) << ToString() << " Mouse over";
	mouseOver_ = true;
}

void GUIElement::MouseNotOverElement(const GUIPoint& point) {
	if (!mouseOver_) return;
	if (mouseLeaveEvent_) {
		mouseLeaveEvent_(point);
	}
	VLOG(3) << ToString() << " Mouse not over";
	mouseOver_ = false;
	//Aufpassen wenn die maus auf eine andren Control los lässt als die wo man gedrückt hat
	// Drag an drop wichtig bei Click blöde
	buttonDown_ = false;
}

void GUIElement::Invalidate()
{
	if (hidden_) return;
	VLOG(3) << ToString() << " Invalidate";

	if (size_.type != sizeType::absolute)
	{
		const auto parentSize = parent_->Size();
		absoluteSize_.width = static_cast<int>(static_cast<double>(parentSize.width) * (static_cast<double>(size_.width) / 100.0));
		absoluteSize_.height = static_cast<int>(static_cast<double>(parentSize.height) * (static_cast<double>(size_.height) / 100.0));

		if ((anchor_ & AnchorFlags::Left) == AnchorFlags::Left && 
			(anchor_ & AnchorFlags::Right) != AnchorFlags::Right) {

			topLeft_.x = parentSize.width - absoluteSize_.width;
		} else {

			if ((anchor_ & AnchorFlags::Right) == AnchorFlags::Right &&
				(anchor_ & AnchorFlags::Left) != AnchorFlags::Left)
			{
				topLeft_.x = 0;
			}
		}

		if(textureControl_->Size().width != absoluteSize_.width || textureControl_->Size().height != absoluteSize_.height) {
			renderer_->ResizeTexture(textureControl_, absoluteSize_);
		}
	}
	UpdateScreenArea();
	SetRedraw();
}

bool GUIElement::ReDraw() {
	
	if (!PrepareDraw()) return false;

	renderer_->RenderTarget(textureControl_);
	Clear();
	VLOG(3) << "begin draw " << ToString();
	Draw();

	return true;
}

std::string GUIElement::ToString() const {
	std::string result("");
	result += name_ + "(" + getTypeName() + ")";
	return result;
}

std::string GUIElement::ToXml() const {
	std::string result("<");
	result += getTypeName();
	result += " name=\"";
	result += name_;
	result += "\" >";
	return result;
}

/**
 * \brief Change the Anchorflags an Repaint the Element
 * \param add Anchorflags to add
 * \param remove Anchorflags to remove
 * \return new Anchorflags
 */
AnchorFlags GUIElement::Anchor(const AnchorFlags add, const AnchorFlags remove)
{
	anchor_ |= add;
	anchor_ &= ~remove;
	Invalidate();
	return anchor_;
}

SDLEventManager* GUIElement::EventManager()
{
	return eventManager_;
}

GUIElement* GUIElement::HitTest(GUIPoint point)
{
	if (hidden_)
		return nullptr;

	if (SDL_PointInRect(&point, &screenArea_)) {

		return this;
	}
	return nullptr;
}

void GUIElement::HandleEventBase(GUIEvent& event)
{
	switch (event.Type) {
		case SDL_MOUSEMOTION: {
			const auto mouseButtonEvent = event.Event.button;
			GUIPoint point(mouseButtonEvent.x, mouseButtonEvent.y);
            //In Linux relea 1 pixel is  Sending in windows not so Repaint in Mouse move ist relly deficult
            SDL_GetMouseState( &point.x, &point.y );
			if(event.Handled) {
				MouseNotOverElement(point);
				break;
			}
           	if (HitTest(point)) {
				MouseOverElement(mouseButtonEvent.button, point);
				event.Handled = true;
				VLOG(3) << ToString() << " handled the MOUSEMOTION";
			}
			else {
				MouseNotOverElement(point);
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN: {
            if(event.Handled) {
                break;
            }
			const auto mouseButtonEvent = event.Event.button;
			const GUIPoint point(mouseButtonEvent.x, mouseButtonEvent.y);
			if (HitTest(point)) {
                event.Handled = ButtonDownBase(mouseButtonEvent.button, mouseButtonEvent.clicks, point);
                if (event.Handled) LOG(DEBUG) << ToString() << "handled the MOUSEBUTTONDOWN clicks " << static_cast<int>(mouseButtonEvent.clicks);
            }
			break;
		}
		case SDL_MOUSEBUTTONUP: {
            if(event.Handled) {
                break;
            }
			const auto mouseButtonEvent = event.Event.button;
			const GUIPoint point(mouseButtonEvent.x, mouseButtonEvent.y);
            if (HitTest(point)) {
                event.Handled = ButtonUpBase(mouseButtonEvent.button, mouseButtonEvent.clicks, point);
                if (event.Handled) LOG(DEBUG) << ToString() << "handled the MOUSEBUTTONUP clicks " << static_cast<int>(mouseButtonEvent.clicks);
            }
			break;
		}
		case SDL_FINGERMOTION: {
			//Todo Fix not hardcodet Screen resultion
			GUIPoint point(static_cast<int>(1024 * event.Event.tfinger.x), static_cast<int>(600 * event.Event.tfinger.y));
			if (event.Handled) {
				MouseNotOverElement(point);
				break;
			}
#ifdef DEBUG
			SDL_WarpMouseInWindow(nullptr, point.x, point.y);
#endif
			if (HitTest(point)) {
				MouseOverElement(SDL_BUTTON_LEFT, point);
				event.Handled = true;
				VLOG(3) << ToString() << " handled the MOUSEMOTION";
			}
			else {
				MouseNotOverElement(point);
			}
		}
        case SDL_FINGERDOWN: {
            if(event.Handled) {
                break;
            }
            //Todo Fix not hardcodet Screen resultion
	        const GUIPoint point(static_cast<int>(1024 * event.Event.tfinger.x), static_cast<int>(600 * event.Event.tfinger.y));
            VLOG(3) << "FINGERDOWN at " << point.x << " x " << point.y;
#ifdef DEBUG
            SDL_WarpMouseInWindow(nullptr, point.x, point.y);
#endif
			if (HitTest(point)) {
				MouseOverElement(SDL_BUTTON_LEFT, point);
                event.Handled = ButtonDownBase(SDL_BUTTON_LEFT, 1, point);
                if (event.Handled) LOG(DEBUG) << ToString() << "handled the SDL_FINGERDOWN";
            }
			break;
		}
        case SDL_FINGERUP: {
            if(event.Handled) {
                break;
            }
            //Todo Fix not hardcodet Screen resultion
	        const GUIPoint point(static_cast<int>(1024 * event.Event.tfinger.x), static_cast<int>(600 * event.Event.tfinger.y));
            if (HitTest(point)) {
                event.Handled = ButtonUpBase(SDL_BUTTON_LEFT, 1, point);
                if (event.Handled) LOG(DEBUG) << ToString() << "handled the SDL_FINGERUP";
            }
			break;
		}
		default:
			HandleEvent(event);
			break;
	}
}

void GUIElement::SetRedraw() {
	needRedraw_ = true;
	if(parent_ != nullptr) parent_->SetRedraw();
}

void GUIElement::SetRenderer(GUITexture* texture) const {
	if (texture == nullptr) {
		renderer_->RenderTarget(textureControl_);
		return;
	}
	renderer_->RenderTarget(texture);
}

GUIPoint GUIElement::ScreenToElementCoords(const GUIPoint& point) const {
	GUIPoint result;
	result.x = point.x - screenArea_.x;
	result.y = point.y - screenArea_.y;
	return result;
}

GUISize GUIElement::Size() const
{
	if(size_.type != sizeType::absolute) {
		return absoluteSize_;
	}
	return size_;
}

GUITexture* GUIElement::Texture() const
{
	if (textureControl_ == nullptr)
		throw GUIException("Element not Created forgott add GUIElementManager ?");
	return textureControl_;
}

GUIPoint GUIElement::TopLeft() const
{
	return topLeft_;
}

GUIPoint GUIElement::ScreenTopLeft() const
{
	return GUIPoint(screenArea_.x, screenArea_.y);
}

GUIElement::GUIElement(const GUIPoint position, const GUISize size, const std::string& name):
	parent_(nullptr),
	name_(name),
	size_(size),
	screenArea_(0,0,0,0),
	anchor_(),
	eventManager_(nullptr),
	fontManager_(nullptr),
	renderer_(nullptr),
    mapManager_(nullptr),
	backgroundColor_(black_color),
	foregroundColor_(white_color),
	topLeft_(position.x, position.y),
	hidden_(false),
	hasFocus_(false),
	needRedraw_(true),
	hasBorder_(false),
	enabled_(true),
	selected_(false)
{
	logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);

	if (hidden_) needRedraw_ = false;
	anchor_ |= AnchorFlags::Top;
	anchor_ |= AnchorFlags::Right;
	if(size_.type == sizeType::relative){
        LOG(DEBUG) << ToString() << " create new element at " << topLeft_.x << " x " << topLeft_.y << " size " << size_.width << "% x " << size_.height << "%";
    } else {
        LOG(DEBUG) << ToString() << " create new element at " << topLeft_.x << " x " << topLeft_.y << " size " << size_.width << " x " << size_.height;
    }
}

GUIElement::~GUIElement()
{
	if (textureControl_ != nullptr)
	{
		delete textureControl_;
	}
	LOG(DEBUG) << ToString().c_str() << " killed";
}

bool GUIElement::NeedRedraw() const
{
	if (hidden_) return false;

	if (needRedraw_) {
		VLOG(2) << ToString() << " need redraw";
	}
	return needRedraw_;
}

void GUIElement::Create(GUIRenderer* renderer, GUIFontManager* fontManager)
{
	renderer_ = renderer;
	fontManager_ = fontManager;

	if (renderer_ == nullptr) {
		throw NullPointerException("renderer can not be null");
	}
	
	if (fontManager_ == nullptr) {
		throw NullPointerException("fontManager can not be null");
	}

	if (size_.type == sizeType::absolute) {
		textureControl_ = renderer_->CreateTexture(size_);
	}
	else {
		if (parent_ == nullptr) {
			throw NullPointerException("with parent null the position musst be absolute");
		}

		const auto parentSize = parent_->Size();
		absoluteSize_.width = static_cast<int>(static_cast<double>(parentSize.width) * (static_cast<double>(size_.width) / 100.0));
		absoluteSize_.height = static_cast<int>(static_cast<double>(parentSize.height) * (static_cast<double>(size_.height) / 100.0));

		textureControl_ = renderer_->CreateTexture(absoluteSize_);
	}

	if(backgroundColor_.a != 0xFF) {
        textureControl_->SetBlendMode(blendMode::blend);
	}

	UpdateScreenArea();
}

void GUIElement::Create(GUIRenderer* renderer, GUIElement* parent, GUIFontManager* fontManager, SDLEventManager* eventManager, GUIImageManager* imageManager, IMapManager* mapManager, Uint32 windowId)
{
	parent_ = parent;
	if (parent_ == nullptr) {
		throw NullPointerException("parent can not be null");
	}

	eventManager_ = eventManager;
	if (eventManager_ == nullptr) {
		throw NullPointerException("eventManager can not be null");
	}

	imageManager_ = imageManager;
	if (imageManager_ == nullptr) {
		throw NullPointerException("imageManager can not be null");
	}
    mapManager_ = mapManager;
    windowId_ = windowId;
	Create(renderer, fontManager);
}

bool GUIElement::ButtonDownBase(Uint8 button, Uint8 clicks, const GUIPoint& point)
{
	if (!enabled_) return false;

	if (button == SDL_BUTTON_LEFT) {
		if (mouseOver_) {
			if (!buttonDown_)
			{
				if (buttonDownEvent_) {
					buttonDownEvent_(button, clicks, point);
				}
				LOG(DEBUG) << "Button down on " << ToString();
			}
			buttonDown_ = true;
			return true;
		}
	}
	return false;
}

bool GUIElement::ButtonUpBase(Uint8 button, Uint8 clicks,const GUIPoint& point)
{
	if (!enabled_) return false;

	if (button == SDL_BUTTON_LEFT) {
		if (buttonDown_) {
			buttonDown_ = false;
			LOG(DEBUG) << "Button up on " << ToString();
			if (buttonUpEvent_) {
				buttonUpEvent_(button, clicks, point);
			}
			return true;
		}
	}
	return false;
}

void GUIElement::Disable() {
    if(!enabled_) return;
	enabled_ = false;
	needRedraw_ = true;
}

void GUIElement::Enable() {
    if(enabled_) return;
    enabled_ = true;
	needRedraw_ = true;
}

void GUIElement::Select() {
	if (selected_) return;
	selected_ = true;
	SetRedraw();
}

void GUIElement::Unselect() {
	if (!selected_) return;
	selected_ = false;
	SetRedraw();
}

void GUIElement::Visible() {
	hidden_ = false;
    SetRedraw();
}

void GUIElement::Invisible() {
    LOG(DEBUG) << "Invisible " << ToString();
    hidden_ = true;
    SetRedraw();
}
