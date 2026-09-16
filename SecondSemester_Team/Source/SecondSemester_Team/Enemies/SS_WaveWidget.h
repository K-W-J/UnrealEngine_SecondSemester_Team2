#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SS_WaveWidget.generated.h"

class UTextBlock;

/** One top-centred text label shared by the wave countdown and wave number. */
UCLASS()
class SECONDSEMESTER_TEAM_API USS_WaveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetWaveLabel(const FString& Label);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WaveText;

	FString CurrentLabel;
};
