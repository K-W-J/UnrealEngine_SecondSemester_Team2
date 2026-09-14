#include "SS_EnemySpawner.h"

#include "UObject/ConstructorHelpers.h"
#include "Enemies/SS_Enemy.h"
#include "Enemies/SS_WaveManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"

ASS_EnemySpawner::ASS_EnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnRoot"));
	static ConstructorHelpers::FClassFinder<ASS_Enemy> EnemyBlueprintClass(
		TEXT("/Game/Blueprints/Enemies/BP_SS_Enemy"));
	EnemyClass = ASS_Enemy::StaticClass();
	if (EnemyBlueprintClass.Succeeded())
	{
		EnemyClass = EnemyBlueprintClass.Class;
	}
}

void ASS_EnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// Check before the immediate spawn, independent of actor BeginPlay ordering.
	for (TActorIterator<ASS_WaveManager> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It))
		{
			StopAutomaticSpawning();
			return;
		}
	}

	if (bSpawnImmediately)
	{
		SpawnEnemy();
	}

	if (SpawnInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ASS_EnemySpawner::SpawnEnemy,
			SpawnInterval,
			true);
	}
}

void ASS_EnemySpawner::SpawnEnemy()
{
	SpawnEnemyOfClass(EnemyClass);
}

void ASS_EnemySpawner::StopAutomaticSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

ASS_Enemy* ASS_EnemySpawner::SpawnEnemyOfClass(TSubclassOf<ASS_Enemy> ClassToSpawn)
{
	UWorld* World = GetWorld();
	if (World == nullptr || ClassToSpawn == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return World->SpawnActor<ASS_Enemy>(ClassToSpawn, GetActorTransform(), SpawnParameters);
}

