#include "Enemies/SS_WaveManager.h"

#include "Enemies/SS_Enemy.h"
#include "Enemies/SS_EnemySpawner.h"
#include "Enemies/SS_WaveWidget.h"
#include "CSH/SecondSemester_TeamCharacter.h"
#include "CSH/SecondSemester_TeamPlayerController.h"
#include "CSH/CSHEndingWidget.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
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
	if (EndingWidget)
	{
		EndingWidget->RemoveFromParent();
		EndingWidget = nullptr;
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

void ASS_WaveManager::SetWaveUIVisible(bool bVisible)
{
	bWaveUIVisible = bVisible;
	if (WaveWidget)
	{
		WaveWidget->SetVisibility(bVisible
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
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
	TotalEnemyCount = 0;
	DefeatedEnemyCount = 0;
	EntryIndex = 0;
	EntrySpawnCount = 0;
	bWaveActive = true;
	for (const FSS_WaveEnemyEntry& Entry : WaveData[WaveIndex]->Enemies)
	{
		if (Entry.EnemyClass && Entry.Count > 0)
		{
			PendingEntries.Add(Entry);
			TotalEnemyCount += Entry.Count;
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
			&ASS_WaveManager::UpdateCountdownDisplay, 0.05f, true);
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
		UpdateWaveProgressDisplay();
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
		WaveWidget->SetWaveProgress(WaveStartDelay > 0.0f
			? FMath::Clamp(TimeRemaining / WaveStartDelay, 0.0f, 1.0f)
			: 0.0f);
	}
}

void ASS_WaveManager::UpdateWaveProgressDisplay()
{
	EnsureWaveWidget();
	if (!WaveWidget)
	{
		return;
	}

	const int32 RemainingEnemyCount = FMath::Max(TotalEnemyCount - DefeatedEnemyCount, 0);
	const float RemainingRatio = TotalEnemyCount > 0
		? static_cast<float>(RemainingEnemyCount) / static_cast<float>(TotalEnemyCount)
		: 0.0f;
	WaveWidget->SetWaveLabel(FString::Printf(TEXT("Wave %d   %d / %d"),
		CurrentWaveIndex + 1, RemainingEnemyCount, TotalEnemyCount));
	WaveWidget->SetWaveProgress(RemainingRatio);
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
			WaveWidget->SetVisibility(bWaveUIVisible
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed);
		}
	}
}

void ASS_WaveManager::StopWave()
{
	GetWorldTimerManager().ClearTimer(WaveSpawnTimer);
	GetWorldTimerManager().ClearTimer(WaveStartTimer);
	GetWorldTimerManager().ClearTimer(CountdownDisplayTimer);
	GetWorldTimerManager().ClearTimer(NextWaveTimer);
	GetWorldTimerManager().ClearTimer(BossHealthTimer);
	bWaveActive = false;
	bIsWaveCountdownActive = false;
	if (WaveWidget)
	{
		WaveWidget->SetWaveLabel(FString());
		WaveWidget->SetWaveProgress(0.0f);
	}
	for (const TWeakObjectPtr<AActor>& Enemy : LivingEnemies)
	{
		if (Enemy.IsValid())
		{
			Enemy->OnDestroyed.RemoveDynamic(this, &ASS_WaveManager::HandleEnemyDestroyed);
		}
	}
	LivingEnemies.Reset();
	BossEnemies.Reset();
	if (WaveWidget)
	{
		WaveWidget->SetBossHealthValues(TArray<float>());
	}
	AliveEnemyCount = 0;
	TotalEnemyCount = 0;
	DefeatedEnemyCount = 0;
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
			if (Enemy->bIsBossEnemy)
			{
				BossEnemies.Add(Enemy);
				UpdateBossHealthDisplay();
				if (!GetWorldTimerManager().IsTimerActive(BossHealthTimer))
				{
					GetWorldTimerManager().SetTimer(BossHealthTimer, this,
						&ASS_WaveManager::UpdateBossHealthDisplay,
						FMath::Max(BossHealthRefreshInterval, 0.02f), true);
				}
			}
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
	++DefeatedEnemyCount;
	BossEnemies.RemoveAll([Enemy](const TWeakObjectPtr<ASS_Enemy>& Boss)
	{
		return !Boss.IsValid() || Boss.Get() == Enemy;
	});
	UpdateBossHealthDisplay();
	AliveEnemyCount = LivingEnemies.Num();
	UpdateWaveProgressDisplay();
	CheckWaveCompleted();
}

void ASS_WaveManager::CheckWaveCompleted()
{
	if (!bWaveActive || bIsWaveCountdownActive || bIsSpawningWave || AliveEnemyCount > 0)
	{
		return;
	}
	bWaveActive = false;
	if (WaveCompletionHealAmount > 0.0f)
	{
		if (ASecondSemester_TeamCharacter* Player = Cast<ASecondSemester_TeamCharacter>(
			UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			Player->Heal(WaveCompletionHealAmount);
		}
	}
	// Defer advancement to avoid recursive StartWave calls for empty waves.
	const bool bHasNextWave = WaveData.IsValidIndex(CurrentWaveIndex + 1)
		&& IsValid(WaveData[CurrentWaveIndex + 1]);
	if (CurrentWaveIndex + 1 >= FMath::Max(FinalWaveNumber, 1) || !bHasNextWave)
	{
		ShowEndingScreen();
	}
	else
	{
		NextWaveTimer = GetWorldTimerManager().SetTimerForNextTick(
			this, &ASS_WaveManager::StartNextWave);
	}
}

void ASS_WaveManager::StartNextWave()
{
	StartWave(CurrentWaveIndex + 1);
}

void ASS_WaveManager::UpdateBossHealthDisplay()
{
	BossEnemies.RemoveAll([](const TWeakObjectPtr<ASS_Enemy>& Boss)
	{
		return !Boss.IsValid() || Boss->bIsDead || Boss->IsActorBeingDestroyed();
	});

	TArray<float> HealthValues;
	HealthValues.Reserve(BossEnemies.Num());
	for (const TWeakObjectPtr<ASS_Enemy>& Boss : BossEnemies)
	{
		if (Boss.IsValid())
		{
			HealthValues.Add(Boss->GetHealthNormalized());
		}
	}
	if (WaveWidget)
	{
		WaveWidget->SetBossHealthValues(HealthValues);
	}
	if (BossEnemies.IsEmpty() && GetWorld())
	{
		GetWorldTimerManager().ClearTimer(BossHealthTimer);
	}
}

void ASS_WaveManager::ShowEndingScreen()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	SetWaveUIVisible(false);
	if (ASecondSemester_TeamPlayerController* CSHController =
		Cast<ASecondSemester_TeamPlayerController>(PlayerController))
	{
		CSHController->SetGameplayHUDVisible(false);
	}
	if (!EndingWidget)
	{
		EndingWidget = CreateWidget<UCSHEndingWidget>(
			PlayerController, UCSHEndingWidget::StaticClass());
	}
	if (!EndingWidget)
	{
		return;
	}

	EndingWidget->AddToPlayerScreen(150);
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(EndingWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
	UGameplayStatics::SetGamePaused(this, true);
}
