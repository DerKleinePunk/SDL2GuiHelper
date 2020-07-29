#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIScreen"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIScreen.h"
#include "GUIElement.h"
#include "../../common/exception/NullPointerException.h"
#include "GUIElementManager.h"
#include "../exception/GUIException.h"
#include "GUIRenderer.h"
#include "GUIImageManager.h"
#include "GUIScreenCanvas.h"

#ifdef LIBOSMSCOUT
#include "../map/MapManager.h"
#endif

constexpr int OPENGL_MAJOR_VERSION = 2;
constexpr int OPENGL_MINOR_VERSION = 1;

constexpr SDL_GLprofile OPENGL_PROFILE = SDL_GLprofile::SDL_GL_CONTEXT_PROFILE_CORE;

bool GUIScreen::HandleWindowEvent(const SDL_Event* event) const {
	switch (event->window.event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_RESIZED:
			LOG(DEBUG) << "Window " << id_ << " resized to " << event->window.data1 << " x "<< event->window.data2;
			Resize(event->window.data1, event->window.data2);
			break;
		case SDL_WINDOWEVENT_EXPOSED:
		    //Todo After Screen Saver window is Black an this is only event we get (Windows)
			LOG(DEBUG) << "Window " << id_ << " EXPOSED";
			manager_->Invalidate();
			Draw();
			break;
		default:
			return false;
	}
	return true;
}

GUIScreen::GUIScreen():
	window_(nullptr),
	renderer_(nullptr),
	manager_(nullptr), 
	imageManager_(nullptr),
	id_(0),
	canvas_(nullptr),
	eventManager_(nullptr) {
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	size_.set(0,0);
}

GUIScreen::~GUIScreen()
{
	if (window_ != nullptr)
	{
		SDL_DestroyWindow(window_);
		window_ = nullptr;
	}

	if (manager_ != nullptr)
	{
		delete manager_;
		manager_ = nullptr;
	}

	if(canvas_ != nullptr)
	{
		delete canvas_;
		canvas_ = nullptr;
	}

	if (renderer_ != nullptr)
	{
		delete renderer_;
		renderer_ = nullptr;
	}
}

GUIElementManager* GUIScreen::Create(std::string title, SDLEventManager* eventManager, IMapManager* mapManager, const std::string& backgroundImage)
{
	if (eventManager == nullptr) {
		throw NullPointerException("eventManager can not be null");
	}

	eventManager_ = eventManager;
	size_ = GUISize(1024, 600);

    /*SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, OPENGL_PROFILE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);*/

	//todo Check Windows Enbaled or not (Android/Raspbarry and so on)
	window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size_.width, size_.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if(window_ == nullptr)
	{
		throw GUIException("SDL_CreateWindow");
	}
	
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		LOG(WARNING) << "Linear texture filtering not enabled!";
	}

	/*if (!SDL_SetHint(SDL_HINT_RENDER_BATCHING, "0"))
	{
		LOG(WARNING) << "RENDER_BATCHING not disabled!";
	}*/

	try
	{
		renderer_ = new GUIRenderer();
		renderer_->Create(window_);
		imageManager_ = new GUIImageManager(renderer_);
	}
	catch (std::exception const& exp)
	{
		delete renderer_;
		renderer_ = nullptr;
		SDL_DestroyWindow(window_);
		window_ = nullptr;
		throw;
	}

    id_ = SDL_GetWindowID(window_);
    canvas_ = new GUIScreenCanvas(size_, backgroundImage);
    manager_ = new GUIElementManager(renderer_, canvas_, eventManager_, imageManager_, mapManager, id_);
    canvas_->imageManager_ = imageManager_;

	auto info = renderer_->GetInfo();

	LOG(INFO) << "Using Renderer " << info.name;

	canvas_->Init();

	return manager_;
}

Uint32 GUIScreen::GetId() const
{
	return id_;
}

void GUIScreen::UpdateAnimationInternal() const
{
	manager_->UpdateAnimation();
}

void GUIScreen::Draw() const {
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
	TIMED_FUNC_IF(redrawTimer, VLOG_IS_ON(4));
#endif
	//wenn user draw ever wer on en Textture clear clear this
	renderer_->RenderTarget(nullptr);
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerBlkObjclear, "clear", VLOG_IS_ON(4));
#endif
    renderer_->Clear();
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerBlkObjDrawTree, "DrawTree", VLOG_IS_ON(4));
#endif
    manager_->DrawTree();
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerBlkObjPresent, "Present", VLOG_IS_ON(4));
#endif
    renderer_->Present();
}

void GUIScreen::Resize(Sint32 width, Sint32 height) const {
	canvas_->Resize(GUISize(width, height));
	manager_->Invalidate();
}

void GUIScreen::HandleEvent(const SDL_Event* event) const
{
	switch (event->type)
	{
		case SDL_RENDER_TARGETS_RESET:
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Target Reset");
			break;
		case SDL_WINDOWEVENT:
			if (HandleWindowEvent(event))
				break;
		default:
			auto currentEvent = GUIEvent(event);
			manager_->HandleEvent(currentEvent);
	}
	
}

bool GUIScreen::NeedRedraw() const
{
	return manager_->NeedRedraw();
}

void GUIScreen::ToggleFullscreen() {
	bool IsFullscreen = SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN;
	SDL_SetWindowFullscreen(window_, IsFullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
}

void GUIScreen::DrawTextOnBootScreen(const std::string& text) {
	canvas_->Text(text);
}

void GUIScreen::Shutdown()
{
	if (manager_ != nullptr) {
		manager_->CloseAll();
		delete manager_;
		manager_ = nullptr;
		//manager_ is deleting the rootNode 
		canvas_ = nullptr;
	}

	if(imageManager_ != nullptr) {
		delete imageManager_;
		imageManager_ = nullptr;
	}

	if (canvas_ != nullptr)	{
		delete canvas_;
		canvas_ = nullptr;
	}

	if (renderer_ != nullptr) {
		delete renderer_;
		renderer_ = nullptr;
	}

	if (window_ != nullptr)	{
		SDL_DestroyWindow(window_);
		window_ = nullptr;
	}
}
