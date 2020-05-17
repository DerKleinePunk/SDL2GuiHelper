#pragma once
#include <SDL_ttf.h>

struct GUIEvent;
class GUITexture;

typedef std::function<void(IGUIElement* sender, int row, void* tag)> ClickListViewDelegate;

class GUIListviewColumn {
	std::string text_;
	GUITexture* textureText_;
	GUITexture* textureTextDetails_;
	int width_;
	int height_;
	int detailheight_;
	int detailwidth_;
	std::string detailText_;
	int imageSize_;
	std::shared_ptr<char> imageData_;
public:
	GUIListviewColumn();
	explicit GUIListviewColumn(const std::string& text);
	~GUIListviewColumn();
	std::string GetText() const;
	GUITexture* GetTexture(GUIRenderer* renderer, TTF_Font*& font, TTF_Font*& smallFont, SDL_Color color, GUITexture** textureTextDetails, int maxWith);
	int GetWidth() const;
	int GetHeight() const;
	void SetDetailText(const std::string& text);
	int GetDetailHeight() const;
	int GetDetailWidth() const;
	int GetImageSize() const;
	char* GetImageData() const;
	void SetImageData(const char* imageData, int size);
};

class GUIListviewRow {
	std::vector<std::shared_ptr<GUIListviewColumn>> columns_;
public:
	bool selected;
	void* Tag;
	GUIListviewRow();
	std::vector<std::shared_ptr<GUIListviewColumn>> GetColumns() const;
	void AddColumn(std::shared_ptr<GUIListviewColumn> column);
	
};

class GUIListview : public GUIElement {
	el::Logger* logger_;
	TTF_Font* font_;
	TTF_Font* smallFont_;
	std::vector<GUIListviewRow> rows_;
	std::vector<std::string> quickJumpList_;
    GUITexture* textureQuickJumpList_;
	GUITexture* textureNoDataText_;
	GUITexture* textureData_;
	GUIPoint drawNoDataTextPosition_;
	Uint32 lastLeftButtonDow_;
	bool inUpdate_;
	bool scrollEnabled_;
	bool scrolling_;
	int downOnY_;
	int movepixel_;
	ClickListViewDelegate OnClick_;
	ClickListViewDelegate OnLongClick_;
	int selectedRow_;
	int quickJumpEntryHeight_;
	int downOnRow_;
	bool _rowHasImage;
	bool _rowHasDetails;
	RTTI_DERIVED(GUIListview);

	void DrawData();
	bool PointOnScrollBar(const GUIPoint& gui_point) const;
	void ButtonDown(Uint8 button, Uint8 clicks, const GUIPoint& point);
	void UpdateScrollSize(const GUIPoint& pointInElement);
	int GetRowAtPoint(const GUIPoint& pointInElement) const;
	void CheckRowClick(const int rowId);
	void CheckScrolling(const GUIPoint& point);
	void ButtonUp(Uint8 button, Uint8 clicks, const GUIPoint& point);
	void MouseMoveEvent(Uint8 button, const GUIPoint& point);
	void MouseLeaveEvent(const GUIPoint& point);
public:
	GUIListview(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor);

	void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;

	void BeginUpdate();
	void EndUpdate();
	void AddRow(const GUIListviewRow& row);
    void Clear();

	void RegisterOnClick(ClickListViewDelegate OnClick);
	void RegisterOnLongClick(ClickListViewDelegate OnClick);
	void ChangeRowHasImage(bool on);
	void ChangeRowHasDetails(bool on);
};

