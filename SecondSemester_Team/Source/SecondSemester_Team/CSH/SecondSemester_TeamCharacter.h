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
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ASecondSemester_TeamCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Camera-relative attachment point for the currently equipped weapon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CSHDeathSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CSHDeathCamera;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;
	
public:
	ASecondSemester_TeamCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category="Player|Health")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool EquipWeapon(TSubclassOf<ACSHWeaponBase> WeaponClass);

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	void StartWeaponFire();
	void StopWeaponFire();
	void StartSprint();
	void StopSprint();
	void UpdateSprint(float DeltaSeconds);
	void ApplyDamageCameraKick(float DamageAmount, const AActor* DamageCauser);
	void UpdateDamageCameraKick(float DeltaSeconds);
	void DebugTakeDamage();
	void DebugHeal();
	void EnterDeathState();
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns the camera-relative weapon attachment point. */
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
	float MaxStamina = 100.0f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Player|Stamina")
	float Stamina = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Movement", meta=(ClampMin="0", Units="cm/s"))
	float CSHWalkSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Movement", meta=(ClampMin="0", Units="cm/s"))
	float CSHSprintSpeed = 850.0f;
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
	bool bSprintRequested = false;
	bool bIsSprinting = false;
	float TimeSinceSprintStopped = 0.0f;
	FRotator DamageCameraKick = FRotator::ZeroRotator;
	bool bDeathStateEntered = false;
};



