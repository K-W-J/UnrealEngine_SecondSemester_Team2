#include "CSHRandomBoxSpawner.h"
#include "CSHWeaponBox.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ACSHRandomBoxSpawner::ACSHRandomBoxSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ACSHRandomBoxSpawner::BeginPlay()
{
    Super::BeginPlay();
    // Only one manager should own the random-box population in this level.
    for (TActorIterator<ACSHRandomBoxSpawner> It(GetWorld()); It; ++It)
    {
        if (*It != this && It->HasActorBegunPlay() && !It->bStopping)
        {
            bStopping = true;
            return;
        }
    }
    if (!BoxClass || BoxPoints.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomBoxSpawner requires BoxClass and BoxPoints: %s"), *GetName());
        return;
    }
    TArray<AActor*> Existing;
    for (TActorIterator<ACSHWeaponBox> It(GetWorld()); It; ++It)
        if (It->IsA(BoxClass)) Existing.Add(*It);
    for (AActor* Box : Existing) Box->Destroy();
    GetWorldTimerManager().SetTimer(RespawnTimer, this, &ACSHRandomBoxSpawner::SpawnNextBox, .2f, false);
}

void ACSHRandomBoxSpawner::SpawnNextBox()
{
    if (bStopping || IsValid(ActiveBox) || !BoxClass) return;
    TArray<AActor*> Candidates;
    for (const auto& Point : BoxPoints)
        if (IsValid(Point) && Point != LastPoint.Get()) Candidates.AddUnique(Point);
    // One valid point cannot satisfy "a different point". Do not loop pickups in place.
    if (Candidates.IsEmpty()) return;
    AActor* Point = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
    LastPoint = Point;
    const FTransform Transform(Point->GetActorRotation(), Point->GetActorLocation() + SpawnOffset);
    ActiveBox = GetWorld()->SpawnActorDeferred<ACSHWeaponBox>(BoxClass, Transform, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!ActiveBox)
    {
        LastPoint.Reset();
        GetWorldTimerManager().SetTimer(RespawnTimer, this, &ACSHRandomBoxSpawner::SpawnNextBox, 1.f, false);
        return;
    }
    // Bind before BeginPlay: the box may be collected immediately when spawned near the player.
    ActiveBox->OnDestroyed.AddDynamic(this, &ACSHRandomBoxSpawner::OnBoxDestroyed);
    ActiveBox->FinishSpawning(Transform);
}

void ACSHRandomBoxSpawner::OnBoxDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor != ActiveBox) return;
    ActiveBox = nullptr;
    if (!bStopping)
        GetWorldTimerManager().SetTimer(RespawnTimer, this, &ACSHRandomBoxSpawner::SpawnNextBox,
            FMath::Max(.1f, RespawnDelay), false);
}

void ACSHRandomBoxSpawner::EndPlay(const EEndPlayReason::Type Reason)
{
    bStopping = true;
    GetWorldTimerManager().ClearTimer(RespawnTimer);
    if (IsValid(ActiveBox))
    {
        ActiveBox->OnDestroyed.RemoveDynamic(this, &ACSHRandomBoxSpawner::OnBoxDestroyed);
        ActiveBox->Destroy();
        ActiveBox = nullptr;
    }
    Super::EndPlay(Reason);
}
