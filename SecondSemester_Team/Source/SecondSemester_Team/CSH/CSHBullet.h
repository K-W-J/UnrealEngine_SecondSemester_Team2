#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSHBullet.generated.h"
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPointLightComponent;
UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHBullet : public AActor
{
    GENERATED_BODY()
public:
    ACSHBullet();
    void IgnoreProjectile(ACSHBullet* OtherProjectile);
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") USphereComponent* Collision;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UStaticMeshComponent* BulletMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UPointLightComponent* TracerLight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement") UProjectileMovementComponent* ProjectileMovement;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0")) float Damage = 20.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="100", Units="cm/s")) float InitialSpeed = 2000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tracer") FLinearColor TracerColor = FLinearColor(1.0f, 0.18f, 0.01f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tracer", meta=(ClampMin="0")) float TracerLightIntensity = 1200.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lifetime", meta=(ClampMin="0.1", Units="s")) float LifeSeconds = 3.0f;
    virtual void BeginPlay() override;
    UFUNCTION() void OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
};
