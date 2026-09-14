#include "CSHTowedCar.h"
#include "SecondSemester_TeamCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/DamageType.h"

ACSHTowedCar::ACSHTowedCar()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

bool ACSHTowedCar::Capture(AActor* Car, USceneComponent* Mount, float Scale)
{
    if (!IsValid(Car) || !Car->GetRootComponent() || Car->GetRootComponent()->Mobility != EComponentMobility::Movable) return false;
    Target = Car;
    OriginalScale = Car->GetActorScale3D();
    OriginalTransform = Car->GetActorTransform();
    bCollision = Car->GetActorEnableCollision();
    bActorTick = Car->IsActorTickEnabled();
    FVector Origin, Extent;
    Car->GetActorBounds(true, Origin, Extent);
    SweepRadius = FMath::Clamp(Extent.GetMin(), 35.f, 120.f);
    TArray<UActorComponent*> Components;
    Car->GetComponents(Components);
    for (auto* Component : Components)
    {
        FSavedComponent State;
        State.Component = Component;
        State.bTick = Component->IsComponentTickEnabled();
        Component->SetComponentTickEnabled(false);
        if (auto* Primitive = Cast<UPrimitiveComponent>(Component))
        {
            State.bPhysics = Primitive->IsSimulatingPhysics();
            State.RelativeTransform = Primitive->GetRelativeTransform();
            State.Parent = Primitive->GetAttachParent();
            State.Socket = Primitive->GetAttachSocketName();
            Primitive->SetSimulatePhysics(false);
        }
        SavedComponents.Add(State);
    }
    Car->Tags.AddUnique(TEXT("CSH_Towed"));
    Car->SetActorTickEnabled(false);
    Car->SetActorEnableCollision(false);
    AttachToComponent(Mount, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Car->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Car->SetActorRelativeScale3D(OriginalScale * Scale);
    return true;
}

void ACSHTowedCar::Throw(const FVector& Start, const FVector& Direction, float Speed, float Damage, float Radius)
{
    if (!IsValid(Target)) { Destroy(); return; }
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    // Start at the held position and sweep the first advance too: never teleport through a wall.
    Target->SetActorScale3D(OriginalScale);
    Velocity = Direction * FMath::Max(1.f, Speed);
    DamageAmount = Damage;
    DamageRadius = Radius;
    bFlying = true;
    TimeFlying = 0.f;
}

void ACSHTowedCar::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsValid(Target)) { Destroy(); return; }
    if (!bFlying) return;
    TimeFlying += DeltaSeconds;
    Velocity.Z -= Gravity * DeltaSeconds;
    const FVector Start = GetActorLocation();
    const FVector End = Start + Velocity * DeltaSeconds;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(CSHTowFlight), false, Target);
    Query.AddIgnoredActor(this);
    Query.AddIgnoredActor(GetOwner());
    FHitResult Hit;
    const bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeSphere(SweepRadius), Query);
    SetActorLocation(bHit ? Hit.Location : End);
    AddActorLocalRotation(FRotator(0.f, 200.f * DeltaSeconds, 70.f * DeltaSeconds));
    if (bHit)
    {
        TArray<AActor*> Ignore = {this, Target, GetOwner()};
        UGameplayStatics::ApplyRadialDamage(this, DamageAmount, Hit.ImpactPoint, DamageRadius,
            UDamageType::StaticClass(), Ignore, this, GetInstigatorController(), true);
        if (ImpactEffect) UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect,
            Hit.ImpactPoint, FRotator::ZeroRotator, FVector(EffectScale));
        if (auto* Player = Cast<ASecondSemester_TeamCharacter>(GetOwner()))
            Player->ApplyExplosionCameraShake(Hit.ImpactPoint, DamageRadius, DamageRadius * 3.f);
        Release();
        Destroy();
    }
    else if (TimeFlying >= FlightSeconds) { Release(); Destroy(); }
}

void ACSHTowedCar::Release()
{
    if (!IsValid(Target)) return;
    Target->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Target->SetActorScale3D(OriginalScale);
    Target->SetActorEnableCollision(bCollision);
    if (!bFlying) Target->SetActorTransform(OriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
    Target->SetActorTickEnabled(bActorTick);
    Target->Tags.Remove(TEXT("CSH_Towed"));
    for (const auto& State : SavedComponents)
    {
        auto* Component = State.Component.Get();
        if (!Component) continue;
        Component->SetComponentTickEnabled(State.bTick);
        if (auto* Primitive = Cast<UPrimitiveComponent>(Component))
        {
            if (State.Parent.IsValid())
            {
                Primitive->AttachToComponent(State.Parent.Get(), FAttachmentTransformRules::KeepRelativeTransform, State.Socket);
                Primitive->SetRelativeTransform(State.RelativeTransform);
            }
            Primitive->SetSimulatePhysics(State.bPhysics);
            if (State.bPhysics) Primitive->SetPhysicsLinearVelocity(bFlying ? Velocity * .25f : FVector::ZeroVector);
        }
    }
    Target = nullptr;
}

void ACSHTowedCar::EndPlay(const EEndPlayReason::Type Reason)
{
    Release();
    Super::EndPlay(Reason);
}
