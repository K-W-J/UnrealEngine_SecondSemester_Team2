// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SS_EnemySpawner.generated.h"

class ASS_Enemy;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ASS_EnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	ASS_EnemySpawner();

	/** Enemy class spawned at this actor's transform. Set this to BP_SS_Enemy to keep its car meshes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Spawner")
	TSubclassOf<ASS_Enemy> EnemyClass;

	/** Seconds between enemy spawns. Set to zero to disable repeated spawning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Spawner", meta = (ClampMin = "0.0", Units = "s"))
	float SpawnInterval = 5.0f;

	/** Spawns one enemy as soon as play begins, before the repeating timer starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Spawner")
	bool bSpawnImmediately = true;

	/** Spawns one enemy at the spawner's current location and rotation. */
	UFUNCTION(BlueprintCallable, Category = "SS Enemy Spawner")
	void SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "SS Enemy Spawner")
	ASS_Enemy* SpawnEnemyOfClass(TSubclassOf<ASS_Enemy> ClassToSpawn);

	UFUNCTION(BlueprintCallable, Category = "SS Enemy Spawner")
	void StopAutomaticSpawning();

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle SpawnTimerHandle;
};
