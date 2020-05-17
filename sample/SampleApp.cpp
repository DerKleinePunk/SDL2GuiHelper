#include "SampleApp.h"
#include <chrono>
#include <thread>

void SampleApp::ApplicationEvent(AppEvent event, void* data1, void* data2) {

}

void SampleApp::KernelstateChanged(KernelState state) {
    if (state == KernelState::Startup) {
        LOG(INFO) << "Kernel is up Create Screen";

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerCreateScreen, "CreateScreen", VLOG_IS_ON(4));
#endif
        _manager = _kernel->CreateScreen("SDL2 Gui Sample", "");
        _kernel->UpdateScreens();
        _kernel->DrawTextOnBootScreen("Starting Services");

        LOG(INFO) << "Starting Services";
/*
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerBlkObjclear, "StartServices", VLOG_IS_ON(4));
#endif
        StartServices();
*/
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        _kernel->DrawTextOnBootScreen("Services Started\nStarting Audio");

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerStartAudio, "StartAudio", VLOG_IS_ON(4));
#endif		
        _kernel->StartAudio("");
		//mediaManager_ = kernel_->GetMediaManger();

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

void SampleApp::Shutdown() {

}