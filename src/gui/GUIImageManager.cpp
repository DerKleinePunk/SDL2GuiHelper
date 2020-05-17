#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIImageManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../../common/easylogging/easylogging++.h"
#include "../../common/exception/NullPointerException.h"
#include "GUIImageManager.h"
#include "GUIRenderer.h"
#include "GUITexture.h"

GUIImageManager::GUIImageManager(GUIRenderer* renderer) {
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	if(renderer == nullptr) {
		throw NullPointerException("renderer can not be null"); 
	}
	renderer_ = renderer;
}

GUIImageManager::~GUIImageManager() {
	VLOG(3) << "dtor";

	auto entry = images_.begin();
	while (entry != images_.end()) {
		delete entry->second;
		++entry;
	}
	images_.clear();
}

GUITexture* GUIImageManager::GetImage(const std::string& fileName) {
	if(images_.find(fileName) != images_.end()) {
		return images_[fileName];
	};
    LOG(DEBUG) << "Image not in Cache load " << fileName;
	auto image = renderer_->LoadImageTexture(fileName);
	images_.insert(std::make_pair(fileName, image));

	return image;
}

void GUIImageManager::RemoveImage(const std::string& fileName) {
	const auto p = images_.find(fileName);
	if(p != images_.end()) {
		delete p->second;
		images_.erase(p);
	};
}
