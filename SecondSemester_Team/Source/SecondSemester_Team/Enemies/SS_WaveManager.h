#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemies/SS_WaveData.h"
#include "SS_WaveManager.generated.h"

class ASS_EnemySpawner;
class ASS_Enemy;
class USS_WaveWidget;
class UCSHEndingWidget;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ASS_WaveManager : public AActor
{
	GENERATED_BODY()

public:
	ASS_WaveManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<TObjectPtr<USS_WaveData>> WaveData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool bStartFirstWaveOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|UI", meta = (ClampMin = "0.0"))
	float WaveStartDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Reward", meta = (ClampMin = "0.0"))
	float WaveCompletionHealAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Boss UI",
		meta = (ClampMin = "0.02", Units = "s"))
	float BossHealthRefreshInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Ending", meta = (ClampMin = "1"))
	int32 FinalWaveNumber = 10;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	TArray<TObjectPtr<ASS_EnemySpawner>> EnemySpawners;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 SpawnedEnemyCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	bool bIsSpawningWave = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 AliveEnemyCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 TotalEnemyCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave")
	int32 DefeatedEnemyCount = 0;

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NotifyEnemyDied(AActor* Enemy);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void CollectEnemySpawners();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave(int32 WaveIndex);

	UFUNCTION(BlueprintCallable, Category = "Wave|UI")
	void SetWaveUIVisible(bool bVisible);

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
	void UpdateWaveProgressDisplay();
	void EnsureWaveWidget();
	void ShowEndingScreen();
	void UpdateBossHealthDisplay();
	void SpawnNextEnemy();
	FTimerHandle WaveSpawnTimer;
	FTimerHandle WaveStartTimer;
	FTimerHandle CountdownDisplayTimer;
	FTimerHandle NextWaveTimer;
	FTimerHandle BossHealthTimer;
	UPROPERTY(Transient)
	TObjectPtr<USS_WaveWidget> WaveWidget;
	UPROPERTY(Transient)
	TObjectPtr<UCSHEndingWidget> EndingWidget;
	TArray<TWeakObjectPtr<AActor>> LivingEnemies;
	TArray<TWeakObjectPtr<ASS_Enemy>> BossEnemies;
	bool bWaveActive = false;
	bool bWaveUIVisible = true;
	bool bIsWaveCountdownActive = false;
	TArray<FSS_WaveEnemyEntry> PendingEntries;
	int32 EntryIndex = 0;
	int32 EntrySpawnCount = 0;
};
