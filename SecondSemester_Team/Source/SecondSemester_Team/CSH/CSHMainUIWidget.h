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

    void SetPointCount(int32 NewPointCount);

    void SetSpecialWeaponHint(const FText& Hint, bool bVisible);

    void ShowTemporaryMessage(const FText& Message, float Duration = 3.0f);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> TemporaryMessageText;

    int32 CurrentPointCount = 0;
    float MessageTimeRemaining = 0.0f;
    float MessageDuration = 0.0f;
};
