#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "CSHWeaponHUDWidget.generated.h"

class STextBlock;

UCLASS()
class SECONDSEMESTER_TEAM_API UCSHWeaponHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetWeaponInfo(const FText& Name, const FText& Description);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    TSharedPtr<STextBlock> WeaponNameText;
    TSharedPtr<STextBlock> WeaponDescriptionText;
    FSlateBrush WeaponIconBrush;
};