#include "CSHWeaponHUDWidget.h"
#include "CSHWeaponBase.h"
#include "SecondSemester_TeamCharacter.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

class SCSHAmmoRing final : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SCSHAmmoRing) : _Percent(1.0f) {} SLATE_ATTRIBUTE(float, Percent) SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Percent = Args._Percent; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(145.0f); }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out,
        int32 Layer, const FWidgetStyle&, bool) const override
    {
        const FVector2D Center = G.GetLocalSize() * .5f;
        const float Radius = FMath::Min(G.GetLocalSize().X, G.GetLocalSize().Y) * .43f;
        constexpr int32 Segments = 56;
        const int32 FilledCount = FMath::RoundToInt(FMath::Clamp(Percent.Get(0.f), 0.f, 1.f) * Segments);
        TArray<FVector2D> Back, Filled;
        for (int32 I = 0; I <= Segments; ++I)
        {
            const float A = FMath::DegreesToRadians(130.f + 280.f * I / Segments);
            const FVector2D P = Center + FVector2D(FMath::Cos(A), FMath::Sin(A)) * Radius;
            Back.Add(P); if (I <= FilledCount) Filled.Add(P);
        }
        FSlateDrawElement::MakeLines(Out, Layer, G.ToPaintGeometry(), Back, ESlateDrawEffect::None,
            FLinearColor(.1f, .17f, .21f, .95f), true, 9.f);
        if (Filled.Num() > 1) FSlateDrawElement::MakeLines(Out, Layer + 1, G.ToPaintGeometry(), Filled,
            ESlateDrawEffect::None, FLinearColor(0.f, .92f, 1.f, 1.f), true, 9.f);
        return Layer + 1;
    }
private: TAttribute<float> Percent;
};

TSharedRef<SWidget> UCSHWeaponHUDWidget::RebuildWidget()
{
    const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
    const FLinearColor Cyan(0.f, .92f, 1.f, 1.f), Muted(.58f, .7f, .75f, 1.f);
    auto GetWeapon = [this]() -> const ACSHWeaponBase* {
        const auto* C = Cast<ASecondSemester_TeamCharacter>(GetOwningPlayerPawn()); return C ? C->CurrentWeapon : nullptr; };
    return SNew(SBox).WidthOverride(500.f).HeightOverride(170.f)
    [
        SNew(SBorder).BorderImage(White).BorderBackgroundColor(FLinearColor(.012f, .02f, .027f, .88f)).Padding(FMargin(20.f, 14.f))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(FMargin(0.f,0.f,16.f,0.f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly).HAlign(HAlign_Left)
                        [ SAssignNew(WeaponNameText, STextBlock).Text(FText::FromString(TEXT("AK-47"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",30)).ColorAndOpacity(FLinearColor::White) ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(FMargin(1.f,6.f,0.f,0.f))
                    [
                        SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly).HAlign(HAlign_Left)
                        [ SAssignNew(WeaponDescriptionText, STextBlock).Text(FText::FromString(TEXT("ASSAULT RIFLE / AUTO"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",11)).ColorAndOpacity(Cyan) ]
                    ]
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(SBox).WidthOverride(145.f).HeightOverride(145.f)
                [
                    SNew(SOverlay)
                    + SOverlay::Slot()
                    [ SNew(SCSHAmmoRing).Percent_Lambda([GetWeapon]() { const auto* W=GetWeapon(); return (!W||W->HasInfiniteAmmo())?1.f:(W->GetMagazineCapacity()>0?(float)W->GetCurrentAmmo()/W->GetMagazineCapacity():0.f); }) ]
                    + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                        [ SNew(STextBlock).Text_Lambda([GetWeapon]() { const auto* W=GetWeapon(); return !W?FText::FromString(TEXT("0")):(W->HasInfiniteAmmo()?FText::FromString(FString::Chr(0x221E)):FText::AsNumber(W->GetCurrentAmmo())); }).Font(FCoreStyle::GetDefaultFontStyle("Bold",46)).ColorAndOpacity(FLinearColor::White) ]
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                        [ SNew(STextBlock).Text_Lambda([GetWeapon]() { const auto* W=GetWeapon(); return (!W||W->HasInfiniteAmmo())?FText::GetEmpty():FText::FromString(FString::Printf(TEXT("/ %d   %d"),W->GetMagazineCapacity(),W->GetReserveAmmo())); }).Font(FCoreStyle::GetDefaultFontStyle("Bold",13)).ColorAndOpacity(Muted) ]
                    ]
                    + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(FMargin(0,0,2,8))
                    [ SNew(STextBlock).Text(FText::FromString(TEXT("AMMO"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",10)).ColorAndOpacity(Cyan) ]
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
