#include "CSHBullet.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "SecondSemester_TeamCharacter.h"

ACSHBullet::ACSHBullet()
{
    PrimaryActorTick.bCanEverTick = false;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(2.0f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionResponseToAllChannels(ECR_Block);
    Collision->SetNotifyRigidBodyCollision(true);
    Collision->OnComponentHit.AddDynamic(this, &ACSHBullet::OnBulletHit);
    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(Collision);
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BulletMesh->SetCastShadow(false);
    BulletMesh->SetOnlyOwnerSee(false);
    BulletMesh->SetOwnerNoSee(false);
    TracerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TracerLight"));
    TracerLight->SetupAttachment(Collision);
    TracerLight->SetCastShadows(false);
    TracerLight->SetAttenuationRadius(140.0f);
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bForceSubStepping = true;
    ProjectileMovement->MaxSimulationTimeStep = 0.005f;
    ProjectileMovement->MaxSimulationIterations = 12;
    static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionAsset(TEXT("/Game/Art/Realistic_Starter_VFX_Pack_Vol2/Particles/Explosion/P_Explosion_Big_A.P_Explosion_Big_A"));
    if (ExplosionAsset.Succeeded()) ImpactExplosionEffect = ExplosionAsset.Object;
}

void ACSHBullet::IgnoreProjectile(ACSHBullet* OtherProjectile)
{
    if (!IsValid(OtherProjectile) || OtherProjectile == this) return;
    Collision->IgnoreActorWhenMoving(OtherProjectile, true);
    OtherProjectile->Collision->IgnoreActorWhenMoving(this, true);
}

void ACSHBullet::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(LifeSeconds);
    Collision->SetSphereRadius(CollisionRadius);
    TracerLight->SetLightColor(TracerColor);
    TracerLight->SetIntensity(TracerLightIntensity);
    ProjectileMovement->InitialSpeed = InitialSpeed;
    ProjectileMovement->MaxSpeed = InitialSpeed;
    ProjectileMovement->ProjectileGravityScale = GravityScale;
    ProjectileMovement->Velocity = GetActorForwardVector() * InitialSpeed;
    Collision->IgnoreActorWhenMoving(GetOwner(), true);
    Collision->IgnoreActorWhenMoving(GetInstigator(), true);
    BulletMesh->SetVisibility(true, true);
    BulletMesh->SetHiddenInGame(false, true);
    if (bHoming) AcquireHomingTarget();
}

void ACSHBullet::AcquireHomingTarget()
{
    AActor* BestTarget = nullptr;
    float BestDistanceSq = FMath::Square(HomingSearchRadius);
    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        APawn* Candidate = *It;
        if (!IsValid(Candidate) || Candidate == GetInstigator() || Candidate->IsActorBeingDestroyed()) continue;
        const float DistanceSq = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
        if (DistanceSq < BestDistanceSq) { BestDistanceSq = DistanceSq; BestTarget = Candidate; }
    }
    if (BestTarget && BestTarget->GetRootComponent())
    {
        ProjectileMovement->bIsHomingProjectile = true;
        ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
        ProjectileMovement->HomingTargetComponent = BestTarget->GetRootComponent();
    }
}

void ACSHBullet::OnBulletHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
    FVector, const FHitResult& Hit)
{
    if (!IsValid(OtherActor) || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;
    if (bSpawnImpactExplosion)
    {
        if (ImpactExplosionEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactExplosionEffect, Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation(), FVector(ImpactExplosionScale), true);
        }
        if (ASecondSemester_TeamCharacter* Player = Cast<ASecondSemester_TeamCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
        {
            Player->ApplyExplosionCameraShake(Hit.ImpactPoint, CameraShakeInnerRadius, CameraShakeOuterRadius);
        }
    }
    if (bExplosive)
    {
        TArray<AActor*> Ignored{GetOwner(), GetInstigator(), this};
        UGameplayStatics::ApplyRadialDamageWithFalloff(this, ExplosionDamage, ExplosionDamage * .35f,
            Hit.ImpactPoint, ExplosionRadius * .25f, ExplosionRadius, 1.f, UDamageType::StaticClass(),
            Ignored, this, GetInstigatorController(), ECC_Visibility);
    }
    else
    {
        UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetActorForwardVector(), Hit,
            GetInstigatorController(), this, UDamageType::StaticClass());
    }
    if (KnockbackImpulse > 0.f && OtherComponent)
    {
        const FVector Impulse = GetActorForwardVector() * KnockbackImpulse;
        if (OtherComponent->IsSimulatingPhysics()) OtherComponent->AddImpulseAtLocation(Impulse, Hit.ImpactPoint);
        if (ACharacter* Character = Cast<ACharacter>(OtherActor)) Character->LaunchCharacter(Impulse, true, true);
    }
    if (bPiercing && !bExplosive && ++PenetrationCount < MaxPenetrations)
    {
        Collision->IgnoreActorWhenMoving(OtherActor, true);
        return;
    }
    Destroy();
}
