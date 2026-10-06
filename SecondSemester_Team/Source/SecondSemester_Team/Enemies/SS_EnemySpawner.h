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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Spawner")
	TSubclassOf<ASS_Enemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Spawner", meta = (ClampMin = "0.0", Units = "s"))
	float SpawnInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Spawner")
	bool bSpawnImmediately = true;

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
