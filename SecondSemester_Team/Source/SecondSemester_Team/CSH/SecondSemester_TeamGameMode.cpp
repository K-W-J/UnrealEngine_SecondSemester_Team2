// Copyright Epic Games, Inc. All Rights Reserved.

#include "SecondSemester_TeamGameMode.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ASecondSemester_TeamGameMode::ASecondSemester_TeamGameMode()
{
	static ConstructorHelpers::FObjectFinder<USoundBase> MusicAsset(
		TEXT("/Game/Audio/Music/MUS_EpicBossBattle.MUS_EpicBossBattle"));
	if (MusicAsset.Succeeded())
	{
		BackgroundMusic = MusicAsset.Object;
	}
}

void ASecondSemester_TeamGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (!BackgroundMusic || IsRunningDedicatedServer())
	{
		return;
	}

	BackgroundMusicComponent = UGameplayStatics::SpawnSound2D(
		this, BackgroundMusic, FMath::Clamp(BackgroundMusicVolume, 0.0f, 1.0f),
		1.0f, 0.0f, nullptr, false, false);
	if (BackgroundMusicComponent)
	{
		// The title screen pauses gameplay; music should continue as UI audio.
		BackgroundMusicComponent->bIsUISound = true;
	}
}

void ASecondSemester_TeamGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BackgroundMusicComponent)
	{
		BackgroundMusicComponent->Stop();
		BackgroundMusicComponent = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}
