#pragma once

#include <map>
#include <vector>

#include "../IAudioManager.h"
#include "../IMapManager.h"

struct GUIEvent;
class GUIRenderer;
class GUIElement;
class GUIElementTreeNode;
class GUIScreenCanvas;
class GUIFontManager;
class GUIImageManager;
class SDLEventManager;
class MiniKernel;

typedef std::vector<GUIElementTreeNode*> ListOfElements;
typedef std::map<GUIElementTreeNode*, GUIElementTreeNode*> TreeMapOfElements;

class GUIElementTreeNode
{
    GUIElement* element_;
    ListOfElements children_;

  public:
    explicit GUIElementTreeNode(GUIElement* element);
    ~GUIElementTreeNode();
    void Add(GUIElement* element);
    void Add(GUIElementTreeNode* element);
    GUIElementTreeNode* Find(const GUIElement* element);
    GUIElement* Element() const;
    bool IsVisible() const;
    bool Draw(GUIRenderer* renderer);
    void PrintVisualTree(std::string& traceText);
    void UpdateAnimation();
    void HandleEvent(GUIEvent& event);
    bool NeedRedraw();
    void Invalidate();
    GUIElement* Close();
    void RemoveChild(GUIElement* element);
    GUIElement* GetElementByName(const std::string& name);
    void DisableElement();
    void EnableElement();
    void VisibleElement() const;
    void InvisibleElement() const;
};

class GUIElementManager
{
    GUIElementTreeNode rootNode_;
    GUIRenderer* renderer_;
    GUIFontManager* fontManager_;
    bool inEvent_;
    TreeMapOfElements tempElements_;
    SDLEventManager* eventManager_;
    GUIImageManager* imageManager_;
    IMapManager* mapManager_;
    uint32_t windowId_;
    GUIElementTreeNode* _modalElement;
    IAudioManager* _audioManager;
    MiniKernel* _kernel;

    bool IsMouseEvent(GUIEvent& event);

  public:
    GUIElementManager(GUIRenderer* renderer,
                      GUIScreenCanvas* canvas,
                      SDLEventManager* eventManager,
                      GUIImageManager* imageManager,
                      IMapManager* mapManager,
                      IAudioManager* audioManager,
                      MiniKernel* kernel,
                      uint32_t windowId);
    virtual ~GUIElementManager();

    void AddElement(GUIElement* element);
    void AddElementBefore(GUIElement* elementToAdd, const std::string& nameBefore);
    void AddElement(GUIElement* parent, GUIElement* element);
    void Close(GUIElement* element);
    void CloseAll();
    void DrawTree();
    void PrintVisualTree();
    void UpdateAnimation();
    void HandleEvent(GUIEvent& event);
    bool NeedRedraw();
    void Invalidate();
    GUIElement* GetElementByName(std::string name);
    void DisableElement(GUIElement* element);
    void EnableElement(GUIElement* element);
    void VisibleElement(GUIElement* element);
    void InvisibleElement(GUIElement* element);
    void SetModalElement(const GUIElement* element);
};
