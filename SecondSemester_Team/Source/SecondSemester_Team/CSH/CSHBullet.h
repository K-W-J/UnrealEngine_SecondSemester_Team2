#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSHBullet.generated.h"
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPointLightComponent;
class UParticleSystem;
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
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0")) float KnockbackImpulse = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage|Explosion") bool bExplosive = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage|Explosion", meta=(ClampMin="0", Units="cm")) float ExplosionRadius = 300.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage|Explosion", meta=(ClampMin="0")) float ExplosionDamage = 80.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects|Explosion") bool bSpawnImpactExplosion = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects|Explosion") TObjectPtr<UParticleSystem> ImpactExplosionEffect;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects|Explosion", meta=(ClampMin="0.01")) float ImpactExplosionScale = 1.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects|Explosion", meta=(ClampMin="0", Units="cm")) float CameraShakeInnerRadius = 250.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects|Explosion", meta=(ClampMin="0", Units="cm")) float CameraShakeOuterRadius = 1800.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage|Piercing") bool bPiercing = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage|Piercing", meta=(ClampMin="1")) int32 MaxPenetrations = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="100", Units="cm/s")) float InitialSpeed = 2000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0.1", Units="cm")) float CollisionRadius = 2.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Homing") bool bHoming = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Homing", meta=(ClampMin="0")) float HomingAcceleration = 8000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Homing", meta=(ClampMin="0", Units="cm")) float HomingSearchRadius = 5000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0")) float GravityScale = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tracer") FLinearColor TracerColor = FLinearColor(1.0f, 0.18f, 0.01f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tracer", meta=(ClampMin="0")) float TracerLightIntensity = 1200.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lifetime", meta=(ClampMin="0.1", Units="s")) float LifeSeconds = 3.0f;
    virtual void BeginPlay() override;
    int32 PenetrationCount = 0;
    void AcquireHomingTarget();
    UFUNCTION() void OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
};
