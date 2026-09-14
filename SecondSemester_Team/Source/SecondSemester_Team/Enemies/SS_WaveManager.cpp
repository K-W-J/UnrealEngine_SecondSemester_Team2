#include "Enemies/SS_WaveManager.h"

#include "Enemies/SS_Enemy.h"
#include "Enemies/SS_EnemySpawner.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ASS_WaveManager::ASS_WaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASS_WaveManager::BeginPlay()
{
	Super::BeginPlay();
	CollectEnemySpawners();
	if (bStartFirstWaveOnBeginPlay && !WaveData.IsEmpty())
	{
		StartWave(0);
	}
}

void ASS_WaveManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopWave();
	Super::EndPlay(EndPlayReason);
}

void ASS_WaveManager::CollectEnemySpawners()
{
	EnemySpawners.Reset();
	if (!GetWorld())
	{
		return;
	}
	for (TActorIterator<ASS_EnemySpawner> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It))
		{
			(*It)->StopAutomaticSpawning();
			EnemySpawners.Add(*It);
		}
	}
}

void ASS_WaveManager::StartWave(int32 WaveIndex)
{
	if (!WaveData.IsValidIndex(WaveIndex) || !IsValid(WaveData[WaveIndex]))
	{
		return;
	}
	StopWave();
	CollectEnemySpawners();
	if (EnemySpawners.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("SS_WaveManager: No enemy spawners in the level."));
		return;
	}
	CurrentWaveIndex = WaveIndex;
	SpawnedEnemyCount = 0;
	EntryIndex = 0;
	EntrySpawnCount = 0;
	bWaveActive = true;
	for (const FSS_WaveEnemyEntry& Entry : WaveData[WaveIndex]->Enemies)
	{
		if (Entry.EnemyClass && Entry.Count > 0)
		{
			PendingEntries.Add(Entry);
		}
	}
	if (PendingEntries.IsEmpty())
	{
		CheckWaveCompleted();
		return;
	}
	bIsSpawningWave = true;
	GetWorldTimerManager().SetTimer(WaveSpawnTimer, this, &ASS_WaveManager::SpawnNextEnemy,
		FMath::Max(WaveData[WaveIndex]->SpawnInterval, 0.05f), true);
	SpawnNextEnemy();
}

void ASS_WaveManager::StopWave()
{
	GetWorldTimerManager().ClearTimer(WaveSpawnTimer);
	GetWorldTimerManager().ClearTimer(NextWaveTimer);
	bWaveActive = false;
	for (const TWeakObjectPtr<AActor>& Enemy : LivingEnemies)
	{
		if (Enemy.IsValid())
		{
			Enemy->OnDestroyed.RemoveDynamic(this, &ASS_WaveManager::HandleEnemyDestroyed);
		}
	}
	LivingEnemies.Reset();
	AliveEnemyCount = 0;
	bIsSpawningWave = false;
	PendingEntries.Reset();
}

void ASS_WaveManager::SpawnNextEnemy()
{
	EnemySpawners.RemoveAll([](const TObjectPtr<ASS_EnemySpawner>& Spawner)
	{
		return !IsValid(Spawner);
	});
	if (!bIsSpawningWave || !PendingEntries.IsValidIndex(EntryIndex) || EnemySpawners.IsEmpty())
	{
		StopWave();
		return;
	}
	const int32 RandomSpawnerIndex = FMath::RandRange(0, EnemySpawners.Num() - 1);
	ASS_EnemySpawner* Spawner = EnemySpawners[RandomSpawnerIndex];
	const FSS_WaveEnemyEntry& Entry = PendingEntries[EntryIndex];
	if (ASS_Enemy* Enemy = Spawner->SpawnEnemyOfClass(Entry.EnemyClass))
	{
		++SpawnedEnemyCount;
		if (IsValid(Enemy) && !Enemy->IsActorBeingDestroyed())
		{
			LivingEnemies.Add(Enemy);
			AliveEnemyCount = LivingEnemies.Num();
			Enemy->OnDestroyed.AddUniqueDynamic(this, &ASS_WaveManager::HandleEnemyDestroyed);
		}
		if (++EntrySpawnCount >= Entry.Count)
		{
			EntrySpawnCount = 0;
			++EntryIndex;
			if (!PendingEntries.IsValidIndex(EntryIndex))
			{
				GetWorldTimerManager().ClearTimer(WaveSpawnTimer);
				bIsSpawningWave = false;
				PendingEntries.Reset();
				CheckWaveCompleted();
			}
		}
	}
}

void ASS_WaveManager::HandleEnemyDestroyed(AActor* Enemy)
{
	NotifyEnemyDied(Enemy);
}

void ASS_WaveManager::NotifyEnemyDied(AActor* Enemy)
{
	if (!Enemy || LivingEnemies.Remove(TWeakObjectPtr<AActor>(Enemy)) == 0)
	{
		return;
	}
	Enemy->OnDestroyed.RemoveDynamic(this, &ASS_WaveManager::HandleEnemyDestroyed);
	AliveEnemyCount = LivingEnemies.Num();
	CheckWaveCompleted();
}

void ASS_WaveManager::CheckWaveCompleted()
{
	if (!bWaveActive || bIsSpawningWave || AliveEnemyCount > 0)
	{
		return;
	}
	bWaveActive = false;
	// Defer advancement to avoid recursive StartWave calls for empty waves.
	if (WaveData.IsValidIndex(CurrentWaveIndex + 1))
	{
		NextWaveTimer = GetWorldTimerManager().SetTimerForNextTick(
			this, &ASS_WaveManager::StartNextWave);
	}
}

void ASS_WaveManager::StartNextWave()
{
	StartWave(CurrentWaveIndex + 1);
}
