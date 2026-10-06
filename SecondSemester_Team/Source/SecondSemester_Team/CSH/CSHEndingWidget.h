#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHEndingWidget.generated.h"

/** Full-screen result menu shown after the final wave is cleared. */
UCLASS()
class SECONDSEMESTER_TEAM_API UCSHEndingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleQuitClicked();
};
