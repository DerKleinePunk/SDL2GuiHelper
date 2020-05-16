#pragma once

enum class AppEvent : int32_t{
    ChangeUiState,
    CloseButtonClick,
    PlayButtonClick,
    MusikStreamStopp,
    MusikStreamError,
    MusikStreamPlay,
    AlbenClick,
    TitelClick,
    PlaylistClick,
    FilelistClick,
    CloseGuiElement,
    NewGeopos,
    BackendConnected,
    BackendDisconnected,
    MapMenuOpen,
    LongClick,
    Click,
    ClosePopup,
    OpenMapTextSearch,
    MediaFileUpdate,
    RemoteControl,
    PowerSupplyOff,
    VolumeUp,
    VolumeDown
};