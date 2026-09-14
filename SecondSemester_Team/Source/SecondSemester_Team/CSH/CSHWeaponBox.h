#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSHWeaponBox.generated.h"
class ACSHWeaponBase;
class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInterface;
UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHWeaponBox : public AActor
{
    GENERATED_BODY()
public:
    ACSHWeaponBox();
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UBoxComponent* PickupTrigger;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* BoxMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* PreviewWeaponMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* BoxGlowMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* PreviewWeaponGlowMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pickup|Highlight") UMaterialInterface* ThroughWallGlowMaterial;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup") TSubclassOf<ACSHWeaponBase> WeaponClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup") bool bDestroyAfterPickup = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="10", Units="cm")) float PickupRadius = 150.0f;
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
    void TryPickup(AActor* OtherActor);
    bool bPickupConsumed = false;
    FTimerHandle PickupCheckTimer;
    void CheckNearbyPlayer();
    UFUNCTION() void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
