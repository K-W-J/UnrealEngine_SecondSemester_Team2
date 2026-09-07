#include "CSHWeaponHUDWidget.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UCSHWeaponHUDWidget::RebuildWidget()
{
    const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
    const FLinearColor PanelColor(0.015f, 0.025f, 0.045f, 0.86f);
    const FLinearColor AccentColor(1.0f, 0.52f, 0.06f, 1.0f);
    const FLinearColor MutedColor(0.55f, 0.7f, 0.82f, 1.0f);

    return SNew(SBox)
        .WidthOverride(330.0f)
        .HeightOverride(116.0f)
        [
            SNew(SBorder)
            .BorderImage(WhiteBrush)
            .BorderBackgroundColor(PanelColor)
            .Padding(FMargin(5.0f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SBox)
                    .WidthOverride(6.0f)
                    [
                        SNew(SBorder)
                        .BorderImage(WhiteBrush)
                        .BorderBackgroundColor(AccentColor)
                    ]
                ]
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .Padding(FMargin(16.0f, 10.0f, 12.0f, 10.0f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SAssignNew(WeaponNameText, STextBlock)
                        .Text(FText::FromString(TEXT("AK-47")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 27))
                        .ColorAndOpacity(FSlateColor(FLinearColor::White))
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(1.0f, 2.0f, 0.0f, 0.0f))
                    [
                        SAssignNew(WeaponDescriptionText, STextBlock)
                        .Text(FText::FromString(TEXT("ASSAULT RIFLE / AUTO")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
                        .ColorAndOpacity(FSlateColor(MutedColor))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        SNew(SSpacer)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("PRIMARY WEAPON")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                        .ColorAndOpacity(FSlateColor(AccentColor))
                    ]
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(4.0f, 0.0f, 16.0f, 0.0f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(FString::Chr(0x221E)))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 50))
                        .ColorAndOpacity(FSlateColor(FLinearColor::White))
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("AMMO")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                        .ColorAndOpacity(FSlateColor(MutedColor))
                    ]
                ]
            ]
        ];
}
void UCSHWeaponHUDWidget::SetWeaponInfo(const FText& Name, const FText& Description)
{
    if (WeaponNameText) WeaponNameText->SetText(Name);
    if (WeaponDescriptionText) WeaponDescriptionText->SetText(Description);
}