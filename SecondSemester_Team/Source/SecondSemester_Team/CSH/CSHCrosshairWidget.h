#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHCrosshairWidget.generated.h"

UCLASS()
class SECONDSEMESTER_TEAM_API UCSHCrosshairWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};