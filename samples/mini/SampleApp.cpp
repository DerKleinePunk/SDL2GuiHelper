#include "SampleApp.h"

#include <chrono>
#include <thread>

#include "../../src/Elements.h"

void SampleApp::ApplicationEvent(AppEvent event, void* data1, void* data2)
{
}

void SampleApp::KernelstateChanged(KernelState state)
{
    if(state == KernelState::Startup) {
        LOG(INFO) << "Kernel is up Create Screen";

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
        TIMED_SCOPE_IF(timerCreateScreen, "CreateScreen", VLOG_IS_ON(4));
#endif
        _manager = _kernel->CreateScreen("SDL2 Gui Mini Sample", "", "", false);
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

        // mediaManager_ = kernel_->GetMediaManger();

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
        TIMED_SCOPE_IF(timerBuildFirstScreen, "BuildFirstScreen", VLOG_IS_ON(4));
#endif

        _kernel->DrawTextOnBootScreen("Services Started\nStarted Audio\nBuilding Screen");

        BuildFirstScreen();
    }
}

void SampleApp::BuildFirstScreen()
{
    auto statusLine = new GUITestElement(GUIPoint(0, 0), GUISize(100, 4, sizeType::relative), lightgray_t_color, "statusLine");
    _manager->AddElement(statusLine);
    statusLine->Anchor(AnchorFlags::Right, AnchorFlags::Left);

    auto uhrTextlabel =
    new GUITextLabel(GUIPoint(300, 0), GUISize(11, 100, sizeType::relative), "Uhrzeit", transparent_color, own_blue_color);
    _manager->AddElement(statusLine, uhrTextlabel);
    uhrTextlabel->Text("Hallo");
    uhrTextlabel->ShowTime(true);
    uhrTextlabel->Anchor(AnchorFlags::Left, AnchorFlags::Right);
    uhrTextlabel->TextAnchor(AnchorFlags::Left & AnchorFlags::Right);

    auto status1Button = new GUITextButton(GUIPoint(30, 40), GUISize(200, 80), "STATUS-1", own_blue_color, white_color);
    _manager->AddElement(status1Button);
    status1Button->FontHeight(48);
    status1Button->SetCorner(5);
    status1Button->Text("1");
    status1Button->RegisterOnClick([this](IGUIElement* sender) { SendRadioState(1); });

    auto status2Button = new GUITextButton(GUIPoint(30, 125), GUISize(200, 80), "STATUS-2", own_blue_color, white_color);
    _manager->AddElement(status2Button);
    status2Button->FontHeight(48);
    status2Button->SetCorner(5);
    status2Button->Text("2");

    auto status3Button = new GUITextButton(GUIPoint(30, 210), GUISize(200, 80), "STATUS-3", own_blue_color, white_color);
    _manager->AddElement(status3Button);
    status3Button->FontHeight(48);
    status3Button->SetCorner(5);
    status3Button->Text("3");

    auto sdsListview = new GUIListview(GUIPoint(250, 40), GUISize(700, 400), "sdsListview", lightgray_t_color, lightblack_color);
    // auto songlistLongClickdelegate = std::bind(&CarPC::SongListLongClick, this, std::placeholders::_1,
    // std::placeholders::_2, std::placeholders::_3); songListview->RegisterOnLongClick(songlistLongClickdelegate);
    _manager->AddElement(sdsListview);
    sdsListview->ChangeRowHasImage(false);
    sdsListview->ChangeRowHasDetails(false);

    LoadSdsList();

    auto menuButton = new GUIRoundPictureButton(GUIPoint(30, 300), GUISize(48, 48), "MapMenuButton", own_blue_color, white_color);
    _manager->AddElement(menuButton);
    menuButton->Image("map_menu.png");
    menuButton->Border(false);
    menuButton->PictureSize(GUISize(30, 30));
    menuButton->Visible();

    // Clear Boot Log
    _kernel->DrawTextOnBootScreen("");

#ifdef DEBUG
    _manager->PrintVisualTree();
#endif
}

void SampleApp::LoadSdsList()
{
    auto element = static_cast<GUIListview*>(_manager->GetElementByName("sdsListview"));
    element->BeginUpdate();
    element->Clear();
    GUIListviewRow row;
    std::shared_ptr<GUIListviewColumn> column(new GUIListviewColumn("SMS mit einem Text ist angekommen"));
    // column->SetDetailText("SDS mit einem Text ist angekommen");
    row.AddColumn(column);
    row.Tag = (void*)1;
    row.selected = true;
    element->AddRow(row);

    GUIListviewRow row2;
    std::shared_ptr<GUIListviewColumn> column2(new GUIListviewColumn("Zweite SMS mit einem Anderen Text"));
    column2->SetDetailText("DetailText in Column g p");
    row2.AddColumn(column2);
    row2.Tag = (void*)2;
    element->AddRow(row2);

    GUIListviewRow row3;
    std::shared_ptr<GUIListviewColumn> column3(new GUIListviewColumn("Dritte SMS mit einem Anderen Text"));
    // column->SetDetailText("DetailText in Column");
    row3.AddColumn(column3);
    row3.Tag = (void*)3;
    element->AddRow(row3);

    GUIListviewRow row4;
    std::shared_ptr<GUIListviewColumn> column4(new GUIListviewColumn("4"));
    // column->SetDetailText("DetailText in Column");
    row4.AddColumn(column4);
    row4.Tag = (void*)4;
    element->AddRow(row4);

    GUIListviewRow row5;
    std::shared_ptr<GUIListviewColumn> column5(new GUIListviewColumn("5"));
    // column->SetDetailText("DetailText in Column");
    row5.AddColumn(column5);
    row5.Tag = (void*)5;
    element->AddRow(row5);

    GUIListviewRow row6;
    std::shared_ptr<GUIListviewColumn> column6(new GUIListviewColumn("6"));
    // column->SetDetailText("DetailText in Column");
    row6.AddColumn(column6);
    row6.Tag = (void*)6;
    element->AddRow(row6);

    for(auto i = 6; i < 30; i++) {
        GUIListviewRow row7;
        std::shared_ptr<GUIListviewColumn> column7(new GUIListviewColumn(std::to_string(i) + " Zeile"));
        if(i % 2 == 0) {
            column7->SetDetailText("DetailText in Column");
        }
        row7.AddColumn(column7);
        row7.Tag = (void*)(intptr_t)i;
        element->AddRow(row7);
    }

    element->EndUpdate();
}

void SampleApp::SendRadioState(int state)
{
    _kernel->ShowErrorMessage("Button gedrückt");
}

SampleApp::SampleApp(MiniKernel* kernel)
{
    if(kernel == nullptr) {
    }

    _kernel = kernel;
}

SampleApp::~SampleApp()
{
}

void SampleApp::Startup()
{
    auto statedelegate = std::bind(&SampleApp::KernelstateChanged, this, std::placeholders::_1);
    _kernel->SetStateCallBack(statedelegate);
    auto eventdelegate =
    std::bind(&SampleApp::ApplicationEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    _kernel->RegisterApplicationEvent(eventdelegate);
}

void SampleApp::Shutdown()
{
}
