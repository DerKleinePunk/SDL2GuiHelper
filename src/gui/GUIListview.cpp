#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "GUIListview"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "../exception/GUIException.h"
#include "GUI.h"
#include "GUIElement.h"
#include "GUIFontManager.h"
#include "GUIListview.h"
#include "GUIRenderer.h"
#include "GUITexture.h"

#define ENTRYHEIGHT_PIXEL 42 // 37
#define FONT_HEIGHT 20
#define SMALLFONT_HEIGHT 14
#define SCROLL_BAR_WIDTH_PIXEL 20

GUIListviewColumn::GUIListviewColumn() : GUIListviewColumn("Kein Text")
{
}

GUIListviewColumn::GUIListviewColumn(const std::string& text) : text_(text), imageData_(nullptr)
{
    textureText_ = nullptr;
    textureTextDetails_ = nullptr;
    width_ = 0;
    height_ = 0;
    detailheight_ = 0;
    detailwidth_ = 0;
    imageSize_ = 0;
}

GUIListviewColumn::~GUIListviewColumn()
{
    if(textureText_ != nullptr) {
        delete textureText_;
        textureText_ = nullptr;
    }
    if(textureTextDetails_ != nullptr) {
        delete textureTextDetails_;
        textureTextDetails_ = nullptr;
    }
}

std::string GUIListviewColumn::GetText() const
{
    return text_;
}

GUITexture* GUIListviewColumn::GetTexture(GUIRenderer* renderer,
                                          TTF_Font*& font,
                                          TTF_Font*& smallFont,
                                          SDL_Color color,
                                          GUITexture** textureTextDetails,
                                          const int maxWith)
{
    if(textureTextDetails_ != nullptr) *textureTextDetails = textureTextDetails_;

    if(textureText_ != nullptr) return textureText_;

    textureText_ = renderer->RenderTextBlended(font, GetText().c_str(), color);

    if(detailText_.size() > 0) {
        textureTextDetails_ =
        renderer->RenderTextBlendedWrapped(smallFont, detailText_.c_str(), color, maxWith);
        *textureTextDetails = textureTextDetails_;
    }


    return textureText_;
}

int GUIListviewColumn::GetWidth() const
{
    return width_;
}

int GUIListviewColumn::GetHeight() const
{
    return height_;
}

void GUIListviewColumn::SetDetailText(const std::string& text)
{
    detailText_ = text;
    if(textureText_ == nullptr) return;

    delete textureText_;
    if(textureTextDetails_ != nullptr) delete textureTextDetails_;
    textureText_ = nullptr;
    textureTextDetails_ = nullptr;
}

int GUIListviewColumn::GetDetailHeight() const
{
    return detailheight_;
}

int GUIListviewColumn::GetDetailWidth() const
{
    return detailwidth_;
}

int GUIListviewColumn::GetImageSize() const
{
    return imageSize_;
}

char* GUIListviewColumn::GetImageData() const
{
    return imageData_.get();
}

void GUIListviewColumn::SetImageData(const char* imageData, const int size)
{
    imageData_.reset(new char[size], std::default_delete<char[]>());
    memcpy(imageData_.get(), imageData, size);
    imageSize_ = size;
}

GUIListviewRow::GUIListviewRow() : selected(false), Tag(nullptr)
{
}

std::vector<std::shared_ptr<GUIListviewColumn>> GUIListviewRow::GetColumns() const
{
    return columns_;
}

void GUIListviewRow::AddColumn(const std::shared_ptr<GUIListviewColumn> column)
{
    columns_.push_back(column);
}

void GUIListview::DrawData()
{
    if(rows_.size() == 0) return;

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerBlkObjDrawData, "DrawListViewData", VLOG_IS_ON(4));
#endif

    // Big Problem Texture size is Limit to 16000 Pixel Long list can't rendert fullsize On Pi Only 2048
    const int height = rows_.size() * ENTRYHEIGHT_PIXEL + 1;

    if(textureData_ == nullptr) {
        textureData_ = renderer_->CreateTexture(Size());
    }

    renderer_->RenderTarget(textureData_);
    renderer_->Clear(lightgray_t_color);

    if(height > Size().height) {
        scrollEnabled_ = true;
    } else {
        scrollEnabled_ = false;
    }
    auto actHight = -movepixel_;
    // Ho can me explan why for(row : rows_) take 40ms and this 1 ms ?
    // http://cpc110.blogspot.de/2017/02/c-iterator-much-slower-than-indexing.html
    for(size_t i = 0; i < rows_.size(); i++) {
        if(actHight + ENTRYHEIGHT_PIXEL > 0 && actHight < Size().height) {
            auto rect = GUIRect(0, actHight, Size().width, ENTRYHEIGHT_PIXEL);
            if(scrollEnabled_) {
                rect.w = rect.w - SCROLL_BAR_WIDTH_PIXEL;
            }
            if(rows_[i].selected) {
                renderer_->DrawRect(rect, own_blue_color);
            }
            rect.h -= 4;
            rect.w -= 4;
            rect.x += 2;
            rect.y += 2;
            renderer_->SetClipRect(rect);
            for(auto& column : rows_[i].GetColumns()) {
                GUIRect dstrect;
                if(_rowHasImage) {
                    dstrect.x = 39;
                    if(column->GetImageSize() > 0) {
                        const auto imageRaw = SDL_RWFromMem(column->GetImageData(), column->GetImageSize());
                        const auto image = renderer_->LoadTextureImageData(imageRaw);
                        if(image == nullptr) {
                            LOG(WARNING) << "Image not get";
                        } else {
                            GUIPoint imagePoint;
                            imagePoint.x = 2;
                            imagePoint.y = actHight + 2;
                            renderer_->RenderCopy(image, imagePoint);
                            delete image;
                        }
                    }
                } else {
                    dstrect.x = 2;
                }
                GUITexture* details = nullptr;
                const auto textureText =
                column->GetTexture(renderer_, font_, smallFont_, foregroundColor_, &details, Size().width);
                if(textureText == nullptr) {
                    throw new GUIException("column GetTexture nullptr");
                }
                dstrect.y = actHight + 1;
                renderer_->RenderCopy(textureText, GUIPoint(dstrect.x, dstrect.y));
                if(details != nullptr) {
                    dstrect.y = actHight + 1 + FONT_HEIGHT;
                    renderer_->RenderCopy(details, GUIPoint(dstrect.x, dstrect.y));
                }
            }
            renderer_->ClearClipRect();
        }

        if(_rowHasDetails || _rowHasImage) {
            actHight += ENTRYHEIGHT_PIXEL;
        } else {
            actHight += 1 + FONT_HEIGHT;
        }
    }

    if(scrollEnabled_) {
        if(textureQuickJumpList_ == nullptr) {
            // Draw an QuikJumpbar
            textureQuickJumpList_ = renderer_->CreateTexture(GUISize(SCROLL_BAR_WIDTH_PIXEL, Size().height));
            renderer_->RenderTarget(textureQuickJumpList_);
            renderer_->Clear(lightgray_t_color);
            const auto rect = GUIRect(0, 0, SCROLL_BAR_WIDTH_PIXEL, Size().height);
            renderer_->DrawRect(rect, own_blue_color);

            auto textPos = 0;
            for(const auto quickJump : quickJumpList_) {
                const auto textureText = renderer_->RenderTextBlended(font_, quickJump, own_blue_color);
                const auto point = GUIPoint(2, textPos);
                renderer_->RenderCopy(textureText, point);
                textPos += SMALLFONT_HEIGHT;
                delete textureText;
            }
            renderer_->RenderTarget(textureData_);
        }
        renderer_->RenderCopy(textureQuickJumpList_, GUIPoint(Size().width - SCROLL_BAR_WIDTH_PIXEL, 0));
    }
    renderer_->RenderTarget(nullptr);
}

bool GUIListview::PointOnScrollBar(const GUIPoint& point) const
{
    if(!scrollEnabled_) return false;
    if(point.x > Size().width - SCROLL_BAR_WIDTH_PIXEL && point.x < Size().width) {
        return true;
    }
    return false;
}

void GUIListview::ButtonDown(const Uint8 button, Uint8 clicks, const GUIPoint& point)
{
    UNUSED(clicks);

    if(button == SDL_BUTTON_LEFT) {
        lastLeftButtonDow_ = SDL_GetTicks();
    }

    const auto pointInElement = ScreenToElementCoords(point);
    downOnY_ = pointInElement.y;

    if(PointOnScrollBar(pointInElement)) {
        scrolling_ = true;
    } else {
        downOnRow_ = GetRowAtPoint(pointInElement);
    }
}

void GUIListview::UpdateScrollSize(const GUIPoint& pointInElement)
{
    const auto yDelta = downOnY_ - pointInElement.y;
    VLOG(3) << "yDelta " << yDelta;
    const int height = rows_.size() * ENTRYHEIGHT_PIXEL + 1;
    movepixel_ += yDelta;
    if(movepixel_ < 0) movepixel_ = 0;
    if(height - Size().height < movepixel_) movepixel_ = height - Size().height;

    VLOG(3) << "movepixel " << movepixel_;
}

int GUIListview::GetRowAtPoint(const GUIPoint& pointInElement) const
{
    if(pointInElement.x < 0 || pointInElement.y < 0) {
        return -1;
    }
    if(rows_.size() == 0) return -1;
    const size_t rowId = (pointInElement.y + movepixel_) / ENTRYHEIGHT_PIXEL;
    return rowId;
}

void GUIListview::CheckRowClick(const int rowId)
{
    auto now = SDL_GetTicks();
    const auto test = -movepixel_ + (rowId * ENTRYHEIGHT_PIXEL);
    if(test < 0) {
        // Test row on Top is visible
        movepixel_ = rowId * ENTRYHEIGHT_PIXEL;
    }
    if((test + ENTRYHEIGHT_PIXEL) > Size().height) {
        // Test row on bottown is visible
        movepixel_ = movepixel_ + ((test + ENTRYHEIGHT_PIXEL) - Size().height) + 1;
    }
    for(auto i = 0; i < static_cast<int>(rows_.size()); i++) {
        if(rowId == i) {
            rows_[i].selected = true;
        } else {
            rows_[i].selected = false;
        }
    }
    selectedRow_ = rowId;
    if(!inUpdate_) {
        DrawData();
        SetRedraw();
    }

    if(SDL_TICKS_PASSED(now, lastLeftButtonDow_ + long_click_time)) {
        if(OnLongClick_ && selectedRow_ != -1) {
            OnLongClick_(this, selectedRow_, rows_[selectedRow_].Tag);
        }
    } else {
        if(OnClick_) {
            OnClick_(this, rowId, rows_[rowId].Tag);
        }
    }
}

void GUIListview::CheckScrolling(const GUIPoint& point)
{
    const auto pointInElement = ScreenToElementCoords(point);
    const auto entryNumber = pointInElement.y / SMALLFONT_HEIGHT;
    VLOG(3) << "entryNumber " << entryNumber << " " << quickJumpList_[entryNumber];

    for(size_t i = 0; i < rows_.size(); i++) {
        auto text = rows_[i].GetColumns()[0]->GetText();
        if(text.substr(0, 1) == quickJumpList_[entryNumber]) {
            movepixel_ = i * ENTRYHEIGHT_PIXEL;
            break;
        }
    }
    scrolling_ = false;
    if(!inUpdate_) {
        DrawData();
        SetRedraw();
    }
}

void GUIListview::ButtonUp(const Uint8 button, Uint8 clicks, const GUIPoint& point)
{
    UNUSED(clicks);

    if(button == SDL_BUTTON_LEFT) {
        const auto pointInElement = ScreenToElementCoords(point);
        const auto rowNow = GetRowAtPoint(pointInElement);
        if(rowNow < 0) return;

        if(lastLeftButtonDow_ != 0) {
            if(scrolling_) {
                CheckScrolling(point);
            } else {
                if(downOnRow_ == rowNow) {
                    CheckRowClick(rowNow);
                } else {
                    UpdateScrollSize(pointInElement);
                    if(!inUpdate_) {
                        DrawData();
                        SetRedraw();
                    }
                }
            }
            lastLeftButtonDow_ = 0;
        }
    }
}

void GUIListview::MouseMoveEvent(const Uint8 button, const GUIPoint& point)
{
    const auto pointInElement = ScreenToElementCoords(point);
    if(std::abs(downOnY_ - pointInElement.y) <= 6) return;

    const auto rowNow = GetRowAtPoint(pointInElement);
    if(downOnRow_ != rowNow && button == SDL_BUTTON_LEFT && rows_.size() != 0) {
        downOnRow_ = -1;
        auto save = movepixel_;
        UpdateScrollSize(pointInElement);
        downOnY_ = pointInElement.y;
        if(!inUpdate_ && save != movepixel_) {
            DrawData();
            SetRedraw();
        }
    }
}

void GUIListview::MouseLeaveEvent(const GUIPoint& point)
{
    auto pointInElement = ScreenToElementCoords(point);
    if(scrolling_ && rows_.size() != 0) {
        VLOG(3) << ToString() << " Mouse leave at " + pointInElement.ToString();
        UpdateScrollSize(pointInElement);
        downOnY_ = pointInElement.y;
        if(!inUpdate_) {
            DrawData();
            SetRedraw();
        }
        scrolling_ = false;
    }
}

GUIListview::GUIListview(const GUIPoint position,
                         const GUISize size,
                         const std::string& name,
                         const SDL_Color background,
                         const SDL_Color textcolor)
    : GUIElement(position, size, name), font_(nullptr), textureQuickJumpList_(nullptr),
      textureNoDataText_(nullptr), textureData_(nullptr)
{
    logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = background;
    foregroundColor_ = textcolor;
    inUpdate_ = false;

    buttonDownEvent_ = std::bind(&GUIListview::ButtonDown, this, std::placeholders::_1,
                                 std::placeholders::_2, std::placeholders::_3);
    buttonUpEvent_ = std::bind(&GUIListview::ButtonUp, this, std::placeholders::_1,
                               std::placeholders::_2, std::placeholders::_3);
    mouseMoveEvent_ =
    std::bind(&GUIListview::MouseMoveEvent, this, std::placeholders::_1, std::placeholders::_2);
    mouseLeaveEvent_ = std::bind(&GUIListview::MouseLeaveEvent, this, std::placeholders::_1);
    movepixel_ = 0;
    selectedRow_ = -1;
    scrolling_ = false;
    downOnY_ = 0;
    _rowHasImage = true;
    _rowHasDetails = true;
}

void GUIListview::Init()
{
    // Things after Control is Created

    font_ = fontManager_->GetDefaultSmallFont(FONT_HEIGHT);
    smallFont_ = fontManager_->GetDefaultSmallFont(SMALLFONT_HEIGHT);
    textureNoDataText_ = renderer_->RenderTextBlended(font_, "no data", foregroundColor_);
    drawNoDataTextPosition_ = GUIPoint(0, 0);
    drawNoDataTextPosition_.x = (Size().width - textureNoDataText_->Size().width) / 2;
}

void GUIListview::Draw()
{
    if(rows_.size() == 0 || textureData_ == nullptr) {
        renderer_->RenderCopy(textureNoDataText_, drawNoDataTextPosition_);
        needRedraw_ = false;
        return;
    }
    renderer_->RenderCopy(textureData_, GUIPoint(0, 0));
    needRedraw_ = false;
}

void GUIListview::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUIListview::UpdateAnimation()
{
}

void GUIListview::Close()
{
    if(textureNoDataText_ != nullptr) {
        delete textureNoDataText_;
        textureNoDataText_ = nullptr;
    }

    if(textureData_ != nullptr) {
        delete textureData_;
        textureData_ = nullptr;
    }
    if(textureQuickJumpList_ != nullptr) {
        delete textureQuickJumpList_;
        textureQuickJumpList_ = nullptr;
    }
    rows_.clear();
}

void GUIListview::BeginUpdate()
{
    inUpdate_ = true;
}

void GUIListview::EndUpdate()
{
    for(size_t i = 0; i < rows_.size(); i++) {
        auto text = rows_[i].GetColumns()[0]->GetText();
        const auto firstChar = text.substr(0, 1);
        // TODO: Is skiping empty string a good idea? It at least prevent crash if the element is not displayed right away, but still crash when scrolling over it.
        // TODO: I think it should be handled somewhere else, handling it there isn't sufficient. The drawing function might be the key.
        if(firstChar != "" && std::find(quickJumpList_.begin(), quickJumpList_.end(), firstChar) ==
                              quickJumpList_.end()) {
            quickJumpList_.push_back(firstChar);
        }
    }
    DrawData();
    inUpdate_ = false;
    SetRedraw();
}

void GUIListview::AddRow(const GUIListviewRow& row)
{
    rows_.push_back(row);

    if(!inUpdate_) {
        DrawData();
        SetRedraw();
    }
}

void GUIListview::Clear()
{
    if(textureData_ != nullptr) {
        delete textureData_;
        textureData_ = nullptr;
    }
    if(textureQuickJumpList_ != nullptr) {
        delete textureQuickJumpList_;
        textureQuickJumpList_ = nullptr;
    }
    rows_.clear();

    if(!inUpdate_) {
        DrawData();
        SetRedraw();
    }
}

void GUIListview::RegisterOnClick(const ClickListViewDelegate onClick)
{
    OnClick_ = onClick;
}

void GUIListview::RegisterOnLongClick(const ClickListViewDelegate onClick)
{
    OnLongClick_ = onClick;
}

void GUIListview::ChangeRowHasImage(bool on)
{
    _rowHasImage = on;
    SetRedraw();
}

void GUIListview::ChangeRowHasDetails(bool on)
{
    _rowHasDetails = on;
    SetRedraw();
}