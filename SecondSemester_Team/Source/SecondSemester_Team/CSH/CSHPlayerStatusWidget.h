#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHPlayerStatusWidget.generated.h"

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API UCSHPlayerStatusWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};