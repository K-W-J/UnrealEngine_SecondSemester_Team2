#pragma once
#include "CoreMinimal.h"
#include "CSHWeaponBase.h"
#include "CSHFeignDeathWeapon.generated.h"

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHFeignDeathWeapon : public ACSHWeaponBase
{
    GENERATED_BODY()
public:
    virtual void StartFiring() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Feign Death", meta=(ClampMin="1")) float FeignDuration = 5.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Feign Death", meta=(ClampMin="0.03")) float SprayInterval = .1f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Feign Death") float SpinSpeed = 360.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Feign Death", meta=(ClampMin="0")) float RecoveryCooldown = 2.f;
private:
    bool bFeigning = false;
    bool bChangedInput = false;
    float Elapsed = 0.f;
    float UntilShot = 0.f;
    FTransform SavedMeshTransform;
    FName SavedCollisionProfile;
    bool bSavedOwnerNoSee = true;
    uint8 SavedMovementMode = 1;
    UPROPERTY() class UCameraComponent* OverviewCamera;
    void FinishFeign();
    void Spray();
};
