#include "CSHWeaponBase.h"
#include "CSHBullet.h"
#include "SecondSemester_TeamCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
ACSHWeaponBase::ACSHWeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root")); SetRootComponent(Root);
    RecoilRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RecoilRoot")); RecoilRoot->SetupAttachment(Root);
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh")); WeaponMesh->SetupAttachment(RecoilRoot);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetOnlyOwnerSee(false);
    WeaponMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    WeaponMesh->SetRenderCustomDepth(true);
    WeaponMesh->SetCustomDepthStencilValue(42);
    WeaponMesh->SetOwnerNoSee(false);
    MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint")); MuzzlePoint->SetupAttachment(RecoilRoot);
}
void ACSHWeaponBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    RecoilOffset = FMath::VInterpTo(RecoilOffset, FVector::ZeroVector, DeltaSeconds, WeaponRecoverySpeed);
    RecoilRoot->SetRelativeLocation(RecoilOffset);
}
void ACSHWeaponBase::EquipTo(ASecondSemester_TeamCharacter* NewOwner)
{
    CharacterOwner = NewOwner; SetOwner(NewOwner); SetInstigator(NewOwner);
    CurrentAmmo = MagazineCapacity;
    ReserveAmmo = bInfiniteAmmo ? 0 : MagazineCapacity * 3;
    SetActorHiddenInGame(false);
    WeaponMesh->SetVisibility(true, true);
    WeaponMesh->SetHiddenInGame(false, true);
    AttachToComponent(NewOwner->GetWeaponSocketComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    SetActorRelativeTransform(FTransform::Identity);
}
void ACSHWeaponBase::StartFiring()
{
    if (!IsValid(CharacterOwner) || (!BulletClass && !bMeleeWeapon) || bTriggerHeld) return;
    if (GetWorld()->GetTimeSeconds() < NextAllowedFireTime) return;
    bTriggerHeld = true; FireOnce();
    if (bAutomatic) GetWorldTimerManager().SetTimer(FireTimer, this, &ACSHWeaponBase::FireOnce, FireInterval, true, FireInterval);
}
void ACSHWeaponBase::StopFiring() { bTriggerHeld = false; GetWorldTimerManager().ClearTimer(FireTimer); }
void ACSHWeaponBase::Reload()
{
    if (bInfiniteAmmo || CurrentAmmo >= MagazineCapacity || ReserveAmmo <= 0) return;
    StopFiring();
    const int32 AmmoToLoad = FMath::Min(MagazineCapacity - CurrentAmmo, ReserveAmmo);
    CurrentAmmo += AmmoToLoad;
    ReserveAmmo -= AmmoToLoad;
}
void ACSHWeaponBase::FireOnce()
{
    if (!bTriggerHeld || !IsValid(CharacterOwner) || (!BulletClass && !bMeleeWeapon)) return;
    if (!bInfiniteAmmo && CurrentAmmo <= 0) { StopFiring(); return; }
    if (!bInfiniteAmmo) --CurrentAmmo;
    NextAllowedFireTime = GetWorld()->GetTimeSeconds() + FireInterval;
    if (bMeleeWeapon)
    {
        const FVector Start = CharacterOwner->GetFirstPersonCameraComponent()->GetComponentLocation();
        const FVector Direction = CharacterOwner->GetFirstPersonCameraComponent()->GetForwardVector();
        const FVector End = Start + Direction * MeleeRange;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(CSHMelee), false, CharacterOwner);
        Query.AddIgnoredActor(this);
        TArray<FHitResult> Hits;
        GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(MeleeRadius), Query);
        TSet<AActor*> DamagedActors;
        for (const FHitResult& Hit : Hits)
        {
            AActor* Target = Hit.GetActor();
            if (!IsValid(Target) || DamagedActors.Contains(Target)) continue;
            DamagedActors.Add(Target);
            UGameplayStatics::ApplyPointDamage(Target, MeleeDamage, Direction, Hit, CharacterOwner->GetController(), this, UDamageType::StaticClass());
            if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
            {
                if (HitComponent->IsSimulatingPhysics()) HitComponent->AddImpulseAtLocation(Direction * MeleeKnockback, Hit.ImpactPoint);
            }
            if (ACharacter* HitCharacter = Cast<ACharacter>(Target)) HitCharacter->LaunchCharacter(Direction * MeleeKnockback, true, true);
        }
        // Melee weapons lunge forward, then Tick smoothly restores the resting position.
        RecoilOffset.X = FMath::Min(RecoilOffset.X + WeaponKickDistance, WeaponKickDistance * 1.5f);
        BP_OnFired();
        if (!bAutomatic) StopFiring();
        return;
    }
    const FVector CamLoc = CharacterOwner->GetFirstPersonCameraComponent()->GetComponentLocation();
    constexpr float AimConvergenceDistance = 5000.0f; // 50 m in Unreal units.
    const FVector CameraEnd = CamLoc + CharacterOwner->GetFirstPersonCameraComponent()->GetForwardVector() * AimConvergenceDistance;

    FCollisionQueryParams AimQuery(SCENE_QUERY_STAT(CSHWeaponAim), true, CharacterOwner);
    AimQuery.AddIgnoredActor(this);
    FHitResult AimHit;
    const bool bAimHit = GetWorld()->LineTraceSingleByChannel(AimHit, CamLoc, CameraEnd, ECC_Visibility, AimQuery);
    const FVector Target = bAimHit ? AimHit.ImpactPoint : CameraEnd;
    const FVector MuzzleLoc = MuzzlePoint->GetComponentLocation();
    FActorSpawnParameters Params; Params.Owner = CharacterOwner; Params.Instigator = CharacterOwner;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector CenterDirection = (Target - MuzzleLoc).GetSafeNormal();
    const float SpreadRadians = FMath::DegreesToRadians(SpreadAngleDegrees);
    FVector SpreadRight;
    FVector SpreadUp;
    CenterDirection.FindBestAxisVectors(SpreadRight, SpreadUp);
    const float MaxSpreadOffset = FMath::Tan(SpreadRadians);

    TArray<ACSHBullet*> SpawnedProjectiles;
    SpawnedProjectiles.Reserve(ProjectilesPerShot);
    for (int32 ProjectileIndex = 0; ProjectileIndex < ProjectilesPerShot; ++ProjectileIndex)
    {
        FVector ShotDirection = CenterDirection;
        if (ProjectilesPerShot > 1 && SpreadRadians > 0.0f)
        {
            // Realistic buckshot pattern: random each shot, concentrated near the center.
            const float Radius = FMath::Square(FMath::FRand());
            const float Azimuth = FMath::FRandRange(0.0f, 2.0f * PI);
            const FVector RadialDirection = SpreadRight * FMath::Cos(Azimuth) + SpreadUp * FMath::Sin(Azimuth);
            ShotDirection = (CenterDirection + RadialDirection * Radius * MaxSpreadOffset).GetSafeNormal();
        }
        if (ACSHBullet* SpawnedProjectile = GetWorld()->SpawnActor<ACSHBullet>(BulletClass, MuzzleLoc, ShotDirection.Rotation(), Params))
        {
            for (ACSHBullet* ExistingProjectile : SpawnedProjectiles)
            {
                SpawnedProjectile->IgnoreProjectile(ExistingProjectile);
            }
            SpawnedProjectiles.Add(SpawnedProjectile);
        }
    }
    if (MuzzleFlash)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlash,
            MuzzlePoint,
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTargetIncludingScale,
            true);
    }
    if (FireSound) UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLoc);
    RecoilOffset.X = FMath::Max(RecoilOffset.X - WeaponKickDistance, -WeaponKickDistance * 1.5f);
    CharacterOwner->AddControllerPitchInput(-CameraPitchKick);
    CharacterOwner->AddControllerYawInput(FMath::FRandRange(-CameraYawKick, CameraYawKick));
    BP_OnFired();
}


