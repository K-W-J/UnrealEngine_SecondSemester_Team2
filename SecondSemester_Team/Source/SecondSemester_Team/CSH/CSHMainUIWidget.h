#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHMainUIWidget.generated.h"

class UUserWidget;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API UCSHMainUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="CSH|UI")
    void SetWeaponUIVisible(bool bVisible);
    void SetWeaponInfo(const FText& Name, const FText& Description);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> WeaponUIInstance;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> PlayerStatusUIInstance;
};