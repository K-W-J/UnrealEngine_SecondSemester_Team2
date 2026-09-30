#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHStartMenuWidget.generated.h"

/** Full-screen title menu displayed before gameplay begins. */
UCLASS()
class SECONDSEMESTER_TEAM_API UCSHStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleQuitClicked();
};
