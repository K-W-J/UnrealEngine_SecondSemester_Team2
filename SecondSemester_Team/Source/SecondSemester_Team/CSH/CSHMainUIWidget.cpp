#include "CSHMainUIWidget.h"
#include "CSHWeaponHUDWidget.h"
#include "CSHPlayerStatusWidget.h"
#include "Blueprint/UserWidget.h"
#include "UObject/SoftObjectPath.h"
#include "Widgets/SOverlay.h"

TSharedRef<SWidget> UCSHMainUIWidget::RebuildWidget()
{
    TSharedRef<SOverlay> RootOverlay = SNew(SOverlay);

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