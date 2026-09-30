// Copyright Epic Games, Inc. All Rights Reserved.

#include "SecondSemester_TeamCharacter.h"
#include "SecondSemester_TeamPlayerController.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/MeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SecondSemester_Team.h"
#include "CSHWeaponBase.h"
#include "CSHFists.h"
#include "InputCoreTypes.h"
#include "Enemies/SS_Enemy.h"
#include "CSHDamageBorderWidget.h"
#include "Blueprint/UserWidget.h"

ASecondSemester_TeamCharacter::ASecondSemester_TeamCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Health = MaxHealth;
	Stamina = MaxStamina;
	// Set size for collision capsule  
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	
FirstPersonMesh->SetRenderCustomDepth(true);
	
FirstPersonMesh->SetCustomDepthStencilValue(42);
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = false;
	FirstPersonCameraComponent->bEnableFirstPersonScale = false;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 1.0f;

	CSHDeathSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CSHDeathSpringArm"));
	CSHDeathSpringArm->SetupAttachment(GetCapsuleComponent());
	CSHDeathSpringArm->TargetArmLength = 380.0f;
	CSHDeathSpringArm->SetRelativeRotation(FRotator(-18.0f, 0.0f, 0.0f));
	CSHDeathSpringArm->bUsePawnControlRotation = false;
	CSHDeathSpringArm->bEnableCameraLag = true;
	CSHDeathSpringArm->CameraLagSpeed = 5.0f;

	CSHDeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CSHDeathCamera"));
	CSHDeathCamera->SetupAttachment(CSHDeathSpringArm, USpringArmComponent::SocketName);
	CSHDeathCamera->SetActive(false);
	CSHDeathCamera->bAutoActivate = false;

	// Weapon attachment point exposed on BP_CSH_Player for easy viewport adjustment.
	WeaponSocket = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponSocket"));
	WeaponSocket->SetupAttachment(FirstPersonCameraComponent);
	WeaponSocket->SetRelativeLocation(FVector(35.0f, 12.0f, -18.0f));

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	
GetMesh()->SetRenderCustomDepth(true);
	
GetMesh()->SetCustomDepthStencilValue(42);

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	OnActorHit.AddUniqueDynamic(this, &ASecondSemester_TeamCharacter::OnEnemyCarActorHit);
	GetCapsuleComponent()->OnComponentHit.AddUniqueDynamic(this, &ASecondSemester_TeamCharacter::OnEnemyCarCapsuleHit);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->MaxWalkSpeed = CSHWalkSpeed;
}


void ASecondSemester_TeamCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
    UpdateBodyVisibility(FirstPersonCameraComponent->IsActive() && !CSHDeathCamera->IsActive());
	UpdateSprint(DeltaSeconds);
	UpdateDamageCameraKick(DeltaSeconds);
	EnsureDamageBorderWidget();
	UpdateDamageBorder(DeltaSeconds);
	UpdateExplosionCameraShake(DeltaSeconds);
}

void ASecondSemester_TeamCharacter::BeginPlay()
{
    Super::BeginPlay();
    CSHDeathCamera->Deactivate();
    FirstPersonCameraComponent->Activate();
    UpdateBodyVisibility(true);
	EnsureDamageBorderWidget();
    if (!IsValid(CurrentWeapon))
    {
        UClass* FistsClass=LoadClass<ACSHFists>(nullptr,TEXT("/Game/CSH/Buleprint/Weapons/Fists/BP_CSH_Fists.BP_CSH_Fists_C"));
        EquipWeapon(FistsClass ? FistsClass : ACSHFists::StaticClass());
    }
}

void ASecondSemester_TeamCharacter::EnsureDamageBorderWidget()
{
	if (DamageBorderWidget)
	{
		return;
	}
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		DamageBorderWidget = CreateWidget<UCSHDamageBorderWidget>(
			PlayerController, UCSHDamageBorderWidget::StaticClass());
		if (DamageBorderWidget)
		{
			DamageBorderWidget->SetFlashTint(DamageBorderColor);
			DamageBorderWidget->AddToPlayerScreen(20);
			DamageBorderWidget->SetFlashOpacity(
				DamageBorderTimeRemaining / FMath::Max(DamageBorderDuration, 0.05f));
		}
	}
}

void ASecondSemester_TeamCharacter::OnEnemyCarActorHit(AActor* SelfActor, AActor* OtherActor,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ASS_Enemy* Enemy = Cast<ASS_Enemy>(OtherActor))
	{
		ApplyEnemyCarImpact(Enemy);
	}
}

void ASecondSemester_TeamCharacter::OnEnemyCarCapsuleHit(UPrimitiveComponent* HitComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ASS_Enemy* Enemy = Cast<ASS_Enemy>(OtherActor))
	{
		ApplyEnemyCarImpact(Enemy);
	}
}

void ASecondSemester_TeamCharacter::ApplyEnemyCarImpact(ASS_Enemy* Enemy)
{
	if (!IsValid(Enemy) || Enemy->bIsDead || bDeathStateEntered || Health <= 0.0f || !GetWorld())
	{
		return;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextEnemyImpactTime)
	{
		return;
	}
	FVector AwayFromEnemy = GetActorLocation() - Enemy->GetActorLocation();
	AwayFromEnemy.Z = 0.0f;
	if (!AwayFromEnemy.Normalize())
	{
		AwayFromEnemy = -Enemy->GetVelocity().GetSafeNormal2D();
		if (AwayFromEnemy.IsNearlyZero())
		{
			AwayFromEnemy = -GetActorForwardVector().GetSafeNormal2D();
		}
	}
	const float CurrentClosingSpeed = FVector::DotProduct(
		Enemy->GetVelocity(), AwayFromEnemy);
	const float RecentClosingSpeed = FVector::DotProduct(
		Enemy->GetRecentDriveVelocity(), AwayFromEnemy);
	const float ClosingSpeed = FMath::Max(0.0f, FMath::Max(CurrentClosingSpeed, RecentClosingSpeed));
	if (ClosingSpeed <= 0.0f)
	{
		return;
	}
	NextEnemyImpactTime = Now + FMath::Max(0.0f, EnemyImpactCooldown);
	const float ImpactDamage = ClosingSpeed >= MinimumEnemyImpactDamageSpeed
		? FMath::Clamp(ClosingSpeed / 100.0f * EnemyImpactDamagePer100Speed,
			0.0f, MaximumEnemyImpactDamage)
		: 0.0f;
	const float LaunchScale = ClosingSpeed / FMath::Max(EnemyImpactReferenceSpeed, 1.0f);
	const float HorizontalLaunchSpeed = FMath::Clamp(
		EnemyImpactHorizontalSpeed * LaunchScale, 0.0f, MaximumEnemyImpactHorizontalSpeed);
	const float UpwardLaunchSpeed = FMath::Clamp(
		EnemyImpactUpwardSpeed * LaunchScale, 0.0f, MaximumEnemyImpactUpwardSpeed);
	if (HorizontalLaunchSpeed > 0.0f || UpwardLaunchSpeed > 0.0f)
	{
		LaunchCharacter(AwayFromEnemy * HorizontalLaunchSpeed
			+ FVector::UpVector * UpwardLaunchSpeed, true, true);
	}
	if (ImpactDamage > 0.0f)
	{
		UGameplayStatics::ApplyDamage(this, ImpactDamage, Enemy->GetController(),
			Enemy, UDamageType::StaticClass());
	}
}

void ASecondSemester_TeamCharacter::UpdateBodyVisibility(bool bFirstPerson)
{
    TArray<UMeshComponent*> BodyMeshes;
    GetComponents<UMeshComponent>(BodyMeshes);
    for (UMeshComponent* BodyPart : BodyMeshes)
    {
        // Actor-owned body meshes only: never propagate to camera-attached weapons.
        const bool bHide = BodyPart == FirstPersonMesh || bFirstPerson;
        if (BodyPart->FirstPersonPrimitiveType != EFirstPersonPrimitiveType::None)
        {
            BodyPart->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;
            BodyPart->MarkRenderStateDirty();
        }
        BodyPart->SetOnlyOwnerSee(false);
        BodyPart->SetOwnerNoSee(bHide);
        BodyPart->SetVisibility(!bHide, false);
        BodyPart->SetHiddenInGame(bHide, false);
    }
}

void ASecondSemester_TeamCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
    Super::CalcCamera(DeltaTime, OutResult);
    // Use the actual chosen POV, after Blueprint tick and camera activation.
    const bool bAtHead = FVector::DistSquared(OutResult.Location,
        FirstPersonCameraComponent->GetComponentLocation()) < FMath::Square(50.0f);
    UpdateBodyVisibility(bAtHead && !CSHDeathCamera->IsActive());
}

float ASecondSemester_TeamCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float FinalDamage = AppliedDamage > 0.0f ? AppliedDamage : FMath::Max(0.0f, DamageAmount);
	Health = FMath::Clamp(Health - FinalDamage, 0.0f, MaxHealth);
	if (FinalDamage > 0.0f)
	{
		ApplyDamageCameraKick(FinalDamage, DamageCauser);
		DamageBorderTimeRemaining = FMath::Max(DamageBorderDuration, 0.05f);
		EnsureDamageBorderWidget();
		if (DamageBorderWidget)
		{
			DamageBorderWidget->SetFlashTint(DamageBorderColor);
			DamageBorderWidget->SetFlashOpacity(1.0f);
		}
	}
	if (Health <= 0.0f)
	{
		EnterDeathState();
	}
	return FinalDamage;
}

void ASecondSemester_TeamCharacter::EnterDeathState()
{
	if (bDeathStateEntered) return;
	bDeathStateEntered = true;
	StopSprint();
	StopWeaponFire();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonCameraComponent->SetActive(false);
	FirstPersonMesh->SetVisibility(false, true);
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetVisibility(true, true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	CSHDeathCamera->SetActive(true);
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);
		PlayerController->SetViewTargetWithBlend(this, 0.35f, VTBlend_Cubic);
	}
}

void ASecondSemester_TeamCharacter::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f) return;
	Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
}

void ASecondSemester_TeamCharacter::DebugTakeDamage()
{
	UGameplayStatics::ApplyDamage(this, 10.0f, GetController(), this, UDamageType::StaticClass());
}

void ASecondSemester_TeamCharacter::DebugHeal()
{
	Heal(10.0f);
}

void ASecondSemester_TeamCharacter::ApplyDamageCameraKick(float DamageAmount, const AActor* DamageCauser)
{
	if (!Controller || MaxHealth <= 0.0f) return;

	const float DamageAlpha = FMath::Clamp(DamageAmount / MaxHealth, 0.0f, 1.0f);
	const float KickStrength = FMath::Sqrt(DamageAlpha);
	float Side = FMath::RandBool() ? 1.0f : -1.0f;
	if (DamageCauser)
	{
		const FVector ToCauser = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		const float SideDot = FVector::DotProduct(GetActorRightVector(), ToCauser);
		if (FMath::Abs(SideDot) > 0.1f) Side = -FMath::Sign(SideDot);
	}

	const FRotator NewKick(-MaxDamageKickPitch * KickStrength, MaxDamageKickYaw * KickStrength * Side, MaxDamageKickRoll * KickStrength * -Side);
	DamageCameraKick += NewKick;
	DamageCameraKick.Pitch = FMath::Clamp(DamageCameraKick.Pitch, -MaxDamageKickPitch, MaxDamageKickPitch);
	DamageCameraKick.Yaw = FMath::Clamp(DamageCameraKick.Yaw, -MaxDamageKickYaw, MaxDamageKickYaw);
	DamageCameraKick.Roll = FMath::Clamp(DamageCameraKick.Roll, -MaxDamageKickRoll, MaxDamageKickRoll);
	Controller->SetControlRotation((Controller->GetControlRotation() + NewKick).GetNormalized());
}

void ASecondSemester_TeamCharacter::UpdateDamageCameraKick(float DeltaSeconds)
{
	if (!Controller || DamageCameraKick.IsNearlyZero(0.01f)) return;
	const FRotator PreviousKick = DamageCameraKick;
	DamageCameraKick = FMath::RInterpTo(DamageCameraKick, FRotator::ZeroRotator, DeltaSeconds, DamageKickRecoverySpeed);
	Controller->SetControlRotation((Controller->GetControlRotation() + (DamageCameraKick - PreviousKick)).GetNormalized());
}

void ASecondSemester_TeamCharacter::UpdateDamageBorder(float DeltaSeconds)
{
	if (!DamageBorderWidget || DamageBorderTimeRemaining <= 0.0f)
	{
		return;
	}
	DamageBorderTimeRemaining = FMath::Max(0.0f, DamageBorderTimeRemaining - DeltaSeconds);
	const float Duration = FMath::Max(DamageBorderDuration, 0.05f);
	DamageBorderWidget->SetFlashOpacity(DamageBorderTimeRemaining / Duration);
}

void ASecondSemester_TeamCharacter::ApplyExplosionCameraShake(const FVector& ExplosionLocation, float InnerRadius, float OuterRadius)
{
    if (!Controller || bDeathStateEntered || OuterRadius <= 0.0f) return;
    const float Distance = FVector::Distance(GetActorLocation(), ExplosionLocation);
    const float SafeInnerRadius = FMath::Min(InnerRadius, OuterRadius);
    const float Strength = 1.0f - FMath::Clamp((Distance - SafeInnerRadius) / FMath::Max(OuterRadius - SafeInnerRadius, 1.0f), 0.0f, 1.0f);
    if (Strength <= 0.0f) return;

    ExplosionShakeStrength = FMath::Max(ExplosionShakeStrength, Strength);
    ExplosionShakeDuration = 0.55f;
    ExplosionShakeRemaining = ExplosionShakeDuration;
    ExplosionShakePhase = FMath::FRandRange(0.0f, 2.0f * PI);
}

void ASecondSemester_TeamCharacter::UpdateExplosionCameraShake(float DeltaSeconds)
{
    if (!Controller || (ExplosionShakeRemaining <= 0.0f && PreviousExplosionShake.IsNearlyZero())) return;

    ExplosionShakeRemaining = FMath::Max(0.0f, ExplosionShakeRemaining - DeltaSeconds);
    const float Time = ExplosionShakeDuration - ExplosionShakeRemaining;
    const float Envelope = ExplosionShakeDuration > 0.0f ? ExplosionShakeRemaining / ExplosionShakeDuration : 0.0f;
    FRotator NewShake = FRotator::ZeroRotator;
    if (ExplosionShakeRemaining > 0.0f)
    {
        const float Amplitude = ExplosionShakeStrength * Envelope;
        NewShake.Pitch = FMath::Sin(Time * 55.0f + ExplosionShakePhase) * 2.2f * Amplitude;
        NewShake.Yaw = FMath::Sin(Time * 47.0f + ExplosionShakePhase * 1.7f) * 1.8f * Amplitude;
        NewShake.Roll = FMath::Sin(Time * 63.0f + ExplosionShakePhase * 0.7f) * 2.5f * Amplitude;
    }
    Controller->SetControlRotation((Controller->GetControlRotation() + NewShake - PreviousExplosionShake).GetNormalized());
    PreviousExplosionShake = NewShake;
    if (ExplosionShakeRemaining <= 0.0f) ExplosionShakeStrength = 0.0f;
}

void ASecondSemester_TeamCharacter::StartSprint()
{
	bSprintRequested = true;
}

void ASecondSemester_TeamCharacter::StopSprint()
{
	bSprintRequested = false;
	bIsSprinting = false;
	TimeSinceSprintStopped = 0.0f;
	GetCharacterMovement()->MaxWalkSpeed = CSHWalkSpeed;
}

void ASecondSemester_TeamCharacter::UpdateSprint(float DeltaSeconds)
{
	const bool bHasMovementInput = GetVelocity().SizeSquared2D() > 25.0f;
	const bool bCanSprint = bSprintRequested && bHasMovementInput && Stamina > 0.0f && Health > 0.0f;
	if (bCanSprint)
	{
		bIsSprinting = true;
		TimeSinceSprintStopped = 0.0f;
		GetCharacterMovement()->MaxWalkSpeed = CSHSprintSpeed;
		Stamina = FMath::Max(0.0f, Stamina - StaminaDrainPerSecond * DeltaSeconds);
		if (Stamina <= 0.0f)
		{
			bIsSprinting = false;
			bSprintRequested = false;
			GetCharacterMovement()->MaxWalkSpeed = CSHWalkSpeed;
		}
	}
	else
	{
		bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = CSHWalkSpeed;
		TimeSinceSprintStopped += DeltaSeconds;
		if (TimeSinceSprintStopped >= StaminaRecoveryDelay)
		{
			Stamina = FMath::Min(MaxStamina, Stamina + StaminaRecoveryPerSecond * DeltaSeconds);
		}
	}
}
void ASecondSemester_TeamCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ASecondSemester_TeamCharacter::StartWeaponFire);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ASecondSemester_TeamCharacter::StopWeaponFire);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &ASecondSemester_TeamCharacter::ReloadWeapon);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &ASecondSemester_TeamCharacter::StartSprint);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &ASecondSemester_TeamCharacter::StopSprint);
	PlayerInputComponent->BindKey(EKeys::Hyphen, IE_Pressed, this, &ASecondSemester_TeamCharacter::DebugTakeDamage);
	PlayerInputComponent->BindKey(EKeys::Equals, IE_Pressed, this, &ASecondSemester_TeamCharacter::DebugHeal);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASecondSemester_TeamCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASecondSemester_TeamCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASecondSemester_TeamCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASecondSemester_TeamCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ASecondSemester_TeamCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogSecondSemester_Team, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ASecondSemester_TeamCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ASecondSemester_TeamCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ASecondSemester_TeamCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ASecondSemester_TeamCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ASecondSemester_TeamCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ASecondSemester_TeamCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}


bool ASecondSemester_TeamCharacter::EquipWeapon(TSubclassOf<ACSHWeaponBase> WeaponClass)
{
    if (!WeaponClass || !GetWorld()) return false;
    StopWeaponFire();
    if (IsValid(CurrentWeapon)) CurrentWeapon->Destroy();
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CurrentWeapon = GetWorld()->SpawnActor<ACSHWeaponBase>(WeaponClass, GetActorTransform(), Params);
    if (!IsValid(CurrentWeapon)) return false;
    CurrentWeapon->EquipTo(this);
    FirstPersonMesh->SetVisibility(false, false);
    if (ASecondSemester_TeamPlayerController* CSHController = Cast<ASecondSemester_TeamPlayerController>(GetController()))
    {
        CSHController->SetWeaponHUDInfo(CurrentWeapon->GetWeaponDisplayName(), CurrentWeapon->GetWeaponDescription());
        CSHController->SetWeaponHUDVisible(true);
    }
    return true;
}

void ASecondSemester_TeamCharacter::StartWeaponFire()
{
    if (IsValid(CurrentWeapon)) CurrentWeapon->StartFiring();
}

void ASecondSemester_TeamCharacter::StopWeaponFire()
{
    if (IsValid(CurrentWeapon)) CurrentWeapon->StopFiring();
}

void ASecondSemester_TeamCharacter::ReloadWeapon()
{
    if (IsValid(CurrentWeapon)) CurrentWeapon->Reload();
}

