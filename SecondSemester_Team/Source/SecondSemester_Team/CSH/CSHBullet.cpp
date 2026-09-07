#include "CSHBullet.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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
    TracerLight->SetLightColor(TracerColor);
    TracerLight->SetIntensity(TracerLightIntensity);
    TracerLight->SetAttenuationRadius(140.0f);
    TracerLight->SetCastShadows(false);
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = InitialSpeed;
    ProjectileMovement->MaxSpeed = InitialSpeed;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bForceSubStepping = true;
    ProjectileMovement->MaxSimulationTimeStep = 0.005f;
    ProjectileMovement->MaxSimulationIterations = 12;
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
    TracerLight->SetLightColor(TracerColor);
    TracerLight->SetIntensity(TracerLightIntensity);
    ProjectileMovement->InitialSpeed = InitialSpeed;
    ProjectileMovement->MaxSpeed = InitialSpeed;
    ProjectileMovement->Velocity = GetActorForwardVector() * InitialSpeed;

    AActor* WeaponOwner = GetOwner();
    AActor* PlayerActor = GetInstigator();
    Collision->IgnoreActorWhenMoving(WeaponOwner, true);
    Collision->IgnoreActorWhenMoving(PlayerActor, true);

    // Make the fast projectile readable as a bright, elongated tracer at runtime.
    BulletMesh->SetVisibility(true, true);
    BulletMesh->SetHiddenInGame(false, true);
    BulletMesh->SetOnlyOwnerSee(false);
    BulletMesh->SetOwnerNoSee(false);

}
void ACSHBullet::OnBulletHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector, const FHitResult& Hit)
{
    if (!IsValid(OtherActor) || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;
    UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetActorForwardVector(), Hit, GetInstigatorController(), this, UDamageType::StaticClass());
    Destroy();
}
