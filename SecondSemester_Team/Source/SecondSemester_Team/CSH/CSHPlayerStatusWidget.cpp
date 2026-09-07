#include "CSHPlayerStatusWidget.h"
#include "SecondSemester_TeamCharacter.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UCSHPlayerStatusWidget::RebuildWidget()
{
    const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
    const FLinearColor PanelColor(0.015f, 0.025f, 0.045f, 0.86f);
    const FLinearColor HealthColor(0.92f, 0.12f, 0.12f, 1.0f);
    const FLinearColor StaminaColor(0.1f, 0.82f, 0.72f, 1.0f);
    const FLinearColor MutedColor(0.68f, 0.76f, 0.84f, 1.0f);

    auto GetCharacter = [this]() -> const ASecondSemester_TeamCharacter*
    {
        return Cast<ASecondSemester_TeamCharacter>(GetOwningPlayerPawn());
    };

    return SNew(SBox)
        .WidthOverride(330.0f)
        .HeightOverride(116.0f)
        [
            SNew(SBorder)
            .BorderImage(WhiteBrush)
            .BorderBackgroundColor(PanelColor)
            .Padding(FMargin(14.0f, 10.0f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.0f)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("HEALTH")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                        .ColorAndOpacity(FSlateColor(HealthColor))
                    ]
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SNew(STextBlock)
                        .Text_Lambda([GetCharacter]()
                        {
                            const auto* Character = GetCharacter();
                            return FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Character ? Character->GetHealth() : 0.0f, Character ? Character->GetMaxHealth() : 100.0f));
                        })
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                        .ColorAndOpacity(FSlateColor(HealthColor))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 4.0f, 0.0f, 8.0f))
                [
                    SNew(SBox).HeightOverride(20.0f)
                    [
                        SNew(SProgressBar)
                        .Percent_Lambda([GetCharacter]() -> TOptional<float>
                        {
                            const auto* Character = GetCharacter();
                            return Character && Character->GetMaxHealth() > 0.0f ? Character->GetHealth() / Character->GetMaxHealth() : 0.0f;
                        })
                        .FillColorAndOpacity(HealthColor)
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.0f)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("STAMINA")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                        .ColorAndOpacity(FSlateColor(MutedColor))
                    ]
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SNew(STextBlock)
                        .Text_Lambda([GetCharacter]()
                        {
                            const auto* Character = GetCharacter();
                            return FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Character ? Character->GetStamina() : 0.0f, Character ? Character->GetMaxStamina() : 100.0f));
                        })
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                        .ColorAndOpacity(FSlateColor(FLinearColor::White))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
                [
                    SNew(SBox).HeightOverride(15.0f)
                    [
                        SNew(SProgressBar)
                        .Percent_Lambda([GetCharacter]() -> TOptional<float>
                        {
                            const auto* Character = GetCharacter();
                            return Character && Character->GetMaxStamina() > 0.0f ? Character->GetStamina() / Character->GetMaxStamina() : 0.0f;
                        })
                        .FillColorAndOpacity(StaminaColor)
                    ]
                ]
            ]
        ];
}