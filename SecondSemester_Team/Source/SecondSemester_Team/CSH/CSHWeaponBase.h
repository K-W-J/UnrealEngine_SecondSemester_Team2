#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSHWeaponBase.generated.h"
class ACSHBullet;
class ASecondSemester_TeamCharacter;
class USceneComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class USoundBase;
UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHWeaponBase : public AActor
{
    GENERATED_BODY()
public:
    ACSHWeaponBase();
    virtual void Tick(float DeltaSeconds) override;
    void EquipTo(ASecondSemester_TeamCharacter* NewOwner);
    void StartFiring();
    void StopFiring();
    const FText& GetWeaponDisplayName() const { return WeaponDisplayName; }
    const FText& GetWeaponDescription() const { return WeaponDescription; }
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") USceneComponent* Root;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") USceneComponent* RecoilRoot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* WeaponMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") USceneComponent* MuzzlePoint;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|UI") FText WeaponDisplayName = FText::FromString(TEXT("AK-47"));
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|UI") FText WeaponDescription = FText::FromString(TEXT("ASSAULT RIFLE / AUTO"));
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") TSubclassOf<ACSHBullet> BulletClass;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.02", Units="s")) float FireInterval = 0.1f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") bool bAutomatic = true;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Shot", meta=(ClampMin="1", ClampMax="32")) int32 ProjectilesPerShot = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Shot", meta=(ClampMin="0", ClampMax="45", Units="Degrees")) float SpreadAngleDegrees = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil", meta=(ClampMin="0", Units="cm")) float WeaponKickDistance = 7.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil", meta=(ClampMin="0")) float WeaponRecoverySpeed = 22.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil", meta=(ClampMin="0", Units="Degrees")) float CameraPitchKick = 0.7f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil", meta=(ClampMin="0", Units="Degrees")) float CameraYawKick = 0.25f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects") UNiagaraSystem* MuzzleFlash;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects") USoundBase* FireSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attachment") FTransform EquippedRelativeTransform = FTransform(FRotator::ZeroRotator, FVector(25.0f, 12.0f, -18.0f));
    UFUNCTION(BlueprintImplementableEvent, Category="Weapon", meta=(DisplayName="On Fired")) void BP_OnFired();
private:
    UPROPERTY() ASecondSemester_TeamCharacter* CharacterOwner;
    FTimerHandle FireTimer;
    FVector RecoilOffset = FVector::ZeroVector;
    bool bTriggerHeld = false;
    void FireOnce();
};

