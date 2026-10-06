#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSHWeaponBox.generated.h"
class ACSHWeaponBase;
class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class USoundBase;
UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHWeaponBox : public AActor
{
    GENERATED_BODY()
public:
    ACSHWeaponBox();
    bool IsAvailableForPickup() const { return !bPickupConsumed && !IsActorBeingDestroyed(); }
    void SetPreviousRandomWeapon(TSubclassOf<ACSHWeaponBase> PreviousWeapon) { PreviousRandomWeaponClass = PreviousWeapon; }
    TSubclassOf<ACSHWeaponBase> GetSelectedWeaponClass() const { return SelectedWeaponClass; }
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UBoxComponent* PickupTrigger;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* BoxMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* PreviewWeaponMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* BoxGlowMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* PreviewWeaponGlowMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pickup|Highlight") UMaterialInterface* ThroughWallGlowMaterial;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup") TSubclassOf<ACSHWeaponBase> WeaponClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup|Random") bool bRandomWeapon = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup|Random", meta=(EditCondition="bRandomWeapon")) TArray<TSubclassOf<ACSHWeaponBase>> RandomWeaponClasses;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup") bool bDestroyAfterPickup = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="0")) int32 PickupPointValue = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="10", Units="cm")) float PickupRadius = 150.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pickup|Audio") TArray<TObjectPtr<USoundBase>> PickupSounds;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pickup|Audio", meta=(ClampMin="0.0", ClampMax="2.0")) float PickupSoundVolume = 0.8f;
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
    void TryPickup(AActor* OtherActor);
    TSubclassOf<ACSHWeaponBase> PreviousRandomWeaponClass;
    TSubclassOf<ACSHWeaponBase> SelectedWeaponClass;
    bool bPickupConsumed = false;
    FTimerHandle PickupCheckTimer;
    void CheckNearbyPlayer();
    UFUNCTION() void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
