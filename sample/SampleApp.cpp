#include "SampleApp.h"

void SampleApp::ApplicationEvent(AppEvent event, void* data1, void* data2) {

}

void SampleApp::KernelstateChanged(KernelState state) {
    if (state == KernelState::Startup) {
        LOG(INFO) << "Start Services";
/*
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerBlkObjclear, "StartServices", VLOG_IS_ON(4));
#endif
        StartServices();
        
        LOG(INFO) << "Kernel is up Create Screen";
*/
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerCreateScreen, "CreateScreen", VLOG_IS_ON(4));
#endif
        _manager = _kernel->CreateScreen("SDL2 Gui Sample", "");
/*
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerStartAudio, "StartAudio", VLOG_IS_ON(4));
#endif		
        StartAudio();

		mediaManager_ = kernel_->GetMediaManger();
        */

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerBuildFirstScreen, "BuildFirstScreen", VLOG_IS_ON(4));
#endif	
		BuildFirstScreen();

    }
}

void SampleApp::BuildFirstScreen() {

#ifdef DEBUG
    _manager->PrintVisualTree();
#endif

}

SampleApp::SampleApp(MiniKernel* kernel) {
    if(kernel == nullptr) {

    }

    _kernel = kernel;
}

SampleApp::~SampleApp() {

}

void SampleApp::Startup() {
    auto statedelegate = std::bind(&SampleApp::KernelstateChanged, this, std::placeholders::_1);
    _kernel->SetStateCallBack(statedelegate);
    auto eventdelegate = std::bind(&SampleApp::ApplicationEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    _kernel->RegisterApplicationEvent(eventdelegate);
}