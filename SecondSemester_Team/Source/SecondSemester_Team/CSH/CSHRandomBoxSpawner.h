#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSHRandomBoxSpawner.generated.h"
class ACSHWeaponBox;

UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHRandomBoxSpawner : public AActor
{
    GENERATED_BODY()
public:
    ACSHRandomBoxSpawner();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Random Box") TSubclassOf<ACSHWeaponBox> BoxClass;
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Random Box") TArray<TObjectPtr<AActor>> BoxPoints;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Random Box") FVector SpawnOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Random Box", meta=(ClampMin="0.1")) float RespawnDelay = .2f;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Random Box") TObjectPtr<ACSHWeaponBox> ActiveBox;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    TWeakObjectPtr<AActor> LastPoint;
    FTimerHandle RespawnTimer;
    bool bStopping = false;
    void SpawnNextBox();
    UFUNCTION() void OnBoxDestroyed(AActor* DestroyedActor);
};
