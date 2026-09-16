#include "Enemies/SS_WaveManager.h"

#include "Enemies/SS_Enemy.h"
#include "Enemies/SS_EnemySpawner.h"
#include "Enemies/SS_WaveWidget.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
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
	if (WaveWidget)
	{
		WaveWidget->RemoveFromParent();
		WaveWidget = nullptr;
	}
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
	EnsureWaveWidget();
	if (WaveStartDelay > 0.0f)
	{
		bIsWaveCountdownActive = true;
		GetWorldTimerManager().SetTimer(WaveStartTimer, this,
			&ASS_WaveManager::BeginWaveSpawning, WaveStartDelay, false);
		UpdateCountdownDisplay();
		GetWorldTimerManager().SetTimer(CountdownDisplayTimer, this,
			&ASS_WaveManager::UpdateCountdownDisplay, 1.0f, true);
		return;
	}
	BeginWaveSpawning();
}

void ASS_WaveManager::BeginWaveSpawning()
{
	GetWorldTimerManager().ClearTimer(WaveStartTimer);
	GetWorldTimerManager().ClearTimer(CountdownDisplayTimer);
	bIsWaveCountdownActive = false;
	EnsureWaveWidget();
	if (WaveWidget)
	{
		WaveWidget->SetWaveLabel(FString::Printf(TEXT("Wave %d"), CurrentWaveIndex + 1));
	}
	if (PendingEntries.IsEmpty())
	{
		CheckWaveCompleted();
		return;
	}
	bIsSpawningWave = true;
	GetWorldTimerManager().SetTimer(WaveSpawnTimer, this, &ASS_WaveManager::SpawnNextEnemy,
		FMath::Max(WaveData[CurrentWaveIndex]->SpawnInterval, 0.05f), true);
	SpawnNextEnemy();
}

void ASS_WaveManager::UpdateCountdownDisplay()
{
	EnsureWaveWidget();
	if (WaveWidget && bIsWaveCountdownActive)
	{
		const float TimeRemaining = GetWorldTimerManager().GetTimerRemaining(WaveStartTimer);
		const int32 Seconds = FMath::Max(1, FMath::CeilToInt(TimeRemaining));
		WaveWidget->SetWaveLabel(FString::Printf(TEXT("Wave %d  -  %d"),
			CurrentWaveIndex + 1, Seconds));
	}
}

void ASS_WaveManager::EnsureWaveWidget()
{
	if (WaveWidget || !GetWorld())
	{
		return;
	}
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController && PlayerController->IsLocalController())
	{
		WaveWidget = CreateWidget<USS_WaveWidget>(PlayerController, USS_WaveWidget::StaticClass());
		if (WaveWidget)
		{
			WaveWidget->AddToPlayerScreen(20);
		}
	}
}

void ASS_WaveManager::StopWave()
{
	GetWorldTimerManager().ClearTimer(WaveSpawnTimer);
	GetWorldTimerManager().ClearTimer(WaveStartTimer);
	GetWorldTimerManager().ClearTimer(CountdownDisplayTimer);
	GetWorldTimerManager().ClearTimer(NextWaveTimer);
	bWaveActive = false;
	bIsWaveCountdownActive = false;
	if (WaveWidget)
	{
		WaveWidget->SetWaveLabel(FString());
	}
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
	if (!bWaveActive || bIsWaveCountdownActive || bIsSpawningWave || AliveEnemyCount > 0)
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
