#include "Enemies/SS_Enemy.h"

#include "Enemies/SS_WaveManager.h"

#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "PhysicsEngine/BodyInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "CSH/SecondSemester_TeamCharacter.h"

ASS_Enemy::ASS_Enemy()
{
	PrimaryActorTick.bCanEverTick = true;
	RandomSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RandomSoundComponent"));
	RandomSoundComponent->SetupAttachment(GetRootComponent());
	RandomSoundComponent->bAutoActivate = false;
	RandomSoundComponent->bStopWhenOwnerDestroyed = true;

	EnemyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("EnemyLight"));
	EnemyLight->SetupAttachment(GetRootComponent());
	EnemyLight->SetRelativeLocation(FVector(140.0f, 0.0f, 45.0f));
	EnemyLight->SetIntensity(1200.0f);
	EnemyLight->SetAttenuationRadius(550.0f);
	EnemyLight->SetLightColor(FLinearColor(1.0f, 0.08f, 0.025f));
	EnemyLight->SetCastShadows(false);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->SetUpdateNavAgentWithOwnersCollisions(false);
	Movement->NavAgentProps.AgentRadius = NavigationAgentRadius;
	Movement->NavAgentProps.AgentHeight = NavigationAgentHeight;
	Movement->bOrientRotationToMovement = false;
	Movement->bRunPhysicsWithNoController = true;
	Movement->DefaultLandMovementMode = MOVE_Walking;
	Movement->MaxWalkSpeed = FollowSpeed;
	Movement->bUseSeparateBrakingFriction = true;
	Movement->BrakingFriction = 0.0f;
	Movement->BrakingDecelerationWalking = 0.0f;
}

void ASS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = FMath::Max(MaxHealth, 1.0f);
	bIsDead = false;
	StuckReferenceLocation = GetActorLocation();
	StuckElapsedTime = 0.0f;
	ScheduleRandomSound();
	OnActorHit.AddUniqueDynamic(this, &ASS_Enemy::HandleCarHit);
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);

	if (!IsValid(FollowTarget))
	{
		FollowTarget = UGameplayStatics::GetPlayerPawn(this, 0);
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement != nullptr)
	{
		Movement->SetUpdateNavAgentWithOwnersCollisions(false);
		Movement->NavAgentProps.AgentRadius = NavigationAgentRadius;
		Movement->NavAgentProps.AgentHeight = NavigationAgentHeight;
		Movement->MaxWalkSpeed = FollowSpeed;
		Movement->bUseSeparateBrakingFriction = true;
		Movement->BrakingFriction = 0.0f;
		Movement->BrakingDecelerationWalking = 0.0f;
		if (Movement->MovementMode == MOVE_None)
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}

	if (bUseRigidBodyPhysics && Movement != nullptr)
	{
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		if (Capsule != nullptr)
		{
			Capsule->SetCanEverAffectNavigation(false);
			if (FBodyInstance* BodyInstance = Capsule->GetBodyInstance())
			{
				BodyInstance->bLockXTranslation = false;
				BodyInstance->bLockYTranslation = false;
				BodyInstance->bLockZTranslation = false;
				BodyInstance->bLockXRotation = false;
				BodyInstance->bLockYRotation = false;
				BodyInstance->bLockZRotation = false;
				BodyInstance->SetDOFLock(EDOFMode::SixDOF);
			}

			Movement->StopMovementImmediately();
			Movement->DisableMovement();
			Movement->Deactivate();
			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Capsule->BodyInstance.bUseCCD = true;
			Capsule->SetLinearDamping(RigidBodyLinearDamping);
			Capsule->SetAngularDamping(RigidBodyAngularDamping);
			Capsule->SetEnableGravity(true);
			Capsule->SetSimulatePhysics(true);
			Capsule->WakeAllRigidBodies();

			if (UBoxComponent* PhysicsBox = FindComponentByClass<UBoxComponent>())
			{
				if (bOverrideVehicleCollisionBox)
				{
					PhysicsBox->SetRelativeScale3D(FVector::OneVector);
					const FVector SafeExtent(
						FMath::Max(VehicleCollisionBoxExtent.X, 1.0f),
						FMath::Max(VehicleCollisionBoxExtent.Y, 1.0f),
						FMath::Max(VehicleCollisionBoxExtent.Z, 1.0f));
					PhysicsBox->SetBoxExtent(SafeExtent, true);
					PhysicsBox->SetRelativeLocation(VehicleCollisionBoxOffset);
				}
				PhysicsBox->SetCanEverAffectNavigation(false);
				PhysicsBox->SetSimulatePhysics(false);
				PhysicsBox->SetCollisionProfileName(TEXT("PhysicsActor"));
				PhysicsBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				PhysicsBox->SetGenerateOverlapEvents(false);
				PhysicsBox->BodyInstance.bUseCCD = true;
				PhysicsBox->BodyInstance.bAutoWeld = true;
				PhysicsBox->WeldTo(Capsule, NAME_None, true);
				PhysicsBox->SetNotifyRigidBodyCollision(true);
			}
		}
	}

	if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		if (NavSystem->IsThereAnywhereToBuildNavigation())
		{
			NavSystem->Build();
		}
	}

	RebuildNavigationPath();
}

void ASS_Enemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RandomSoundTimer);
	RandomSoundComponent->Stop();
	Super::EndPlay(EndPlayReason);
}

void ASS_Enemy::ScheduleRandomSound()
{
	if (bIsDead || IsActorBeingDestroyed())
	{
		return;
	}
	const float MinInterval = FMath::Max(0.1f, FMath::Min(MinRandomSoundInterval, MaxRandomSoundInterval));
	const float MaxInterval = FMath::Max(MinInterval, FMath::Max(MinRandomSoundInterval, MaxRandomSoundInterval));
	GetWorldTimerManager().SetTimer(RandomSoundTimer, this, &ASS_Enemy::PlayRandomSound,
		FMath::FRandRange(MinInterval, MaxInterval), false);
}

void ASS_Enemy::PlayRandomSound()
{
	if (bIsDead || IsActorBeingDestroyed())
	{
		return;
	}
	if (!bEnableRandomSound)
	{
		RandomSoundComponent->Stop();
	}
	else if (RandomSound && !RandomSoundComponent->IsPlaying())
	{
		RandomSoundComponent->SetSound(RandomSound);
		RandomSoundComponent->SetVolumeMultiplier(FMath::Max(0.0f, RandomSoundVolume));
		RandomSoundComponent->Play();
	}
	ScheduleRandomSound();
}

void ASS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsDead)
	{
		return;
	}
	RecentDriveVelocity = GetVelocity();
	NavigationRecoveryCheckTime -= DeltaTime;
	if (NavigationRecoveryCheckTime <= 0.0f)
	{
		NavigationRecoveryCheckTime = 0.1f;
		UpdateNavigationRecovery();
	}

	if (!IsValid(FollowTarget))
	{
		FollowTarget = UGameplayStatics::GetPlayerPawn(this, 0);
		if (!IsValid(FollowTarget))
		{
			return;
		}
	}
	if (UpdateStuckTeleport(DeltaTime))
	{
		return;
	}

	const bool bPlayerWithinChargeRange = DirectChargeRadius > 0.0f
		&& FVector::DistSquared2D(GetActorLocation(), FollowTarget->GetActorLocation())
			<= FMath::Square(DirectChargeRadius);
	if (bDirectChargeWithinRange != bPlayerWithinChargeRange)
	{
		PathRecalculationTimeRemaining = 0.0f;
	}

	PathRecalculationTimeRemaining -= DeltaTime;
	if (PathRecalculationTimeRemaining <= 0.0f)
	{
		RebuildNavigationPath();
	}

	ApplyPathFollowingForce(DeltaTime);
}

bool ASS_Enemy::UpdateStuckTeleport(float DeltaTime)
{
	if (!bEnableStuckTeleport || !IsValid(FollowTarget))
	{
		StuckElapsedTime = 0.0f;
		StuckReferenceLocation = GetActorLocation();
		return false;
	}

	const bool bTargetIsFarEnough = FVector::DistSquared2D(
		GetActorLocation(), FollowTarget->GetActorLocation()) >
		FMath::Square(TargetAcceptanceRadius + StuckMovementTolerance);
	const bool bTryingToDrive = bTargetIsFarEnough
		&& FollowSpeed > UE_SMALL_NUMBER && MaxMovementForce > UE_SMALL_NUMBER;
	if (!bTryingToDrive)
	{
		StuckElapsedTime = 0.0f;
		StuckReferenceLocation = GetActorLocation();
		return false;
	}

	if (FVector::DistSquared2D(GetActorLocation(), StuckReferenceLocation) >=
		FMath::Square(FMath::Max(StuckMovementTolerance, 1.0f)))
	{
		StuckElapsedTime = 0.0f;
		StuckReferenceLocation = GetActorLocation();
		return false;
	}

	StuckElapsedTime += DeltaTime;
	if (StuckElapsedTime < FMath::Max(StuckTeleportDelay, 0.5f))
	{
		return false;
	}

	if (!TeleportToNearbyNavigation())
	{
		StuckElapsedTime = FMath::Max(StuckTeleportDelay - 1.0f, 0.0f);
		return false;
	}

	StuckElapsedTime = 0.0f;
	StuckReferenceLocation = GetActorLocation();
	NavigationPoints.Reset();
	CurrentPathPointIndex = INDEX_NONE;
	bHasValidNavigationPath = false;
	PathRecalculationTimeRemaining = 0.0f;
	StraightAccelerationAlpha = 0.0f;
	SmoothedDriveDirection = FVector::ZeroVector;
	RebuildNavigationPath();
	return true;
}

bool ASS_Enemy::TeleportToNearbyNavigation()
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!World || !NavSystem || !Movement || !Capsule)
	{
		return false;
	}

	const ANavigationData* NavData = NavSystem->GetNavDataForProps(Movement->NavAgentProps);
	if (!NavData)
	{
		NavData = NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	}
	if (!NavData)
	{
		return false;
	}

	const FVector Origin = GetActorLocation();
	FVector PreferredDirection = IsValid(FollowTarget)
		? FollowTarget->GetActorLocation() - Origin
		: GetActorForwardVector();
	PreferredDirection.Z = 0.0f;
	if (!PreferredDirection.Normalize())
	{
		PreferredDirection = GetActorForwardVector().GetSafeNormal2D();
	}

	const float MinimumDistance = FMath::Max(StuckTeleportMinimumDistance, 50.0f);
	const float SearchRadius = FMath::Max(StuckTeleportSearchRadius, MinimumDistance);
	const FVector ProjectionExtent(
		FMath::Max(NavigationAgentRadius, 100.0f),
		FMath::Max(NavigationAgentRadius, 100.0f),
		FMath::Max(NavigationAgentHeight, 300.0f));
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FRotator UprightRotation(0.0f, GetActorRotation().Yaw, 0.0f);
	constexpr int32 RingCount = 4;
	constexpr int32 DirectionCount = 12;

	for (int32 RingIndex = 0; RingIndex < RingCount; ++RingIndex)
	{
		const float RingAlpha = RingCount > 1
			? static_cast<float>(RingIndex) / static_cast<float>(RingCount - 1)
			: 1.0f;
		const float ProbeDistance = FMath::Lerp(MinimumDistance, SearchRadius, RingAlpha);
		for (int32 DirectionIndex = 0; DirectionIndex < DirectionCount; ++DirectionIndex)
		{
			const float Angle = 360.0f * static_cast<float>(DirectionIndex) /
				static_cast<float>(DirectionCount);
			const FVector ProbeDirection = PreferredDirection.RotateAngleAxis(Angle, FVector::UpVector);
			const FVector ProbeLocation = Origin + ProbeDirection * ProbeDistance;
			FNavLocation ProjectedPoint;
			if (!NavSystem->ProjectPointToNavigation(
				ProbeLocation, ProjectedPoint, ProjectionExtent, NavData))
			{
				continue;
			}
			if (FVector::DistSquared2D(Origin, ProjectedPoint.Location) <
				FMath::Square(MinimumDistance * 0.75f))
			{
				continue;
			}

			const FVector Destination = ProjectedPoint.Location +
				FVector::UpVector * (CapsuleHalfHeight + 5.0f);
			if (!TeleportTo(Destination, UprightRotation, false, false))
			{
				continue;
			}

			if (Capsule->IsSimulatingPhysics())
			{
				Capsule->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Capsule->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
				Capsule->WakeAllRigidBodies();
			}
			else
			{
				Movement->StopMovementImmediately();
			}
			if (bDrawDebugPath)
			{
				DrawDebugSphere(World, Destination, 60.0f, 16, FColor::Cyan, false, 5.0f, 0, 4.0f);
				DrawDebugLine(World, Origin, Destination, FColor::Cyan, false, 5.0f, 0, 4.0f);
			}
			return true;
		}
	}

	return false;
}

void ASS_Enemy::UpdateNavigationRecovery()
{
	bReturningToNavigation = false;
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!NavSystem || !Movement)
	{
		return;
	}
	const ANavigationData* NavData = NavSystem->GetNavDataForProps(Movement->NavAgentProps);
	if (!NavData)
	{
		return;
	}
	const FVector FeetLocation = GetActorLocation() - FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FNavLocation ProjectedPoint;
	const bool bOnNavigation = NavSystem->ProjectPointToNavigation(
		FeetLocation, ProjectedPoint, FVector(25.0f, 25.0f, 100.0f), NavData)
		&& FVector::DistSquared2D(FeetLocation, ProjectedPoint.Location) <= FMath::Square(25.0f);
	if (bEnemyOutsideNavigation != !bOnNavigation)
	{
		bEnemyOutsideNavigation = !bOnNavigation;
		PathRecalculationTimeRemaining = 0.0f;
	}
}

void ASS_Enemy::ApplyNavigationRecoveryForce()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Capsule || !Movement)
	{
		return;
	}
	FVector Offset = LastValidNavigationPoint - GetActorLocation();
	Offset.Z = 0.0f;
	const bool bPhysics = bUseRigidBodyPhysics && Capsule->IsSimulatingPhysics();
	FVector Velocity = bPhysics ? Capsule->GetPhysicsLinearVelocity() : Movement->Velocity;
	Velocity.Z = 0.0f;
	const FVector DesiredVelocity = Offset.GetSafeNormal() * FMath::Min(RecoverySpeed, Offset.Size() * 2.0f);
	const FVector Acceleration = ((DesiredVelocity - Velocity) * 4.0f).GetClampedToMaxSize(MaxMovementForce);
	if (bPhysics)
	{
		Capsule->AddForce(Acceleration, NAME_None, true);
	}
	else
	{
		Movement->AddForce(Acceleration * Movement->Mass);
	}
	if (bDrawDebugPath)
	{
		DrawDebugSphere(GetWorld(), LastValidNavigationPoint, 35.0f, 12, FColor::Orange, false);
		DrawDebugLine(GetWorld(), GetActorLocation(), LastValidNavigationPoint, FColor::Orange, false);
	}
}

void ASS_Enemy::HandleCarHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ASecondSemester_TeamCharacter* Player = Cast<ASecondSemester_TeamCharacter>(OtherActor))
	{
		FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
		ToPlayer.Z = 0.0f;
		if (!ToPlayer.Normalize())
		{
			ToPlayer = GetRecentDriveVelocity().GetSafeNormal2D();
		}
		const FVector PlayerVelocity = Player->GetVelocity();
		const float CurrentClosingSpeed = FVector::DotProduct(
			GetVelocity() - PlayerVelocity, ToPlayer);
		const float RecentClosingSpeed = FVector::DotProduct(
			GetRecentDriveVelocity() - PlayerVelocity, ToPlayer);
		const float ClosingSpeed = FMath::Max(0.0f,
			FMath::Max(CurrentClosingSpeed, RecentClosingSpeed));
		if (!bIsDead && HitSound && GetWorld()
			&& ClosingSpeed >= MinimumCollisionDamageSpeed)
		{
			const double Now = GetWorld()->GetTimeSeconds();
			if (Now >= NextPlayerImpactSoundTime)
			{
				NextPlayerImpactSoundTime = Now + FMath::Max(PlayerImpactSoundCooldown, 0.0f);
				const FVector SoundLocation = Hit.bBlockingHit
					? FVector(Hit.ImpactPoint)
					: Player->GetActorLocation();
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, SoundLocation);
			}
		}
		Player->ApplyEnemyCarImpact(this);
		return;
	}
	ASS_Enemy* OtherCar = Cast<ASS_Enemy>(OtherActor);
	if (bIsDead || !IsValid(OtherCar) || OtherCar == this || OtherCar->bIsDead)
	{
		return;
	}
	FVector ToOther = OtherCar->GetActorLocation() - GetActorLocation();
	ToOther.Z = 0.0f;
	if (!ToOther.Normalize())
	{
		ToOther = GetRecentDriveVelocity().GetSafeNormal2D();
	}
	const float CurrentClosingSpeed = FVector::DotProduct(
		GetVelocity() - OtherCar->GetVelocity(), ToOther);
	const float RecentClosingSpeed = FVector::DotProduct(
		GetRecentDriveVelocity() - OtherCar->GetRecentDriveVelocity(), ToOther);
	const float ClosingSpeed = FMath::Max(0.0f,
		FMath::Max(CurrentClosingSpeed, RecentClosingSpeed));
	ApplyCollisionDamage(Hit.ImpactPoint, ClosingSpeed);
	if (IsValid(OtherCar))
	{
		OtherCar->ApplyCollisionDamage(Hit.ImpactPoint, ClosingSpeed);
	}
}

void ASS_Enemy::ApplyCollisionDamage(FVector HitLocation, float ClosingSpeed)
{
	if (bIsDead || !GetWorld() || CollisionDamage <= 0.0f
		|| ClosingSpeed < MinimumCollisionDamageSpeed)
	{
		return;
	}
	const float Damage = FMath::Clamp(
		ClosingSpeed / FMath::Max(CollisionDamageReferenceSpeed, 1.0f)
			* CollisionDamage, 0.0f, MaximumCollisionDamage);
	if (Damage <= 0.0f)
	{
		return;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextCollisionDamageTime)
	{
		return;
	}
	NextCollisionDamageTime = Now + FMath::Max(CollisionDamageCooldown, 0.05f);
	ApplyCarDamage(Damage, HitLocation);
}

float ASS_Enemy::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || IsActorBeingDestroyed() || !CanBeDamaged()
		|| !FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.0f || bIsDead || IsActorBeingDestroyed())
	{
		return 0.0f;
	}
	FVector HitLocation = GetActorLocation();
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		HitLocation = static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.ImpactPoint;
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		const FRadialDamageEvent& RadialEvent = static_cast<const FRadialDamageEvent&>(DamageEvent);
		if (!RadialEvent.ComponentHits.IsEmpty())
		{
			HitLocation = RadialEvent.ComponentHits[0].ImpactPoint;
		}
	}
	const float HealthDamage = FMath::Min(ActualDamage, CurrentHealth);
	ApplyCarDamage(ActualDamage, HitLocation);
	return HealthDamage;
}

void ASS_Enemy::ApplyCarDamage(float Damage, FVector HitLocation)
{
	if (bIsDead || IsActorBeingDestroyed() || !GetWorld() || !FMath::IsFinite(Damage) || Damage <= 0.0f)
	{
		return;
	}
	const float AppliedDamage = FMath::Min(Damage, CurrentHealth);
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);
	bIsDead = CurrentHealth <= 0.0f;
	const bool bFatalDamage = bIsDead;
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HitEffect, HitLocation);
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitLocation);
	}
	OnCarDamaged(AppliedDamage, HitLocation);
	if (!bFatalDamage || IsActorBeingDestroyed())
	{
		return;
	}
	SetActorTickEnabled(false);
	GetWorldTimerManager().ClearTimer(RandomSoundTimer);
	RandomSoundComponent->Stop();
	SetActorEnableCollision(false);
	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionEffect, GetActorLocation(), GetActorRotation());
	}
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}
	OnCarExploded();
	Destroy();
}

void ASS_Enemy::MarkCapturedAsDefeated()
{
	if (bIsDead || IsActorBeingDestroyed())
	{
		return;
	}

	CurrentHealth = 0.0f;
	bIsDead = true;
	SetCanBeDamaged(false);
	SetActorTickEnabled(false);
	GetWorldTimerManager().ClearTimer(RandomSoundTimer);
	if (RandomSoundComponent)
	{
		RandomSoundComponent->Stop();
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ASS_WaveManager> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				It->NotifyEnemyDied(this);
			}
		}
	}
}

void ASS_Enemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASS_Enemy::SetFollowTarget(AActor* NewTarget)
{
	bDirectlyFollowingOffNavTarget = false;
	bDirectChargeWithinRange = false;
	FollowTarget = IsValid(NewTarget) ? NewTarget : UGameplayStatics::GetPlayerPawn(this, 0);
	PathRecalculationTimeRemaining = 0.0f;
	NavigationPoints.Reset();
	CurrentPathPointIndex = INDEX_NONE;
	bHasValidNavigationPath = false;
	StraightAccelerationAlpha = 0.0f;
	SmoothedDriveDirection = FVector::ZeroVector;
}

void ASS_Enemy::RebuildNavigationPath()
{
	bDirectlyFollowingOffNavTarget = false;
	bDirectChargeWithinRange = false;
	PathRecalculationTimeRemaining = PathRecalculationInterval;
	NavigationPoints.Reset();
	CurrentPathPointIndex = INDEX_NONE;
	bHasValidNavigationPath = false;

	if (!IsValid(FollowTarget) || GetWorld() == nullptr)
	{
		return;
	}
	if (DirectChargeRadius > 0.0f
		&& FVector::DistSquared2D(GetActorLocation(), FollowTarget->GetActorLocation())
			<= FMath::Square(DirectChargeRadius))
	{
		bDirectChargeWithinRange = true;
		bDirectlyFollowingOffNavTarget = true;
		return;
	}
	if (bEnemyOutsideNavigation)
	{
		bDirectlyFollowingOffNavTarget = true;
		return;
	}

	AActor* PathfindingContext = this;
	if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			if (ANavigationData* VehicleNavData = NavSystem->GetNavDataForProps(Movement->NavAgentProps))
			{
				PathfindingContext = VehicleNavData;
				FVector TargetFeet = FollowTarget->GetActorLocation();
				if (const ACharacter* TargetCharacter = Cast<ACharacter>(FollowTarget))
				{
					TargetFeet.Z -= TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				}
				else
				{
					FVector Origin, Extent;
					FollowTarget->GetActorBounds(true, Origin, Extent);
					TargetFeet = Origin - FVector::UpVector * Extent.Z;
				}
				FNavLocation ProjectedTarget;
				const bool bTargetOnNavigation = NavSystem->ProjectPointToNavigation(
					TargetFeet, ProjectedTarget, FVector(25.0f, 25.0f, 100.0f), VehicleNavData)
					&& FVector::DistSquared2D(TargetFeet, ProjectedTarget.Location) <= FMath::Square(25.0f);
				if (!bTargetOnNavigation)
				{
					bDirectlyFollowingOffNavTarget = true;
					return;
				}
			}
		}
	}

	UNavigationPath* Path = UNavigationSystemV1::FindPathToActorSynchronously(
		GetWorld(), GetActorLocation(), FollowTarget, 50.0f, PathfindingContext);

	if (!IsValid(Path) || !Path->IsValid() || Path->PathPoints.Num() < 2)
	{
		return;
	}

	NavigationPoints.Reserve(Path->PathPoints.Num());
	for (const FNavPathPoint& PathPoint : Path->PathPoints)
	{
		NavigationPoints.Add(PathPoint.Location);
	}

	CurrentPathPointIndex = 1;
	bHasValidNavigationPath = true;

	if (bDrawDebugPath)
	{
		for (int32 PointIndex = 0; PointIndex < NavigationPoints.Num(); ++PointIndex)
		{
			DrawDebugSphere(GetWorld(), NavigationPoints[PointIndex], 24.0f, 8, FColor::Green,
				false, PathRecalculationInterval, 0, 3.0f);
			if (PointIndex > 0)
			{
				DrawDebugLine(GetWorld(), NavigationPoints[PointIndex - 1], NavigationPoints[PointIndex],
					FColor::Green, false, PathRecalculationInterval, 0, 3.0f);
			}
		}
	}
}

void ASS_Enemy::ApplyPathFollowingForce(float DeltaTime)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement == nullptr || !IsValid(FollowTarget))
	{
		return;
	}

	const FVector ActorLocation = GetActorLocation();
	if (!bDirectlyFollowingOffNavTarget
		&& FVector::DistSquared2D(ActorLocation, FollowTarget->GetActorLocation()) <=
		FMath::Square(TargetAcceptanceRadius))
	{
		ApplyStoppingForce();
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	const bool bRigidBodyActive = bUseRigidBodyPhysics && Capsule != nullptr && Capsule->IsSimulatingPhysics();
	const FVector RawVelocity = bRigidBodyActive
		? Capsule->GetPhysicsLinearVelocity()
		: Movement->Velocity;
	const FVector CurrentPlanarVelocity(RawVelocity.X, RawVelocity.Y, 0.0f);
	const float DynamicAcceptanceRadius = PathPointAcceptanceRadius +
		FMath::Min(CurrentPlanarVelocity.Size() * PathLookAheadTime, 300.0f);

	while (!bDirectlyFollowingOffNavTarget && NavigationPoints.IsValidIndex(CurrentPathPointIndex) &&
		FVector::DistSquared2D(ActorLocation, NavigationPoints[CurrentPathPointIndex]) <=
			FMath::Square(DynamicAcceptanceRadius))
	{
		++CurrentPathPointIndex;
	}

	if (!bDirectlyFollowingOffNavTarget && !NavigationPoints.IsValidIndex(CurrentPathPointIndex))
	{
		ApplyStoppingForce();
		return;
	}

	FVector SteeringTarget = FollowTarget->GetActorLocation();
	if (!bDirectlyFollowingOffNavTarget)
	{
		SteeringTarget = NavigationPoints[CurrentPathPointIndex];
		float RemainingLookAhead = FMath::Max(PathSteeringLookAheadDistance, 0.0f);
		FVector SegmentStart = ActorLocation;
		SegmentStart.Z = SteeringTarget.Z;
		for (int32 PointIndex = CurrentPathPointIndex;
			PointIndex < NavigationPoints.Num() && RemainingLookAhead > UE_SMALL_NUMBER;
			++PointIndex)
		{
			const FVector SegmentEnd = NavigationPoints[PointIndex];
			const float SegmentLength = FVector::Dist2D(SegmentStart, SegmentEnd);
			if (SegmentLength >= RemainingLookAhead && SegmentLength > UE_SMALL_NUMBER)
			{
				const float SegmentAlpha = RemainingLookAhead / SegmentLength;
				SteeringTarget = FMath::Lerp(SegmentStart, SegmentEnd, SegmentAlpha);
				break;
			}

			SteeringTarget = SegmentEnd;
			RemainingLookAhead -= SegmentLength;
			SegmentStart = SegmentEnd;
		}
	}
	if (bDrawDebugPath)
	{
		DrawDebugLine(GetWorld(), ActorLocation, SteeringTarget,
			FColor::Yellow, false, 0.0f, 0, 5.0f);
	}

	FVector Direction = SteeringTarget - ActorLocation;
	Direction.Z = 0.0f;
	if (!Direction.Normalize())
	{
		return;
	}
	if (SmoothedDriveDirection.IsNearlyZero())
	{
		SmoothedDriveDirection = Direction;
	}
	else if (SteeringResponseSpeed > 0.0f)
	{
		const FVector BlendedDirection = FMath::VInterpTo(
			SmoothedDriveDirection, Direction, DeltaTime, SteeringResponseSpeed);
		SmoothedDriveDirection = BlendedDirection.GetSafeNormal2D();
	}
	else
	{
		SmoothedDriveDirection = Direction;
	}
	if (!SmoothedDriveDirection.IsNearlyZero())
	{
		Direction = SmoothedDriveDirection;
	}

	const float ForwardSpeed = FVector::DotProduct(CurrentPlanarVelocity, Direction);
	const float DirectionAlignment = CurrentPlanarVelocity.SizeSquared() > FMath::Square(25.0f)
		? FVector::DotProduct(CurrentPlanarVelocity.GetSafeNormal(), Direction)
		: 1.0f;
	const bool bDrivingStraight = DirectionAlignment >= 0.94f;
	const float AccelerationBuildRate = 1.0f / FMath::Max(StraightAccelerationTime, 0.05f);
	const float AccelerationLossRate = AccelerationBuildRate * 3.0f;
	StraightAccelerationAlpha = FMath::Clamp(
		StraightAccelerationAlpha + (bDrivingStraight ? AccelerationBuildRate : -AccelerationLossRate) * DeltaTime,
		0.0f,
		1.0f);
	const float DriveForceRatio = FMath::Lerp(
		MinimumDriveForceRatio, 1.0f, StraightAccelerationAlpha);
	const float Throttle = FollowSpeed > UE_SMALL_NUMBER
		? FMath::Clamp(1.0f - ForwardSpeed / FollowSpeed, 0.0f, 1.0f)
		: 0.0f;

	bool bClimbableCurbAhead = false;
	if (bRigidBodyActive && CurbProbeDistance > 0.0f && MaximumClimbableCurbHeight > 0.0f)
	{
		const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		const float ProbeRadius = 18.0f;
		const float FloorZ = ActorLocation.Z - CapsuleHalfHeight;
		const FVector LowProbeStart(ActorLocation.X, ActorLocation.Y, FloorZ + ProbeRadius + 4.0f);
		const FVector HighProbeStart(
			ActorLocation.X, ActorLocation.Y, FloorZ + MaximumClimbableCurbHeight + ProbeRadius);
		const FVector ProbeOffset = Direction * CurbProbeDistance;
		FCollisionQueryParams ProbeParams(SCENE_QUERY_STAT(SS_EnemyCurbProbe), false, this);
		ProbeParams.AddIgnoredActor(this);
		FHitResult LowHit;
		FHitResult HighHit;
		const FCollisionShape ProbeShape = FCollisionShape::MakeSphere(ProbeRadius);
		const bool bLowBlocked = GetWorld()->SweepSingleByChannel(
			LowHit, LowProbeStart, LowProbeStart + ProbeOffset, FQuat::Identity,
			ECC_WorldStatic, ProbeShape, ProbeParams);
		const bool bHighBlocked = GetWorld()->SweepSingleByChannel(
			HighHit, HighProbeStart, HighProbeStart + ProbeOffset, FQuat::Identity,
			ECC_WorldStatic, ProbeShape, ProbeParams);
		bClimbableCurbAhead = bLowBlocked && !bHighBlocked;
	}

	const float CurbForceMultiplier = bClimbableCurbAhead
		? FMath::Max(CurbDriveForceMultiplier, 1.0f)
		: 1.0f;
	const float ChargeForceMultiplier = bDirectChargeWithinRange
		? FMath::Max(DirectChargeForceMultiplier, 1.0f)
		: 1.0f;
	const float EffectiveMaxMovementForce =
		MaxMovementForce * CurbForceMultiplier * ChargeForceMultiplier;
	const FVector DriveAcceleration = Direction * EffectiveMaxMovementForce * DriveForceRatio * Throttle;
	const FVector DragAcceleration = -CurrentPlanarVelocity * SteeringGain;
	FVector AppliedAcceleration =
		(DriveAcceleration + DragAcceleration).GetClampedToMaxSize(EffectiveMaxMovementForce);
	if (bClimbableCurbAhead)
	{
		AppliedAcceleration.Z += CurbUpwardAcceleration * Throttle;
	}
	if (bRigidBodyActive)
	{
		Capsule->AddForce(AppliedAcceleration, NAME_None, true);
	}
	else
	{
		Movement->AddForce(AppliedAcceleration * Movement->Mass);
	}

	if (bUseMovementFacingTorque && CurrentPlanarVelocity.SizeSquared() > FMath::Square(5.0f))
	{
		UpdateFacingRotation(DeltaTime, CurrentPlanarVelocity.GetSafeNormal());
	}
}

void ASS_Enemy::ApplyStoppingForce()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement == nullptr)
	{
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	const bool bRigidBodyActive = bUseRigidBodyPhysics && Capsule != nullptr && Capsule->IsSimulatingPhysics();
	const FVector RawVelocity = bRigidBodyActive
		? Capsule->GetPhysicsLinearVelocity()
		: Movement->Velocity;
	const FVector PlanarVelocity(RawVelocity.X, RawVelocity.Y, 0.0f);
	StraightAccelerationAlpha = 0.0f;
	FVector BrakingAcceleration = -PlanarVelocity * FMath::Max(SteeringGain, 3.0f);
	BrakingAcceleration = BrakingAcceleration.GetClampedToMaxSize(MaxMovementForce);
	if (bRigidBodyActive)
	{
		Capsule->AddForce(BrakingAcceleration, NAME_None, true);
	}
	else
	{
		Movement->AddForce(BrakingAcceleration * Movement->Mass);
	}
}

void ASS_Enemy::UpdateFacingRotation(float DeltaTime, const FVector& MovementDirection)
{
	if (MovementDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const float DesiredYaw = MovementDirection.Rotation().Yaw;
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (bUseRigidBodyPhysics && Capsule != nullptr && Capsule->IsSimulatingPhysics())
	{
		const float YawErrorRadians = FMath::DegreesToRadians(
			FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredYaw));
		const float MaxYawSpeedRadians = FMath::DegreesToRadians(MaxAngularSpeed);
		const float DesiredYawSpeed = FMath::Clamp(
			YawErrorRadians * RotationInterpSpeed,
			-MaxYawSpeedRadians,
			MaxYawSpeedRadians);
		const float CurrentYawSpeed = Capsule->GetPhysicsAngularVelocityInRadians().Z;
		const float YawAcceleration = FMath::Clamp(
			(DesiredYawSpeed - CurrentYawSpeed) * 4.0f,
			-MaxYawAngularAcceleration,
			MaxYawAngularAcceleration);
		Capsule->AddTorqueInRadians(FVector::UpVector * YawAcceleration, NAME_None, true);
		return;
	}

	const float InterpolatedYaw = FMath::FInterpTo(
		CurrentRotation.Yaw, DesiredYaw, DeltaTime, RotationInterpSpeed);
	const float YawStep = FMath::Clamp(
		FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, InterpolatedYaw),
		-MaxAngularSpeed * DeltaTime,
		MaxAngularSpeed * DeltaTime);

	SetActorRotation(FRotator(0.0f, CurrentRotation.Yaw + YawStep, 0.0f));
}
