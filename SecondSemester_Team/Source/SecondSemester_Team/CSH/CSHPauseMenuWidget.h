#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHPauseMenuWidget.generated.h"

UCLASS()
class SECONDSEMESTER_TEAM_API UCSHPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void HandleContinueClicked();

	UFUNCTION()
	void HandleTitleClicked();

	UFUNCTION()
	void HandleQuitClicked();
};
