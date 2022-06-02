#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "SampleApp"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "SampleApp.h"
#include "../../src/Elements.h"
#include <chrono>
#include <thread>

const SDL_Color my_light_blue = { 0x00, 0xA1, 0x9C, 0xff };

void SampleApp::ApplicationEvent(AppEvent event, void* data1, void* data2) {

}

void SampleApp::KernelstateChanged(KernelState state) {
    if (state == KernelState::Startup) {
        LOG(INFO) << "Kernel Start Core Services";
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
        TIMED_SCOPE_IF(timerCoreServices, "StartCoreServices", VLOG_IS_ON(4));
#endif
        _kernel->StartCoreServices();

        LOG(INFO) << "Kernel is up Create Screen";

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerCreateScreen, "CreateScreen", VLOG_IS_ON(4));
#endif
        _manager = _kernel->CreateScreen("SDL2 Gui Musik Sample", "", "", false);
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
    
    BuildMainScreen();
    BuildMapScreen();
    BuildPlayerScreen();
    BuildAutoScreen();
    BuildCommandBar();

    //Clear Boot Log
    _kernel->DrawTextOnBootScreen("");

#ifdef DEBUG
    _manager->PrintVisualTree();
#endif

    UpdateUI(UiState::home);

    auto element = _manager->GetElementByName("commandBar");
    if(element != nullptr) {
        _manager->VisibleElement(element);
    }
}

void SampleApp::LoadSdsList() {
    auto element = static_cast<GUIListview*>(_manager->GetElementByName("sdsListview"));
    element->BeginUpdate();
    element->Clear();
    GUIListviewRow row;
    std::shared_ptr<GUIListviewColumn> column(new GUIListviewColumn("SMS mit einem Text ist angekommen"));
    row.AddColumn(column);
    row.Tag = (void*)1;
    element->AddRow(row);
    
    GUIListviewRow row2;
    std::shared_ptr<GUIListviewColumn> column2(new GUIListviewColumn("Zweite SMS mit einem Anderen Text"));
    row2.AddColumn(column2);
    row2.Tag = (void*)2;
    element->AddRow(row2);

    element->EndUpdate();
}

void SampleApp::SendRadioState(int state) 
{
    _kernel->ShowErrorMessage("Button gedrückt");
}

void SampleApp::BuildMainScreen()
{
    auto mainScreenBackground =
    new GUITestElement(GUIPoint(0, 0), GUISize(100, 100, sizeType::relative), transparent_color, "mainScreen");
    mainScreenBackground->Invisible();
    _manager->AddElement(mainScreenBackground);


    auto status1Button = new GUITextButton(GUIPoint(30, 40), GUISize(200, 80), "STATUS-1", own_blue_color, white_color);
    _manager->AddElement(mainScreenBackground, status1Button);
    status1Button->FontHeight(48);
    status1Button->SetCorner(5);
    status1Button->Text("1");
    status1Button->RegisterOnClick([this](IGUIElement* sender) { SendRadioState(1); });
    
    auto status2Button = new GUITextButton(GUIPoint(30, 125), GUISize(200, 80), "STATUS-2", own_blue_color, white_color);
    _manager->AddElement(mainScreenBackground, status2Button);
    status2Button->FontHeight(48);
    status2Button->SetCorner(5);
    status2Button->Text("2");

    auto status3Button = new GUITextButton(GUIPoint(30, 210), GUISize(200, 80), "STATUS-3", own_blue_color, white_color);
    _manager->AddElement(mainScreenBackground, status3Button);
    status3Button->FontHeight(48);
    status3Button->SetCorner(5);
    status3Button->Text("3");

    auto sdsListview = new GUIListview(GUIPoint(250, 40), GUISize(700, 400),"sdsListview", lightgray_t_color, my_light_blue);
    _manager->AddElement(mainScreenBackground, sdsListview);
    sdsListview->ChangeRowHasImage(false);
    sdsListview->ChangeRowHasDetails(false);
    
    LoadSdsList();

}

void SampleApp::BuildMapScreen()
{
    auto mapScreenBackground =
    new GUITestElement(GUIPoint(0, 0), GUISize(100, 100, sizeType::relative), transparent_color, "mapScreen");
    _manager->AddElement(mapScreenBackground);
    mapScreenBackground->Invisible();

    _mapDialog = new MapDialog(mapScreenBackground, _manager, _kernel->GetEventManager());
    _mapDialog->Init();
}

void SampleApp::BuildPlayerScreen()
{
    auto playerScreenBackground =
    new GUITestElement(GUIPoint(0, 0), GUISize(100, 100, sizeType::relative), transparent_color, "playerScreen");
    _manager->AddElement(playerScreenBackground);
    playerScreenBackground->Invisible();

    _playerDialog = new MusikPlayerDialog(playerScreenBackground, _manager, _kernel->GetEventManager());
    _playerDialog->Init();
}

void SampleApp::BuildAutoScreen()
{
    auto playerScreenBackground =
    new GUITestElement(GUIPoint(0, 0), GUISize(100, 100, sizeType::relative), transparent_color, "autoScreen");
    _manager->AddElement(playerScreenBackground);
    playerScreenBackground->Invisible();

    _autoDialog = new AutomationDialog(playerScreenBackground, _manager, _kernel->GetEventManager());
    _autoDialog->Init();
}

void SampleApp::BuildCommandBar()
{
    auto commandBar = new GUITestElement(GUIPoint(0, 500), GUISize(1024, 100), transparent_color, "commandBar");
    _manager->AddElement(commandBar);
    commandBar->Invisible();

    auto tempButton = new GUITextButton(GUIPoint(15, 5), GUISize(150, 90), "homeCommandButton",
                                        own_red_color, white_color);
    tempButton->FontHeight(40);
    tempButton->Text("Home");
    tempButton->RegisterOnClick([this](IGUIElement* sender) { UpdateUI(UiState::home); });
    _manager->AddElement(commandBar, tempButton);

    auto buttonstartPos = 175;

    tempButton = new GUITextButton(GUIPoint(buttonstartPos, 5), GUISize(150, 90),
                                   "mapCommandButton", own_red_color, white_color);
    tempButton->FontHeight(40);
    tempButton->Text("Karte");
    tempButton->RegisterOnClick([this](IGUIElement* sender) { UpdateUI(UiState::map); });
    _manager->AddElement(commandBar, tempButton);

    buttonstartPos += 175;

    tempButton = new GUITextButton(GUIPoint(buttonstartPos, 5), GUISize(150, 90),
                                   "playerCommandButton", own_red_color, white_color);
    tempButton->FontHeight(40);
    tempButton->Text("Musik");
    tempButton->RegisterOnClick([this](IGUIElement* sender) { UpdateUI(UiState::musikPlayer); });
    _manager->AddElement(commandBar, tempButton);

    buttonstartPos += 175;

    tempButton = new GUITextButton(GUIPoint(buttonstartPos, 5), GUISize(150, 90),
                                   "automationCommandButton", own_red_color, white_color);
    tempButton->FontHeight(40);
    tempButton->Text("Haus");
    tempButton->RegisterOnClick([this](IGUIElement* sender) { UpdateUI(UiState::automation); });
    _manager->AddElement(commandBar, tempButton);
}

void SampleApp::UpdateUI(UiState newUIState)
{
    if(_appUiStateCurrent == newUIState) return;

    GUIElement* element = nullptr;
    switch(newUIState) {
    case UiState::home:
        element = _manager->GetElementByName("mainScreen");
        if(element != nullptr) {
            _manager->VisibleElement(element);
        }
        element = _manager->GetElementByName("mapScreen");
        if(element != nullptr) {
            _mapDialog->Hide();
        }
        element = _manager->GetElementByName("playerScreen");
        if(element != nullptr) {
            _playerDialog->Hide();
        }
        element = _manager->GetElementByName("autoScreen");
        if(element != nullptr) {
            _autoDialog->Hide();
        }
        element = _manager->GetElementByName("homeCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_selected_command_color);
        }
        element = _manager->GetElementByName("sdsCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("mapCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("playerCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        break;
    case UiState::sds:
        element = _manager->GetElementByName("mainScreen");
        if(element != nullptr) {
            _manager->InvisibleElement(element);
        }
        element = _manager->GetElementByName("mapScreen");
        if(element != nullptr) {
            _mapDialog->Hide();
        }
        element = _manager->GetElementByName("playerScreen");
        if(element != nullptr) {
            _playerDialog->Hide();
        }
        element = _manager->GetElementByName("autoScreen");
        if(element != nullptr) {
            _autoDialog->Hide();
        }
        element = _manager->GetElementByName("homeCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("sdsCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_selected_command_color);
        }
        element = _manager->GetElementByName("mapCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("playerCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        break;
    case UiState::map:
        element = _manager->GetElementByName("mainScreen");
        if(element != nullptr) {
            _manager->InvisibleElement(element);
        }
        element = _manager->GetElementByName("mapScreen");
        if(element != nullptr) {
            _mapDialog->Show();
        }
        element = _manager->GetElementByName("playerScreen");
        if(element != nullptr) {
            _playerDialog->Hide();
        }
        element = _manager->GetElementByName("autoScreen");
        if(element != nullptr) {
            _autoDialog->Hide();
        }
        element = _manager->GetElementByName("homeCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("sdsCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("mapCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_selected_command_color);
        }
        element = _manager->GetElementByName("playerCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        break;
    case UiState::musikPlayer:
        element = _manager->GetElementByName("mainScreen");
        if(element != nullptr) {
            _manager->InvisibleElement(element);
        }
        element = _manager->GetElementByName("mapScreen");
        if(element != nullptr) {
            _manager->InvisibleElement(element);
        }
        element = _manager->GetElementByName("playerScreen");
        if(element != nullptr) {
            _playerDialog->Show();
        }
        element = _manager->GetElementByName("autoScreen");
        if(element != nullptr) {
            _autoDialog->Hide();
        }
        element = _manager->GetElementByName("homeCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("sdsCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("mapCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("playerCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_selected_command_color);
        }
        element = _manager->GetElementByName("automationCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        break;
    case UiState::automation:
        element = _manager->GetElementByName("mainScreen");
        if(element != nullptr) {
            _manager->InvisibleElement(element);
        }
        element = _manager->GetElementByName("mapScreen");
        if(element != nullptr) {
            _manager->InvisibleElement(element);
        }
        element = _manager->GetElementByName("playerScreen");
        if(element != nullptr) {
            _playerDialog->Hide();
        }
        element = _manager->GetElementByName("autoScreen");
        if(element != nullptr) {
            _autoDialog->Show();
        }
        element = _manager->GetElementByName("homeCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("sdsCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("mapCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("playerCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_red_color);
        }
        element = _manager->GetElementByName("automationCommandButton");
        if(element != nullptr) {
            static_cast<GUITextButton*>(element)->ChangeBackColor(own_selected_command_color);
        }
        break;
    default:
        LOG(ERROR) << "Unkown UI State";
        break;
    }
    _appUiStateCurrent = newUIState;
}


SampleApp::SampleApp(MiniKernel* kernel) {
    if(kernel == nullptr) {

    }

    _kernel = kernel;
    _appUiStateCurrent = UiState::undefined;
    _mapDialog = nullptr;
    _playerDialog = nullptr;

    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}

SampleApp::~SampleApp() {

}

void SampleApp::Startup() {
    auto statedelegate = std::bind(&SampleApp::KernelstateChanged, this, std::placeholders::_1);
    _kernel->SetStateCallBack(statedelegate);
    auto eventdelegate = std::bind(&SampleApp::ApplicationEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    _kernel->RegisterApplicationEvent(eventdelegate);

    KernelConfig config;
    //config.AudioFileForClick = _config->GetSoundForClick();
    //config.AudioFileForLongClick = _config->GetSoundForClick();
    config.mapDataPath = "/home/punky/develop/sdl2guitests/src/maps/europe/germany/hessen";
    std::vector<std::string> iconPaths;
    iconPaths.push_back("/home/punky/develop/libosmscout/libosmscout/data/icons/14x14/standard/");
    iconPaths.push_back("/home/punky/develop/libosmscout/libosmscout/data/icons/svg/standard/");
    config.mapIconPaths = iconPaths;
    config.mapStyle = "/home/punky/develop/libosmscout/stylesheets/standard.oss";
    //config.markerImageFile = _config->GetMarkerImageFile();
    config.startMapPosition.Set(50.4090,9.3671);
    config.BackgroundScreen = black_color;
    config.ForegroundScreen = my_light_blue;

    _kernel->SetConfig(config);
}

void SampleApp::Shutdown() {
    if(_mapDialog != nullptr) {
        delete _mapDialog;
        _mapDialog = nullptr;
    }

    if(_playerDialog != nullptr) {
        delete _playerDialog;
        _playerDialog = nullptr;
    }
}
