#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemies/SS_WaveData.h"
#include "SS_WaveManager.generated.h"

class ASS_EnemySpawner;
class USS_WaveWidget;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ASS_WaveManager : public AActor
{
	GENERATED_BODY()

public:
	ASS_WaveManager();

	/** Array order is the wave order. StartWave uses a zero-based index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<TObjectPtr<USS_WaveData>> WaveData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool bStartFirstWaveOnBeginPlay = true;

	/** Countdown before every wave, including the first one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|UI", meta = (ClampMin = "0.0"))
	float WaveStartDelay = 3.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	TArray<TObjectPtr<ASS_EnemySpawner>> EnemySpawners;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex = INDEX_NONE;

	/** Counts successful spawns, not living enemies. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 SpawnedEnemyCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	bool bIsSpawningWave = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 AliveEnemyCount = 0;

	/** Call from death logic when an enemy leaves a corpse instead of being destroyed. */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NotifyEnemyDied(AActor* Enemy);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void CollectEnemySpawners();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave(int32 WaveIndex);

	/** Stops pending spawns; already spawned enemies are left alive. */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StopWave();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleEnemyDestroyed(AActor* Enemy);
	void CheckWaveCompleted();
	void StartNextWave();
	void BeginWaveSpawning();
	void UpdateCountdownDisplay();
	void EnsureWaveWidget();
	void SpawnNextEnemy();
	FTimerHandle WaveSpawnTimer;
	FTimerHandle WaveStartTimer;
	FTimerHandle CountdownDisplayTimer;
	FTimerHandle NextWaveTimer;
	UPROPERTY(Transient)
	TObjectPtr<USS_WaveWidget> WaveWidget;
	TArray<TWeakObjectPtr<AActor>> LivingEnemies;
	bool bWaveActive = false;
	bool bIsWaveCountdownActive = false;
	TArray<FSS_WaveEnemyEntry> PendingEntries;
	int32 EntryIndex = 0;
	int32 EntrySpawnCount = 0;
};
