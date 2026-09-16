#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHDamageBorderWidget.generated.h"

class UBorder;

/** Full-screen red edge flash shown briefly when the player takes damage. */
UCLASS()
class SECONDSEMESTER_TEAM_API UCSHDamageBorderWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetFlashOpacity(float InOpacity);
	void SetFlashTint(const FLinearColor& InTint);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> EdgeBorders;

	float FlashOpacity = 0.0f;
	FLinearColor FlashTint = FLinearColor::Red;
	void RefreshColors();
};
