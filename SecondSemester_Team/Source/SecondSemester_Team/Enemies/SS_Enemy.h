#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SS_Enemy.generated.h"

class AActor;
class UNiagaraSystem;
class USoundBase;
class UAudioComponent;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ASS_Enemy : public ACharacter
{
	GENERATED_BODY()

public:
	ASS_Enemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Audio")
	bool bEnableRandomSound = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Audio")
	TObjectPtr<USoundBase> RandomSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Audio", meta = (ClampMin = "0.1", Units = "s"))
	float MinRandomSoundInterval = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Audio", meta = (ClampMin = "0.1", Units = "s"))
	float MaxRandomSoundInterval = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Audio", meta = (ClampMin = "0.0"))
	float RandomSoundVolume = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SS Enemy Audio")
	TObjectPtr<UAudioComponent> RandomSoundComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Combat", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SS Enemy Combat")
	float CurrentHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SS Enemy Combat")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Combat", meta = (ClampMin = "0.0"))
	float CollisionDamage = 5.0f;

	/** Minimum seconds between collision damage ticks on this car. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Combat", meta = (ClampMin = "0.05", Units = "s"))
	float CollisionDamageCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Combat|Effects")
	TObjectPtr<UNiagaraSystem> HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Combat|Effects")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Combat|Effects")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Combat|Effects")
	TObjectPtr<USoundBase> ExplosionSound;

	UFUNCTION(BlueprintCallable, Category = "SS Enemy Combat")
	void ApplyCarDamage(float Damage, FVector HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "SS Enemy Combat")
	void OnCarDamaged(float Damage, FVector HitLocation);

	/** Called before destruction; spawn additional debris here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SS Enemy Combat")
	void OnCarExploded();

	/** Changes the actor pursued by this enemy. Passing nullptr restores player tracking. */
	UFUNCTION(BlueprintCallable, Category = "SS Enemy Movement")
	void SetFollowTarget(AActor* NewTarget);

	/** Maximum mass-independent acceleration toward the current navigation point in cm/s squared. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement", meta = (ClampMin = "0.0", DisplayName = "Max Drive Acceleration"))
	float MaxMovementForce = 2400.0f;

	/** Desired planar speed while following the path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement", meta = (ClampMin = "0.0"))
	float FollowSpeed = 2000.0f;

	/** Natural planar drag. Lower values preserve more momentum while turning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics", meta = (ClampMin = "0.0", DisplayName = "Linear Drag"))
	float SteeringGain = 0.08f;

	/** Seconds of mostly straight driving required to reach full engine force. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics", meta = (ClampMin = "0.05"))
	float StraightAccelerationTime = 2.5f;

	/** Fraction of engine force available from a standstill or immediately after a sharp turn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumDriveForceRatio = 0.35f;

	/** Multiplies forward force while the low obstacle probe detects a climbable curb. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics|Curb Climb", meta = (ClampMin = "1.0"))
	float CurbDriveForceMultiplier = 3.0f;

	/** Upward mass-independent acceleration used to lift the vehicle over a curb. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics|Curb Climb", meta = (ClampMin = "0.0"))
	float CurbUpwardAcceleration = 1800.0f;

	/** Distance ahead of the vehicle checked for a curb. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics|Curb Climb", meta = (ClampMin = "0.0"))
	float CurbProbeDistance = 340.0f;

	/** Obstacles lower than this height are treated as climbable curbs instead of walls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics|Curb Climb", meta = (ClampMin = "1.0"))
	float MaximumClimbableCurbHeight = 85.0f;

	/** Uses the root capsule as a freely rotating rigid body instead of kinematic character movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Rigid Body")
	bool bUseRigidBodyPhysics = true;

	/** Linear damping applied by Chaos to the physical capsule. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Rigid Body", meta = (ClampMin = "0.0"))
	float RigidBodyLinearDamping = 0.15f;

	/** Angular damping applied by Chaos. Lower values allow more collision-driven spinning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Rigid Body", meta = (ClampMin = "0.0"))
	float RigidBodyAngularDamping = 0.8f;

	/** Maximum yaw angular acceleration used to face the physical movement direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Rigid Body", meta = (ClampMin = "0.0"))
	float MaxYawAngularAcceleration = 12.0f;

	/** Applies steering torque toward movement direction. Disabled uses only collision and inertia for rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Rigid Body")
	bool bUseMovementFacingTorque = false;

	/** Extra distance used to change to the following path point before reaching the current one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics", meta = (ClampMin = "0.0"))
	float PathLookAheadTime = 0.35f;

	/** Distance at which a navigation point is considered reached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement", meta = (ClampMin = "1.0"))
	float PathPointAcceptanceRadius = 110.0f;

	/** Distance from the target at which the enemy stops pushing forward. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement", meta = (ClampMin = "0.0"))
	float TargetAcceptanceRadius = 140.0f;

	/** Seconds between path updates while the target is moving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement", meta = (ClampMin = "0.05"))
	float PathRecalculationInterval = 0.35f;

	/** How quickly the actor's forward direction aligns with its current movement direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics", meta = (ClampMin = "0.0", DisplayName = "Movement Facing Speed"))
	float RotationInterpSpeed = 8.0f;

	/** Maximum turn rate toward the current movement direction in degrees per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Physics", meta = (ClampMin = "0.0"))
	float MaxAngularSpeed = 240.0f;

	/** Draws the generated navigation points and the currently followed segment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement")
	bool bDrawDebugPath = false;

	/** True while a usable NavMesh path to the target exists. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SS Enemy Movement")
	bool bHasValidNavigationPath = false;

	/** True when the target is outside the vehicle NavMesh and is pursued directly. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SS Enemy Movement|Navigation")
	bool bDirectlyFollowingOffNavTarget = false;

	/** Circular navigation footprint that contains the Box even while the car is rotating. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Navigation", meta = (ClampMin = "1.0"))
	float NavigationAgentRadius = 120.0f;

	/** Total vehicle height used when selecting compatible navigation data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Navigation", meta = (ClampMin = "1.0"))
	float NavigationAgentHeight = 170.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Navigation Recovery")
	bool bEnableNavigationRecovery = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Navigation Recovery", meta = (ClampMin = "1.0"))
	float RecoverySpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SS Enemy Movement|Navigation Recovery", meta = (ClampMin = "1.0"))
	float RecoveryAcceptanceRadius = 50.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SS Enemy Movement|Navigation Recovery")
	FVector LastValidNavigationPoint = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SS Enemy Movement|Navigation Recovery")
	bool bHasSavedNavigationPoint = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SS Enemy Movement|Navigation Recovery")
	bool bReturningToNavigation = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void ScheduleRandomSound();
	void PlayRandomSound();
	FTimerHandle RandomSoundTimer;
	void UpdateNavigationRecovery();
	void ApplyNavigationRecoveryForce();
	float NavigationRecoveryCheckTime = 0.0f;
	UFUNCTION()
	void HandleCarHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
	void ApplyCollisionDamage(FVector HitLocation);
	double NextCollisionDamageTime = 0.0;
	void RebuildNavigationPath();
	void ApplyPathFollowingForce(float DeltaTime);
	void ApplyStoppingForce();
	void UpdateFacingRotation(float DeltaTime, const FVector& MovementDirection);

	UPROPERTY(Transient)
	TObjectPtr<AActor> FollowTarget;

	TArray<FVector> NavigationPoints;
	int32 CurrentPathPointIndex = INDEX_NONE;
	float PathRecalculationTimeRemaining = 0.0f;
	float StraightAccelerationAlpha = 0.0f;
};
