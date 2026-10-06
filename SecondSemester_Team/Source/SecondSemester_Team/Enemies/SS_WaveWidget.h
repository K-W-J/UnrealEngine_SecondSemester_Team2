#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SS_WaveWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UVerticalBox;

/** One top-centred text label shared by the wave countdown and wave number. */
UCLASS()
class SECONDSEMESTER_TEAM_API USS_WaveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetWaveLabel(const FString& Label);
	void SetWaveProgress(float Progress);
	void SetBossHealthValues(const TArray<float>& HealthValues);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> WaveProgressBar;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> BossBarsBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UProgressBar>> BossHealthBars;

	FString CurrentLabel;
	float CurrentWaveProgress = 0.0f;
	TArray<float> CurrentBossHealthValues;

	void RebuildBossBars();
};
