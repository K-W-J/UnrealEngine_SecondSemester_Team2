#include "Enemies/SS_Enemy.h"

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

ASS_Enemy::ASS_Enemy()
{
	PrimaryActorTick.bCanEverTick = true;

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

			// Weld the Blueprint box into the simulated root body so impacts on the
			// box generate physical torque instead of requiring scripted rotation.
			if (UBoxComponent* PhysicsBox = FindComponentByClass<UBoxComponent>())
			{
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

void ASS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsDead)
	{
		return;
	}

	if (!IsValid(FollowTarget))
	{
		FollowTarget = UGameplayStatics::GetPlayerPawn(this, 0);
		if (!IsValid(FollowTarget))
		{
			return;
		}
	}

	PathRecalculationTimeRemaining -= DeltaTime;
	if (PathRecalculationTimeRemaining <= 0.0f)
	{
		RebuildNavigationPath();
	}

	ApplyPathFollowingForce(DeltaTime);
}

void ASS_Enemy::HandleCarHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	ASS_Enemy* OtherCar = Cast<ASS_Enemy>(OtherActor);
	if (bIsDead || !IsValid(OtherCar) || OtherCar == this || OtherCar->bIsDead)
	{
		return;
	}
	// Damage both cars; cooldown prevents duplicate callbacks from doubling damage.
	ApplyCollisionDamage(Hit.ImpactPoint);
	if (IsValid(OtherCar))
	{
		OtherCar->ApplyCollisionDamage(Hit.ImpactPoint);
	}
}

void ASS_Enemy::ApplyCollisionDamage(FVector HitLocation)
{
	if (bIsDead || !GetWorld() || CollisionDamage <= 0.0f)
	{
		return;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextCollisionDamageTime)
	{
		return;
	}
	NextCollisionDamageTime = Now + FMath::Max(CollisionDamageCooldown, 0.05f);
	ApplyCarDamage(CollisionDamage, HitLocation);
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

void ASS_Enemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASS_Enemy::SetFollowTarget(AActor* NewTarget)
{
	FollowTarget = IsValid(NewTarget) ? NewTarget : UGameplayStatics::GetPlayerPawn(this, 0);
	PathRecalculationTimeRemaining = 0.0f;
	NavigationPoints.Reset();
	CurrentPathPointIndex = INDEX_NONE;
	bHasValidNavigationPath = false;
	StraightAccelerationAlpha = 0.0f;
}

void ASS_Enemy::RebuildNavigationPath()
{
	PathRecalculationTimeRemaining = PathRecalculationInterval;
	NavigationPoints.Reset();
	CurrentPathPointIndex = INDEX_NONE;
	bHasValidNavigationPath = false;

	if (!IsValid(FollowTarget) || GetWorld() == nullptr)
	{
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
	if (FVector::DistSquared2D(ActorLocation, FollowTarget->GetActorLocation()) <=
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

	while (NavigationPoints.IsValidIndex(CurrentPathPointIndex) &&
		FVector::DistSquared2D(ActorLocation, NavigationPoints[CurrentPathPointIndex]) <=
			FMath::Square(DynamicAcceptanceRadius))
	{
		++CurrentPathPointIndex;
	}

	if (!NavigationPoints.IsValidIndex(CurrentPathPointIndex))
	{
		ApplyStoppingForce();
		return;
	}

	if (bDrawDebugPath)
	{
		DrawDebugLine(GetWorld(), ActorLocation, NavigationPoints[CurrentPathPointIndex],
			FColor::Yellow, false, 0.0f, 0, 5.0f);
	}

	FVector Direction = NavigationPoints[CurrentPathPointIndex] - ActorLocation;
	Direction.Z = 0.0f;
	if (!Direction.Normalize())
	{
		return;
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
	const float EffectiveMaxMovementForce = MaxMovementForce * CurbForceMultiplier;
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
