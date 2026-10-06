// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SecondSemester_TeamCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class USceneComponent;
class USpringArmComponent;
class UInputAction;
class ACSHWeaponBase;
class ACSHFists;
class ASS_Enemy;
class UCSHDamageBorderWidget;
class UCSHGameOverWidget;
class UCSHStartMenuWidget;
class ACameraActor;
class ASS_WaveManager;
class USoundBase;
class UMeshComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class ASecondSemester_TeamCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CSHDeathSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CSHDeathCamera;

protected:

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;
	
public:
	ASecondSemester_TeamCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	void UpdateBodyVisibility(bool bFirstPerson);
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void ApplyEnemyCarImpact(ASS_Enemy* Enemy);

	void FinishTitleScreen();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0", Units="cm/s", DisplayName="Horizontal Launch at Reference Speed"))
	float EnemyImpactHorizontalSpeed = 1100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0", Units="cm/s", DisplayName="Upward Launch at Reference Speed"))
	float EnemyImpactUpwardSpeed = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="1", Units="cm/s"))
	float EnemyImpactReferenceSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0", Units="cm/s"))
	float MaximumEnemyImpactHorizontalSpeed = 3200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0", Units="cm/s"))
	float MaximumEnemyImpactUpwardSpeed = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0", Units="s"))
	float EnemyImpactCooldown = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0"))
	float EnemyImpactDamagePer100Speed = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0", Units="cm/s"))
	float MinimumEnemyImpactDamageSpeed = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Enemy Impact", meta=(ClampMin="0"))
	float MaximumEnemyImpactDamage = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Border", meta=(ClampMin="0.05", Units="s"))
	float DamageBorderDuration = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Border")
	FLinearColor DamageBorderColor = FLinearColor(1.0f, 0.025f, 0.025f, 1.0f);

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Player|Damage Border")
	TObjectPtr<UCSHDamageBorderWidget> DamageBorderWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Audio")
	TArray<TObjectPtr<USoundBase>> HurtSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Audio", meta=(ClampMin="0.0", ClampMax="2.0"))
	float HurtSoundVolume = 0.75f;

	UFUNCTION(BlueprintCallable, Category="Player|Health")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool EquipWeapon(TSubclassOf<ACSHWeaponBase> WeaponClass);

	UFUNCTION(Exec, BlueprintCallable, Category="Cheat")
	void CheatKillAllEnemies();

	UFUNCTION(Exec, BlueprintCallable, Category="Cheat")
	void CheatEquipRandomWeapon();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cheat")
	TArray<TSoftClassPtr<ACSHWeaponBase>> CheatWeaponClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<ACSHWeaponBase> UnarmedWeaponClass;

    UFUNCTION(BlueprintCallable, Category="Player|Camera")
    void ApplyExplosionCameraShake(const FVector& ExplosionLocation, float InnerRadius, float OuterRadius);

protected:
	UFUNCTION()
	void OnEnemyCarActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnEnemyCarCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);


	void MoveInput(const FInputActionValue& Value);

	void LookInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

protected:

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	void StartWeaponFire();
	void StopWeaponFire();
	void ReloadWeapon();
	bool EquipUnarmedWeapon();
	void StartSprint();
	void StopSprint();
	void UpdateSprint(float DeltaSeconds);
	void ApplyDamageCameraKick(float DamageAmount, const AActor* DamageCauser);
	void UpdateDamageCameraKick(float DeltaSeconds);
	void UpdateDamageBorder(float DeltaSeconds);
	void EnsureDamageBorderWidget();
	void ShowGameOverWidget();
	void ShowStartMenuWidget();
	void SetGameplayUIVisible(bool bVisible);
	void UpdateExplosionCameraShake(float DeltaSeconds);
	void DebugTakeDamage();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Audio") TObjectPtr<USoundBase> HurtSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Audio") TObjectPtr<USoundBase> HeavyHurtSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Audio", meta=(ClampMin="0")) float HeavyHurtThreshold = 25.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Audio", meta=(ClampMin="0")) float HurtSoundCooldown = 0.15f;
    float NextHurtSoundTime = -1.f;
	void DebugHeal();
	void EnterDeathState();
	void CacheBodyMeshes();
	ASS_WaveManager* GetWaveManager();
	

public:

	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	USceneComponent* GetWeaponSocketComponent() const { return WeaponSocket; }

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
	ACSHWeaponBase* CurrentWeapon = nullptr;

	UFUNCTION(BlueprintPure, Category="Player|Health")
	float GetHealth() const { return Health; }
	UFUNCTION(BlueprintPure, Category="Player|Health")
	float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintPure, Category="Player|Stamina")
	float GetStamina() const { return Stamina; }
	UFUNCTION(BlueprintPure, Category="Player|Stamina")
	float GetMaxStamina() const { return MaxStamina; }
	UFUNCTION(BlueprintPure, Category="Player|Movement")
	bool IsSprinting() const { return bIsSprinting; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Health", meta=(ClampMin="1"))
	float MaxHealth = 100.0f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Player|Health")
	float Health = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Stamina", meta=(ClampMin="1"))
	float MaxStamina = 150.0f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Player|Stamina")
	float Stamina = 150.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Movement", meta=(ClampMin="0", Units="cm/s"))
	float CSHWalkSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Movement", meta=(ClampMin="0", Units="cm/s"))
	float CSHSprintSpeed = 1100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Stamina", meta=(ClampMin="0"))
	float StaminaDrainPerSecond = 25.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Stamina", meta=(ClampMin="0"))
	float StaminaRecoveryPerSecond = 18.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Stamina", meta=(ClampMin="0", Units="s"))
	float StaminaRecoveryDelay = 0.75f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Camera", meta=(ClampMin="0"))
	float MaxDamageKickPitch = 18.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Camera", meta=(ClampMin="0"))
	float MaxDamageKickYaw = 14.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Camera", meta=(ClampMin="0"))
	float MaxDamageKickRoll = 10.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Damage Camera", meta=(ClampMin="0.1"))
	float DamageKickRecoverySpeed = 5.0f;

private:
	double NextEnemyImpactTime = 0.0;
	float DamageBorderTimeRemaining = 0.0f;
	bool bSprintRequested = false;
	bool bIsSprinting = false;
	float TimeSinceSprintStopped = 0.0f;
	FRotator DamageCameraKick = FRotator::ZeroRotator;
	bool bDeathStateEntered = false;
	bool bBodyVisibilityInitialized = false;
	bool bBodyHiddenForFirstPerson = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMeshComponent>> CachedBodyMeshes;

	UPROPERTY(Transient)
	TObjectPtr<ASS_WaveManager> CachedWaveManager;

	UPROPERTY(Transient)
	TObjectPtr<UCSHGameOverWidget> GameOverWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|UI", meta=(AllowPrivateAccess="true"))
	bool bShowStartMenuOnBeginPlay = true;

	UPROPERTY(Transient)
	TObjectPtr<UCSHStartMenuWidget> StartMenuWidget;

	bool bStartMenuPresented = false;

	UPROPERTY(EditDefaultsOnly, Category="Player|UI|Title Camera")
	FVector TitleCameraLocalOffset = FVector(-1200.0f, 650.0f, 450.0f);

	UPROPERTY(EditDefaultsOnly, Category="Player|UI|Title Camera")
	FVector TitleCameraFocusLocalOffset = FVector(450.0f, 0.0f, 100.0f);

	UPROPERTY(EditDefaultsOnly, Category="Player|UI|Title Camera", meta=(ClampMin="0.0", Units="s"))
	float TitleCameraBlendDuration = 0.75f;

	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> ActiveTitleCamera;

	bool bSpawnedRuntimeTitleCamera = false;
	
float ExplosionShakeRemaining = 0.0f;
	
float ExplosionShakeDuration = 0.0f;
	
float ExplosionShakeStrength = 0.0f;
	
float ExplosionShakePhase = 0.0f;
	
FRotator PreviousExplosionShake = FRotator::ZeroRotator;
};



