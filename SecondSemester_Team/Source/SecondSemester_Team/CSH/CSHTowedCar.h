#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSHTowedCar.generated.h"

class UPrimitiveComponent;
class UParticleSystem;

// Temporary carrier: preserves the real car and restores it when released.
UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHTowedCar : public AActor
{
    GENERATED_BODY()
public:
    ACSHTowedCar();
    bool Capture(AActor* Car, USceneComponent* Mount, float Scale);
    void Throw(const FVector& Start, const FVector& Direction, float Speed, float Damage, float Radius);
    void Release();
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Tow") UParticleSystem* ImpactEffect;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Tow") float EffectScale = 3.f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Tow") float FlightSeconds = 5.f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Tow") float Gravity = 350.f;
private:
    UPROPERTY() TObjectPtr<AActor> Target;
    struct FSavedComponent
    {
        TWeakObjectPtr<UActorComponent> Component;
        bool bTick = false;
        bool bPhysics = false;
        FTransform RelativeTransform;
        TWeakObjectPtr<USceneComponent> Parent;
        FName Socket;
    };
    TArray<FSavedComponent> SavedComponents;
    FVector OriginalScale;
    FTransform OriginalTransform;
    bool bCollision = true;
    bool bActorTick = true;
    bool bFlying = false;
    FVector Velocity;
    float DamageAmount = 0.f;
    float DamageRadius = 0.f;
    float TimeFlying = 0.f;
    float SweepRadius = 60.f;
};
