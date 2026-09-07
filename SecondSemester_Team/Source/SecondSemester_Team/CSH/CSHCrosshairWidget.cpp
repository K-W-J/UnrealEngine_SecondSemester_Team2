#include "CSHCrosshairWidget.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

TSharedRef<SWidget> UCSHCrosshairWidget::RebuildWidget()
{
    const FLinearColor CrosshairColor(1.0f, 0.55f, 0.08f, 0.95f);
    const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

    return SNew(SBox)
        .WidthOverride(24.0f)
        .HeightOverride(24.0f)
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SBox)
                .WidthOverride(24.0f)
                .HeightOverride(2.0f)
                [
                    SNew(SBorder)
                    .BorderImage(WhiteBrush)
                    .BorderBackgroundColor(CrosshairColor)
                ]
            ]
            + SOverlay::Slot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SBox)
                .WidthOverride(2.0f)
                .HeightOverride(24.0f)
                [
                    SNew(SBorder)
                    .BorderImage(WhiteBrush)
                    .BorderBackgroundColor(CrosshairColor)
                ]
            ]
            + SOverlay::Slot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SBox)
                .WidthOverride(4.0f)
                .HeightOverride(4.0f)
                [
                    SNew(SBorder)
                    .BorderImage(WhiteBrush)
                    .BorderBackgroundColor(FLinearColor::White)
                ]
            ]
        ];
}