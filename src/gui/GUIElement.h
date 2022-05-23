#pragma once

#include "IGUIElement.h"
#include "../rtti.h"
#include "../../common/easylogging/easylogging++.h"
#include "GUIScreen.h"
#include "../IMapManager.h"
#include "../IAudioManager.h"

class GUIRenderer;
class GUITexture;
class GUIElementManager;
class GUIFontManager;
class GUIImageManager;
class SDLEventManager;
class logger_;
class MiniKernel;

class GUIElement : public IGUIElement
{
	friend class GUIElementTreeNode;
	friend class GUIElementManager;
	friend class GUIScreenCanvas;

	GUIElement* parent_;
	GUITexture* textureControl_;
	std::string name_;
	GUISize size_;
	GUISize absoluteSize_;
	GUIRect screenArea_;

	AnchorFlags anchor_;
	SDLEventManager* eventManager_;
	el::Logger* logger_;

	RTTI_BASE(GUIElement)

	bool PrepareDraw() const;
	void Clear(GUIRect* rect = nullptr);
	void UpdateScreenArea();
	void MouseOverElement(Uint8 button, const GUIPoint& point);
	void MouseNotOverElement(const GUIPoint& point);
	bool ButtonDownBase(Uint8 button, Uint8 clicks, const GUIPoint& point);
	bool ButtonUpBase(Uint8 button, Uint8 clicks, const GUIPoint& point);
	void Disable();
	void Enable();
	virtual void Select();
	virtual void Unselect();
	void Visible();
	void Invisible();

protected:
	GUIFontManager* fontManager_;
	GUIImageManager* imageManager_;
	GUIRenderer* renderer_;
    IMapManager* mapManager_;
	IAudioManager* _audioManager;
	MiniKernel* _kernel;
	SDL_Color backgroundColor_;
	SDL_Color foregroundColor_;
	GUIPoint topLeft_;
	bool hidden_;
	bool hasFocus_;
	bool needRedraw_;
	bool mouseOver_;
	bool buttonDown_;
	bool hasBorder_;
	bool enabled_;
	bool selected_;
    Uint32 windowId_;
	void Invalidate();
	bool ReDraw();
	
	GUISize Size() const;
	GUITexture* Texture() const;
	GUIPoint TopLeft() const;
	GUIPoint ScreenTopLeft() const;
	bool NeedRedraw() const;
	void Create(GUIRenderer* renderer, GUIFontManager* fontManager);
	void Create(GUIRenderer* renderer, GUIElement* parent, GUIFontManager* fontManager, SDLEventManager* eventManager, GUIImageManager* imageManager, IMapManager* mapManager, IAudioManager* audioManager, MiniKernel* kernel, Uint32 windowId);
	void HandleEventBase(GUIEvent& event);
	void SetRedraw();
	void SetRenderer(GUITexture* texture = nullptr) const;
	GUIPoint ScreenToElementCoords(const GUIPoint& point) const;

public:
	GUIElement(GUIPoint position, GUISize size, const std::string& name);
	virtual ~GUIElement();

	std::string ToString() const;
	std::string ToXml() const;

	AnchorFlags Anchor(AnchorFlags add, AnchorFlags remove);
	SDLEventManager* EventManager() override;
	virtual GUIElement* HitTest(GUIPoint point);

};

