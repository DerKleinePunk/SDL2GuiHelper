#include "SampleApp.h"
#include "../src/Elements.h"
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
        _manager = _kernel->CreateScreen("SDL2 Gui Sample", "", "");
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
		
        _kernel->DrawTextOnBootScreen("Services Started\nStarted Audio\nBuilding Screen");

        BuildFirstScreen();

    }
}

void SampleApp::BuildFirstScreen() {
    auto statusLine = new GUITestElement(GUIPoint(0, 0), GUISize(100, 4, sizeType::relative), lightgray_t_color, "statusLine");
    _manager->AddElement(statusLine);
	statusLine->Anchor(AnchorFlags::Right, AnchorFlags::Left);

    auto uhrTextlabel = new GUITextLabel(GUIPoint(300, 0), GUISize(11, 100, sizeType::relative), "Uhrzeit", transparent_color, own_blue_color);
    _manager->AddElement(statusLine, uhrTextlabel);
    uhrTextlabel->Text("Hallo");
    uhrTextlabel->ShowTime(true);
    uhrTextlabel->Anchor(AnchorFlags::Left, AnchorFlags::Right);
    uhrTextlabel->TextAnchor(AnchorFlags::Left & AnchorFlags::Right);
    
    auto status1Button = new GUITextButton(GUIPoint(40, 40), GUISize(40, 40), "STATUS-1", own_blue_color, white_color);
    _manager->AddElement(status1Button);
    status1Button->Text("1");
    
    auto status2Button = new GUITextButton(GUIPoint(85, 40), GUISize(40, 40), "STATUS-2", own_blue_color, white_color);
    _manager->AddElement(status2Button);
    status2Button->Text("2");

    auto status3Button = new GUITextButton(GUIPoint(130, 40), GUISize(40, 40), "STATUS-3", own_blue_color, white_color);
    _manager->AddElement(status3Button);
    status3Button->Text("3");

    auto sdsListview = new GUIListview(GUIPoint(250, 40), GUISize(700, 400),"sdsListview", lightgray_t_color,lightblack_color);
	//auto songlistLongClickdelegate = std::bind(&CarPC::SongListLongClick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//songListview->RegisterOnLongClick(songlistLongClickdelegate);
	_manager->AddElement(sdsListview);
    sdsListview->ChangeRowHasImage(false);
    sdsListview->ChangeRowHasDetails(false);
    
    LoadSdsList();

    //Clear Boot Log
    _kernel->DrawTextOnBootScreen("");

#ifdef DEBUG
    _manager->PrintVisualTree();
#endif

}

void SampleApp::LoadSdsList() {
    auto element = static_cast<GUIListview*>(_manager->GetElementByName("sdsListview"));
    element->BeginUpdate();
    element->Clear();
    GUIListviewRow row;
    std::shared_ptr<GUIListviewColumn> column(new GUIListviewColumn("SDS mit einem Text ist angekommen"));
    //column->SetDetailText("SDS mit einem Text ist angekommen");
    row.AddColumn(column);
    row.Tag = (void*)1;
    element->AddRow(row);
    
    GUIListviewRow row2;
    std::shared_ptr<GUIListviewColumn> column2(new GUIListviewColumn("Zweite SDS mit einem Anderen Text"));
    //column->SetDetailText("SDS mit einem Text ist angekommen");
    row2.AddColumn(column2);
    row2.Tag = (void*)2;
    element->AddRow(row2);

    element->EndUpdate();
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