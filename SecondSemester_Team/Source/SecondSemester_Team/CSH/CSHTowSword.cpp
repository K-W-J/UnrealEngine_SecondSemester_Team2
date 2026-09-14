#include "CSHTowSword.h"
#include "CSHTowedCar.h"
#include "SecondSemester_TeamCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

namespace
{
bool IsTowableCar(AActor* Actor)
{
    if (!IsValid(Actor) || Actor->ActorHasTag(TEXT("CSH_Towed")) || Actor->ActorHasTag(TEXT("NoTow"))) return false;
    if (Actor->ActorHasTag(TEXT("Towable")) || Actor->ActorHasTag(TEXT("Vehicle"))) return true;
    for (UClass* Type = Actor->GetClass(); Type; Type = Type->GetSuperClass())
        if (Type->GetName().Contains(TEXT("Vehicle"))) return true;
    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents(Meshes);
    for (auto* Mesh : Meshes)
        if (Mesh->Mobility == EComponentMobility::Movable && Mesh->GetStaticMesh()
            && Mesh->GetStaticMesh()->GetPathName().Contains(TEXT("/CitySampleVehicles/"))) return true;
    return false;
}
}

ACSHTowSword::ACSHTowSword()
{
    WeaponDisplayName = FText::FromString(TEXT("PARKING ENFORCER"));
    WeaponDescription = FText::FromString(TEXT("LMB: TOW / R: THROW"));
    bMeleeWeapon = true;
    bInfiniteAmmo = true;
    bAutomatic = false;
    FireInterval = .5f;
    WeaponKickDistance = 16.f;
    CameraPitchKick = CameraYawKick = 0.f;
    TowedCarClass = ACSHTowedCar::StaticClass();
}

void ACSHTowSword::StartFiring()
{
    if (!IsValid(CharacterOwner) || CharacterOwner->GetHealth() <= 0.f || Cars.Num() >= MaxCars
        || GetWorld()->GetTimeSeconds() < NextAllowedFireTime) return;
    NextAllowedFireTime = GetWorld()->GetTimeSeconds() + FMath::Max(.05f, FireInterval);
    RecoilOffset.X = WeaponKickDistance;
    BP_OnFired();
    const auto* Camera = CharacterOwner->GetFirstPersonCameraComponent();
    const FVector Start = Camera->GetComponentLocation();
    const FVector Direction = Camera->GetForwardVector();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(CSHTow), false, CharacterOwner);
    Query.AddIgnoredActor(this);
    TArray<FOverlapResult> Results;
    GetWorld()->OverlapMultiByObjectType(Results, Start + Direction * CaptureRange * .5f,
        Direction.ToOrientationQuat(), FCollisionObjectQueryParams::AllObjects,
        FCollisionShape::MakeSphere(CaptureRange * .5f + CaptureRadius), Query);
    AActor* Best = nullptr;
    float BestDistance = TNumericLimits<float>::Max();
    for (const auto& Result : Results)
    {
        AActor* Candidate = Result.GetActor();
        if (!IsTowableCar(Candidate) || Candidate == CharacterOwner) continue;
        FVector Origin, Extent;
        Candidate->GetActorBounds(true, Origin, Extent);
        const FVector Delta = Origin - Start;
        const float Along = FVector::DotProduct(Delta, Direction);
        if (Along < 0.f || Along > CaptureRange + Extent.GetMax()
            || (Delta - Direction * Along).Size() > CaptureRadius + Extent.GetMin()) continue;
        FHitResult Wall;
        if (GetWorld()->LineTraceSingleByChannel(Wall, Start, Origin, ECC_Visibility, Query)
            && Wall.GetActor() != Candidate) continue;
        if (Delta.SizeSquared() < BestDistance) { Best = Candidate; BestDistance = Delta.SizeSquared(); }
    }
    if (!Best || !TowedCarClass) return;
    FActorSpawnParameters Params;
    Params.Owner = CharacterOwner;
    Params.Instigator = CharacterOwner;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Carrier = GetWorld()->SpawnActor<ACSHTowedCar>(TowedCarClass, MuzzlePoint->GetComponentTransform(), Params);
    if (Carrier && Carrier->Capture(Best, MuzzlePoint, FMath::Max(.01f, HeldScale)))
    {
        Carrier->SetActorRelativeLocation(FVector(Cars.Num() * StackSpacing, 0, 0));
        Cars.Add(Carrier);
    }
    else if (Carrier) Carrier->Destroy();
}

void ACSHTowSword::Reload()
{
    if (!IsValid(CharacterOwner) || CharacterOwner->GetHealth() <= 0.f || Cars.IsEmpty()) return;
    auto* Camera = CharacterOwner->GetFirstPersonCameraComponent();
    for (int32 Index = 0; Index < Cars.Num(); ++Index)
    {
        if (!IsValid(Cars[Index])) continue;
        const float Side = Index - (Cars.Num() - 1) * .5f;
        const FVector Direction = (Camera->GetForwardVector() + Camera->GetRightVector() * Side * .08f).GetSafeNormal();
        Cars[Index]->Throw(Camera->GetComponentLocation() + Direction * 240.f,
            Direction, ThrowSpeed, ImpactDamage, ImpactRadius);
    }
    Cars.Empty();
    RecoilOffset.X = -WeaponKickDistance;
    NextAllowedFireTime = GetWorld()->GetTimeSeconds() + FireInterval;
}

void ACSHTowSword::ReleaseCars()
{
    for (const auto& Car : Cars) if (IsValid(Car)) { Car->Release(); Car->Destroy(); }
    Cars.Empty();
}

void ACSHTowSword::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Cars.RemoveAll([](const auto& Car) { return !IsValid(Car); });
    if (!IsValid(CharacterOwner) || CharacterOwner->GetHealth() <= 0.f) ReleaseCars();
}

void ACSHTowSword::EndPlay(const EEndPlayReason::Type Reason)
{
    ReleaseCars();
    Super::EndPlay(Reason);
}
