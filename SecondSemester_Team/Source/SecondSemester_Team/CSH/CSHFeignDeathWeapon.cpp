#include "CSHFeignDeathWeapon.h"
#include "CSHBullet.h"
#include "SecondSemester_TeamCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraFunctionLibrary.h"

void ACSHFeignDeathWeapon::StartFiring()
{
    if (bFeigning || !IsValid(CharacterOwner) || CharacterOwner->GetHealth() <= 0 || !BulletClass
        || GetWorld()->GetTimeSeconds() < NextAllowedFireTime || (!bInfiniteAmmo && CurrentAmmo <= 0)) return;
    auto* Body = CharacterOwner->GetMesh();
    if (!Body->GetPhysicsAsset()) return;
    OverviewCamera = nullptr;
    TArray<UCameraComponent*> Cameras;
    CharacterOwner->GetComponents<UCameraComponent>(Cameras);
    for (auto* Camera : Cameras)
        if (Camera->GetFName() == TEXT("CSHDeathCamera")) OverviewCamera = Cast<UCameraComponent>(Camera);
    if (!OverviewCamera) return;
    StopFiring();
    if (!bInfiniteAmmo) --CurrentAmmo;
    bFeigning = true;
    Elapsed = 0; UntilShot = .45f;
    SavedMeshTransform = Body->GetRelativeTransform();
    SavedCollisionProfile = Body->GetCollisionProfileName();
    bSavedOwnerNoSee = Body->bOwnerNoSee;
    SavedMovementMode = CharacterOwner->GetCharacterMovement()->MovementMode;
    CharacterOwner->GetCharacterMovement()->StopMovementImmediately();
    CharacterOwner->GetCharacterMovement()->DisableMovement();
    if (auto* PC = Cast<APlayerController>(CharacterOwner->GetController()))
    {
        PC->SetIgnoreMoveInput(true); PC->SetIgnoreLookInput(true); bChangedInput = true;
    }
    CharacterOwner->GetFirstPersonCameraComponent()->SetActive(false);
    OverviewCamera->SetActive(true);
    SetActorHiddenInGame(true);
    Body->SetOwnerNoSee(false);
    Body->SetVisibility(true);
    Body->SetCollisionProfileName(TEXT("Ragdoll"));
    Body->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    Body->SetAllBodiesSimulatePhysics(true);
    Body->SetSimulatePhysics(true);
    Body->WakeAllRigidBodies();
    Body->AddImpulse(CharacterOwner->GetActorForwardVector() * -180.f, NAME_None, true);
}

void ACSHFeignDeathWeapon::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bFeigning) return;
    if (!IsValid(CharacterOwner)) { bFeigning = false; return; }
    if (CharacterOwner->GetHealth() <= 0) { FinishFeign(); return; }
    Elapsed += DeltaSeconds;
    if (Elapsed >= FeignDuration) { FinishFeign(); return; }
    if (Elapsed > .45f)
        CharacterOwner->GetMesh()->SetAllPhysicsAngularVelocityInDegrees(FVector(0,0,SpinSpeed), false);
    UntilShot -= DeltaSeconds;
    if (UntilShot <= 0.f) { Spray(); UntilShot = FMath::Max(.03f, SprayInterval); }
}

void ACSHFeignDeathWeapon::Spray()
{
    const FVector Center = CharacterOwner->GetMesh()->GetSocketLocation(TEXT("pelvis")) + FVector(0,0,30);
    // Opposing shots turn around the body, independent of the hidden first-person gun.
    for (int32 i=0; i<2; ++i)
    {
        const FRotator Rotation(5.f, Elapsed * SpinSpeed + i * 180.f, 0.f);
        FActorSpawnParameters Params;
        Params.Owner = CharacterOwner; Params.Instigator = CharacterOwner;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        GetWorld()->SpawnActor<ACSHBullet>(BulletClass, Center, Rotation, Params);
        if (MuzzleFlash) UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleFlash, Center, Rotation);
    }
}

void ACSHFeignDeathWeapon::FinishFeign()
{
    if (!bFeigning) return;
    bFeigning = false;
    if (!IsValid(CharacterOwner)) return;
    if (bChangedInput)
    {
        if (auto* PC = Cast<APlayerController>(CharacterOwner->GetController()))
        { PC->SetIgnoreMoveInput(false); PC->SetIgnoreLookInput(false); }
        bChangedInput = false;
    }
    // Actual death keeps its own ragdoll/camera/input state; never resurrect it.
    if (CharacterOwner->GetHealth() <= 0) return;
    auto* Body = CharacterOwner->GetMesh();
    Body->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
    Body->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    Body->SetAllBodiesSimulatePhysics(false);
    Body->SetSimulatePhysics(false);
    Body->AttachToComponent(CharacterOwner->GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    Body->SetRelativeTransform(SavedMeshTransform);
    Body->SetCollisionProfileName(SavedCollisionProfile);
    Body->SetOwnerNoSee(bSavedOwnerNoSee);
    if (OverviewCamera) OverviewCamera->SetActive(false);
    CharacterOwner->GetFirstPersonCameraComponent()->SetActive(true);
    CharacterOwner->GetCharacterMovement()->SetMovementMode(static_cast<EMovementMode>(SavedMovementMode));
    SetActorHiddenInGame(false);
    NextAllowedFireTime = GetWorld()->GetTimeSeconds() + RecoveryCooldown;
}

void ACSHFeignDeathWeapon::EndPlay(const EEndPlayReason::Type Reason)
{
    FinishFeign();
    Super::EndPlay(Reason);
}
