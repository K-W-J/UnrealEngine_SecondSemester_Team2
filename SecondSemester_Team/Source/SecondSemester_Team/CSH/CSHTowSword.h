#pragma once
#include "CoreMinimal.h"
#include "CSHWeaponBase.h"
#include "CSHTowSword.generated.h"

class ACSHTowedCar;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHTowSword : public ACSHWeaponBase
{
    GENERATED_BODY()
public:
    ACSHTowSword();
    virtual void StartFiring() override;
    virtual void Reload() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") int32 MaxCars = 5;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") float CaptureRange = 650.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") float CaptureRadius = 150.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") float HeldScale = 0.12f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") float StackSpacing = 26.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") float ThrowSpeed = 2000.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") float ImpactDamage = 150.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") float ImpactRadius = 450.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Tow") TSubclassOf<ACSHTowedCar> TowedCarClass;
    UFUNCTION(BlueprintPure, Category="Weapon|Tow") int32 GetCapturedCarCount() const { return Cars.Num(); }
private:
    UPROPERTY() TArray<TObjectPtr<ACSHTowedCar>> Cars;
    void ReleaseCars();
};
