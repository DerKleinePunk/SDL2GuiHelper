#pragma once

class GUITexture;
class GUIRenderer;

class GUIImageManager {
	std::map<std::string, GUITexture*> images_;
	GUIRenderer* renderer_;
public:
	explicit GUIImageManager(GUIRenderer* renderer);
	~GUIImageManager();
	GUITexture* GetImage(std::string fileName);
};
