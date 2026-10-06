#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHMainUIWidget.generated.h"

class UUserWidget;
class UTextBlock;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API UCSHMainUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="CSH|UI")
    void SetWeaponUIVisible(bool bVisible);
    void SetWeaponInfo(const FText& Name, const FText& Description);

    /** Updates the weapon-box point counter in the upper-left corner. */
    void SetPointCount(int32 NewPointCount);

    /** Shows a large control hint near the bottom centre of the screen. */
    void SetSpecialWeaponHint(const FText& Hint, bool bVisible);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> WeaponUIInstance;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> PlayerStatusUIInstance;
    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> RadarUIInstance;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PointText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SpecialWeaponHintText;

    int32 CurrentPointCount = 0;
};
