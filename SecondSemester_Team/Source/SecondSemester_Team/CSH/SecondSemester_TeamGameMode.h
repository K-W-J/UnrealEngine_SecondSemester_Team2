// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SecondSemester_TeamGameMode.generated.h"

class UAudioComponent;
class USoundBase;

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class ASecondSemester_TeamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASecondSemester_TeamGameMode();

	/** Looping music used for the title screen and wave combat. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio|Music")
	TObjectPtr<USoundBase> BackgroundMusic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio|Music",
		meta=(ClampMin="0.0", ClampMax="1.0"))
	float BackgroundMusicVolume = 0.20f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BackgroundMusicComponent;
};



