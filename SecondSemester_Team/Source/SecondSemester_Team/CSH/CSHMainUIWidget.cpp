#include "CSHMainUIWidget.h"
#include "CSHWeaponHUDWidget.h"
#include "CSHPlayerStatusWidget.h"
#include "CSHRadarWidget.h"
#include "SecondSemester_TeamCharacter.h"
#include "CSHWeaponBase.h"
#include "CSHTowSword.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UObject/SoftObjectPath.h"
#include "Widgets/SOverlay.h"

TSharedRef<SWidget> UCSHMainUIWidget::RebuildWidget()
{
    TSharedRef<SOverlay> RootOverlay = SNew(SOverlay);

    PointText = NewObject<UTextBlock>(this, TEXT("PointText"));
    PointText->SetJustification(ETextJustify::Left);
    PointText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.82f, 0.18f, 1.0f)));
    PointText->SetShadowOffset(FVector2D(1.0f, 2.0f));
    PointText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
    FSlateFontInfo PointFont = PointText->GetFont();
    PointFont.Size = 28;
    PointText->SetFont(PointFont);
    SetPointCount(CurrentPointCount);
    RootOverlay->AddSlot()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .Padding(FMargin(34.0f, 28.0f, 0.0f, 0.0f))
        [
            PointText->TakeWidget()
        ];

    SpecialWeaponHintText = NewObject<UTextBlock>(this, TEXT("SpecialWeaponHintText"));
    SpecialWeaponHintText->SetJustification(ETextJustify::Center);
    SpecialWeaponHintText->SetColorAndOpacity(
        FSlateColor(FLinearColor(1.0f, 0.78f, 0.12f, 1.0f)));
    SpecialWeaponHintText->SetShadowOffset(FVector2D(2.0f, 3.0f));
    SpecialWeaponHintText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
    FSlateFontInfo HintFont = SpecialWeaponHintText->GetFont();
    HintFont.Size = 36;
    SpecialWeaponHintText->SetFont(HintFont);
    SpecialWeaponHintText->SetVisibility(ESlateVisibility::Collapsed);
    RootOverlay->AddSlot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Bottom)
        .Padding(FMargin(0.0f, 0.0f, 0.0f, 88.0f))
        [
            SpecialWeaponHintText->TakeWidget()
        ];

    const TSoftClassPtr<UUserWidget> WeaponUIClass(
        FSoftObjectPath(TEXT("/Game/CSH/Buleprint/UI/WBP_CSH_WeaponUI.WBP_CSH_WeaponUI_C")));

    if (UClass* LoadedClass = WeaponUIClass.LoadSynchronous())
    {
        WeaponUIInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), LoadedClass);
        if (WeaponUIInstance)
        {
            WeaponUIInstance->SetVisibility(ESlateVisibility::Collapsed);

            RootOverlay->AddSlot()
                .HAlign(HAlign_Right)
                .VAlign(VAlign_Bottom)
                .Padding(FMargin(0.0f, 0.0f, 34.0f, 34.0f))
                [
                    WeaponUIInstance->TakeWidget()
                ];
        }
    }

    const TSoftClassPtr<UUserWidget> StatusUIClass(
        FSoftObjectPath(TEXT("/Game/CSH/Buleprint/UI/WBP_CSH_PlayerStatusUI.WBP_CSH_PlayerStatusUI_C")));
    UClass* LoadedStatusClass = StatusUIClass.LoadSynchronous();
    if (!LoadedStatusClass)
    {
        LoadedStatusClass = UCSHPlayerStatusWidget::StaticClass();
    }
    PlayerStatusUIInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), LoadedStatusClass);
    if (PlayerStatusUIInstance)
    {
        PlayerStatusUIInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
        RootOverlay->AddSlot()
            .HAlign(HAlign_Left)
            .VAlign(VAlign_Bottom)
            .Padding(FMargin(34.0f, 0.0f, 0.0f, 34.0f))
            [
                PlayerStatusUIInstance->TakeWidget()
            ];
    }

    UClass* RadarClass = LoadClass<UCSHRadarWidget>(nullptr, TEXT("/Game/CSH/Buleprint/UI/WBP_CSH_Radar.WBP_CSH_Radar_C"));
    RadarUIInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), RadarClass ? RadarClass : UCSHRadarWidget::StaticClass());
    if (RadarUIInstance)
    {
        RadarUIInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
        RootOverlay->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top)
            .Padding(FMargin(0.f,34.f,34.f,0.f))[RadarUIInstance->TakeWidget()];
    }
    if (const auto* Player=Cast<ASecondSemester_TeamCharacter>(GetOwningPlayerPawn()))
    {
        if (IsValid(Player->CurrentWeapon))
        {
            SetWeaponInfo(Player->CurrentWeapon->GetWeaponDisplayName(),Player->CurrentWeapon->GetWeaponDescription());
            SetWeaponUIVisible(true);
            SetSpecialWeaponHint(
                FText::FromString(TEXT("R 키를 누르면 꽂은 자동차가 날아갑니다")),
                Player->CurrentWeapon->IsA<ACSHTowSword>());
        }
    }
    return RootOverlay;
}
void UCSHMainUIWidget::SetWeaponUIVisible(bool bVisible)
{
    if (WeaponUIInstance)
    {
        WeaponUIInstance->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
}
void UCSHMainUIWidget::SetWeaponInfo(const FText& Name, const FText& Description)
{
    if (UCSHWeaponHUDWidget* WeaponHUD = Cast<UCSHWeaponHUDWidget>(WeaponUIInstance))
    {
        WeaponHUD->SetWeaponInfo(Name, Description);
    }
}

void UCSHMainUIWidget::SetPointCount(int32 NewPointCount)
{
    CurrentPointCount = FMath::Max(0, NewPointCount);
    if (PointText)
    {
        PointText->SetText(FText::FromString(
            FString::Printf(TEXT("POINT  %d"), CurrentPointCount)));
    }
}

void UCSHMainUIWidget::SetSpecialWeaponHint(const FText& Hint, bool bVisible)
{
    if (!SpecialWeaponHintText)
    {
        return;
    }
    SpecialWeaponHintText->SetText(Hint);
    SpecialWeaponHintText->SetVisibility(
        bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
