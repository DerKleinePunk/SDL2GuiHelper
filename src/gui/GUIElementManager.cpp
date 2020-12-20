/**
* @file  GUIElementManager.cpp
*
* Implementation for Manager GUI Elements.
*
* @date  2015-04-10
*/

#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIElementManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif


#include "../../common/easylogging/easylogging++.h"
#include "GUI.h"
#include "../../common/exception/NullPointerException.h"
#include "../../common/exception/NotImplementedException.h"
#include "../../common/exception/IllegalStateException.h"
#include "../../common/exception/ArgumentException.h"
#include "GUIElementManager.h"
#include "GUIElement.h"
#include "GUIRenderer.h"
#include "GUIFontManager.h"
#include "IPopupDialog.h"

#ifdef LIBOSMSCOUT
#include "../map/MapManager.h"
#endif

/**
* Create new Element for the element tree.
* @param element      pointer of the element 
*/
GUIElementTreeNode::GUIElementTreeNode(GUIElement* element) {
	element_ = element;
}

/**
* delete element from the element tree it also delete all children.
*/
GUIElementTreeNode::~GUIElementTreeNode() {
	auto child = children_.begin();
	while (child != children_.end()) {
		const auto node = (*child);
		delete node;
		child = children_.erase(child);
	}
	delete element_;
}

void GUIElementTreeNode::Add(GUIElement* element) {
    Add(new GUIElementTreeNode(element));
}

void GUIElementTreeNode::Add(GUIElementTreeNode* element) {
	children_.push_back(element);
	if(!element_->hidden_) {
		element_->Invalidate();
    }
}

GUIElementTreeNode* GUIElementTreeNode::Find(const GUIElement* element)
{
	if (element == element_) return this;
	auto child = children_.begin();
	while (child != children_.end()) {
		const auto result = (*child)->Find(element);
		if(result != nullptr)
		{
			return result;
		}
		++child;
	}

	return nullptr;
}

GUIElement* GUIElementTreeNode::Element() const {
	return element_;
}

bool GUIElementTreeNode::IsVisible() const {
	return !element_->hidden_;
}

bool GUIElementTreeNode::Draw(GUIRenderer* renderer) {
	const auto meNeedsDraw = element_->ReDraw();
	if (!meNeedsDraw && !NeedRedraw()) return false;

	auto child = children_.begin();
	while (child != children_.end()) {
		if ((*child)->IsVisible()) {
			if ((*child)->Draw(renderer) || meNeedsDraw) {
				renderer->RenderTarget(element_->Texture());
				const auto childElement = (*child)->Element();
				const auto texture = childElement->Texture();
				renderer->RenderCopy(texture, childElement->TopLeft());
				VLOG(3) << element_->ToString() << " " << childElement->ToString() << " copy Texture " << childElement->TopLeft().x << " x " << childElement->TopLeft().y;
			}
		}
		++child;
	}
	return true;
}

void GUIElementTreeNode::PrintVisualTree(std::string& traceText) {
	traceText.append(element_->ToXml());

	auto child = children_.begin();
	while (child != children_.end()) {
		(*child)->PrintVisualTree(traceText);
		++child;
	}
	traceText.append("</");
	traceText.append(element_->getTypeName());
	traceText.append(">");
}

void GUIElementTreeNode::UpdateAnimation()
{
	if(element_->hidden_) return;

	element_->UpdateAnimation();
	auto child = children_.begin();
	while (child != children_.end()) {
		(*child)->UpdateAnimation();
		++child;
	}
}

void GUIElementTreeNode::HandleEvent(GUIEvent& event)
{
	auto child = children_.rbegin();// last child first
	while (child != children_.rend()) {
		auto childElement = *child;
		if (childElement->IsVisible() || event.Type > SDL_USEREVENT) {
			childElement->HandleEvent(event);
		}
		++child;
	}
	element_->HandleEventBase(event);
}

bool GUIElementTreeNode::NeedRedraw() {
	if (element_->NeedRedraw()) {
		return true;
	}

	if(element_->hidden_) {
		return false;
	}

	auto child = children_.rbegin();// last child first
	while (child != children_.rend()) {
		if ((*child)->NeedRedraw()) {
			return true;
		}
		++child;
	}
	return false;
}

void GUIElementTreeNode::Invalidate()
{
	element_->Invalidate();
	auto child = children_.begin();
	while (child != children_.end()) {
		(*child)->Invalidate();
		++child;
	}
}

GUIElement* GUIElementTreeNode::Close()
{
	auto child = children_.begin();
	while (child != children_.end()) {
		(*child)->Close();
		delete (*child);
		++child;
	}
	children_.clear();

	element_->Close();
	const auto parent = element_->parent_;
	return parent;
}

void GUIElementTreeNode::RemoveChild(GUIElement* element)
{
	auto child = children_.begin();
	while (child != children_.end()) {
		auto node = (*child);
		if(node->Element() == element) {
			LOG(DEBUG) << element_->ToString() << " RemoveChild " << node->Element()->ToString();
			node->Close();
			children_.erase(child);
			delete node;
			break;
		}
		++child;
	}
	element_->Invalidate();
}

GUIElement* GUIElementTreeNode::GetElementByName(const std::string& name) {
	if(element_->name_ == name) {
		return element_;
	}

	auto child = children_.begin();
	while (child != children_.end()) {
		const auto element = (*child)->GetElementByName(name);
		if(element != nullptr) {
			return element;
		};
		++child;
	}

	return nullptr;
}

void GUIElementTreeNode::DisableElement() {
	element_->Disable();

	auto child = children_.begin();
	while (child != children_.end()) {
		(*child)->DisableElement();
		++child;
	}
}

void GUIElementTreeNode::EnableElement() {
	element_->Enable();

	auto child = children_.begin();
	while (child != children_.end()) {
		(*child)->EnableElement();
		++child;
	}
}

void GUIElementTreeNode::VisibleElement() const {
	element_->Visible();
    element_->parent_->Invalidate();
}

void GUIElementTreeNode::InvisibleElement() const {
	element_->Invisible();
}

bool GUIElementManager::IsMouseEvent(GUIEvent& event)
{
	if(event.Type == SDL_MOUSEBUTTONDOWN || 
		event.Type == SDL_MOUSEBUTTONUP ||
		event.Type == SDL_FINGERDOWN ||
		event.Type == SDL_FINGERUP) {
		return true;
	}
	return false;
}

GUIElementManager::GUIElementManager(GUIRenderer* renderer,GUIScreenCanvas* canvas, SDLEventManager* eventManager, GUIImageManager* imageManager, IMapManager* mapManager, Uint32 windowId):
	rootNode_(GUIElementTreeNode(reinterpret_cast<GUIElement*>(canvas))),
	inEvent_(false)
{
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);

	renderer_ = renderer;
	fontManager_ = new GUIFontManager();
	rootNode_.Element()->Create(renderer_, fontManager_);
	eventManager_ = eventManager;
    imageManager_ = imageManager;
    mapManager_ = mapManager;
    windowId_ = windowId;
}

GUIElementManager::~GUIElementManager() {
	if(fontManager_ != nullptr) {
		delete fontManager_;
		fontManager_ = nullptr;
	}
}

/**
* Add an new Element an other Element in Tree. When call in EventHandling add later wenn Event is Handeld
* @param element      the element to insert
*/
void GUIElementManager::AddElement(GUIElement* element)
{
	const auto test = rootNode_.GetElementByName(element->name_);
	if(test != nullptr) {
		throw ArgumentException("Name musst be unique");
	}

	element->Create(renderer_, rootNode_.Element(), fontManager_, eventManager_, imageManager_, mapManager_, windowId_);
	if (!inEvent_) {
        rootNode_.Add(element);
        element->Init();
	} else {
		tempElements_.insert(std::make_pair(&rootNode_, new GUIElementTreeNode(element)));
	}
}

/**
* Add an new Element before an other Element in Tree (NotImplemented).
* @param elementToAdd      the element to insert
* @param nameBefore        the element before insert the element (logik tree)
*/
void GUIElementManager::AddElementBefore(GUIElement* elementToAdd, const std::string& nameBefore) {
	UNUSED(elementToAdd);
	UNUSED(nameBefore);
	throw NotImplementedException("AddElementBefore");
	/*auto beforeElement = rootNode_.GetElementByName(nameBefore);
	if(beforeElement == nullptr) {
		throw NullPointerException("beforeElement not found");
	}
	beforeElement-
	elementToAdd->Create(renderer_, rootNode_.Element(), fontManager_, eventManager_);
	if (!inEvent_) {
		rootNode_.Add(elementToAdd);
	}
	else {
		throw NullPointerException("this not work in an event");
	}*/
}

void GUIElementManager::AddElement(GUIElement* parent, GUIElement* element) {
	auto test = rootNode_.GetElementByName(element->name_);
	if (test != nullptr) {
		throw ArgumentException("Name musst be unique");
	}
	auto node = rootNode_.Find(parent);
	if(node == nullptr) {
		auto elementMapEntry = tempElements_.begin();
		while (elementMapEntry != tempElements_.end()) {
			const auto nodeTemp = elementMapEntry->first;
			if(nodeTemp->Element() == parent) {
				node = nodeTemp;
				break;
			}
			if (elementMapEntry->second->Element() == parent) {
				elementMapEntry->second->Add(element);
				element->Create(renderer_, parent, fontManager_, eventManager_, imageManager_, mapManager_, windowId_);
				return;
			}
			++elementMapEntry;
		}
		if (node == nullptr) {
			throw NullPointerException("parent not found");
		}
	}
	element->Create(renderer_, parent, fontManager_, eventManager_, imageManager_, mapManager_, windowId_);
	if (!inEvent_) {
        node->Add(element);
        element->Init();
	} else {
		tempElements_.insert(std::make_pair(node, new GUIElementTreeNode(element)));
	}
}

/**
* close a element. Do not use this during Event handler for this use AppEvent::CloseEvent
* @param element      the element to close
*/
void GUIElementManager::Close(GUIElement* element) {
	if(inEvent_) {
		throw IllegalStateException("can't close an element in event Handler (element remove it self)");
	}

	auto node = rootNode_.Find(element);
	if (node == nullptr) {
		throw NullPointerException("element not found");
	}

	const auto parent = node->Close();
	auto nodeParent = rootNode_.Find(parent);
	if (nodeParent == nullptr) {
		throw NullPointerException("parent not found");
	}

	nodeParent->RemoveChild(element);

	GUIEvent helpEvent;
	helpEvent.Event.type = SDL_MOUSEMOTION;
	helpEvent.Type = SDL_MOUSEMOTION;
	SDL_GetMouseState(&helpEvent.Event.button.x, &helpEvent.Event.button.y);
	rootNode_.HandleEvent(helpEvent);
}

void GUIElementManager::CloseAll() {
	rootNode_.Close();
}

void GUIElementManager::PrintVisualTree() {
	std::string traceText("");
	rootNode_.PrintVisualTree(traceText);
	LOG(DEBUG) << traceText;
}

void GUIElementManager::UpdateAnimation() {
	rootNode_.UpdateAnimation();
}

/**
* give the Event to all element in the Tree the are visible
* if add new element during event add this to the visualtree
* @param event  the event
*/
void GUIElementManager::HandleEvent(GUIEvent& event) {

	if(_modalElement != nullptr && IsMouseEvent(event)) {
		_modalElement->HandleEvent(event);
		return;
	};

	inEvent_ = true;
	rootNode_.HandleEvent(event);
	inEvent_ = false;
	auto layoutChanced = false;
	auto elementMapEntry = tempElements_.begin();
	while(elementMapEntry != tempElements_.end()) {
		auto node = elementMapEntry->first;
		const auto element = elementMapEntry->second;
		node->Add(element);
        element->Element()->Init();
		++elementMapEntry;
		layoutChanced = true;
	}
	tempElements_.clear();
	if(layoutChanced)
	{
#ifdef DEBUG
		PrintVisualTree();
#endif
		GUIEvent helpEvent;
		helpEvent.Event.type = SDL_MOUSEMOTION;
		helpEvent.Type = SDL_MOUSEMOTION;
		SDL_GetMouseState(&helpEvent.Event.button.x, &helpEvent.Event.button.y);
		rootNode_.HandleEvent(helpEvent);
	}
}

bool GUIElementManager::NeedRedraw() {
	return rootNode_.NeedRedraw();
}

void GUIElementManager::Invalidate()
{
	rootNode_.Invalidate();
}

GUIElement* GUIElementManager::GetElementByName(std::string name) {
	return rootNode_.GetElementByName(name);
}

void GUIElementManager::DisableElement(GUIElement* element) {
	auto node = rootNode_.Find(element);
	if (node == nullptr) {
		throw NullPointerException("element not found");
	}
	node->DisableElement();
}

void GUIElementManager::EnableElement(GUIElement* element) {
	auto node = rootNode_.Find(element);
	if (node == nullptr) {
		throw NullPointerException("element not found");
	}
	node->EnableElement();
}

void GUIElementManager::VisibleElement(GUIElement* element) {
	const auto node = rootNode_.Find(element);
	if (node == nullptr) {
		throw NullPointerException("element not found");
	}
	node->VisibleElement();
}

void GUIElementManager::InvisibleElement(GUIElement* element) {
	const auto node = rootNode_.Find(element);
	if (node == nullptr) {
		throw NullPointerException("element not found");
	}
	node->InvisibleElement();
}

void GUIElementManager::DrawTree(){
    rootNode_.Draw(renderer_);
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerBlkObjDraw, "copy screen", VLOG_IS_ON(4));
#endif
    renderer_->RenderTarget(nullptr);
	const auto texture = rootNode_.Element()->Texture();
    renderer_->RenderCopy(texture, rootNode_.Element()->TopLeft());
}

void GUIElementManager::SetModalElement(const GUIElement* element) 
{
	if(element == nullptr) {
		_modalElement = nullptr;
		return;
	}

	_modalElement = rootNode_.Find(element);
}