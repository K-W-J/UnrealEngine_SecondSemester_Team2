#include "CSHFists.h"
#include "SecondSemester_TeamCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ACSHFists::ACSHFists()
{
    bMeleeWeapon=true;
    bAutomatic=false;
    bInfiniteAmmo=true;
    MagazineCapacity=1;
    FireInterval=.45f;
    MeleeRange=170.f;
    MeleeRadius=30.f;
    MeleeDamage=20.f;
    MeleeKnockback=250.f;
    CameraPitchKick=CameraYawKick=0.f;
    WeaponDisplayName=FText::FromString(TEXT("FISTS"));
    WeaponDescription=FText::FromString(TEXT("LMB / PUNCH"));
    LeftGlove=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftGlove"));
    LeftGlove->SetupAttachment(RecoilRoot);
    LeftGlove->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftGlove->SetRenderCustomDepth(true);
    LeftGlove->SetCustomDepthStencilValue(42);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (Sphere.Succeeded()) { WeaponMesh->SetStaticMesh(Sphere.Object); LeftGlove->SetStaticMesh(Sphere.Object); }
    WeaponMesh->SetRelativeScale3D(FVector(.17f,.13f,.15f));
    LeftGlove->SetRelativeScale3D(FVector(.17f,.13f,.15f));
    WeaponMesh->SetRelativeLocation(FVector(0,5,-3));
    LeftGlove->SetRelativeLocation(FVector(0,-29,-3));
    // Simple glove silhouette with a thumb on each hand; replace meshes in the BP if desired.
    for (int32 Index=0; Index<2; ++Index)
    {
        UStaticMeshComponent* Thumb=CreateDefaultSubobject<UStaticMeshComponent>(Index==0 ? TEXT("RightThumb") : TEXT("LeftThumb"));
        Thumb->SetupAttachment(Index==0 ? WeaponMesh : LeftGlove.Get());
        if (Sphere.Succeeded()) Thumb->SetStaticMesh(Sphere.Object);
        Thumb->SetRelativeScale3D(FVector(.6f,.45f,.5f));
        Thumb->SetRelativeLocation(FVector(5,Index==0 ? -40.f : 40.f,-12));
        Thumb->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Thumb->SetRenderCustomDepth(true);
        Thumb->SetCustomDepthStencilValue(42);
    }
}

void ACSHFists::StartFiring()
{
    if (!IsValid(CharacterOwner) || CharacterOwner->GetHealth()<=0.f || !GetWorld()
        || GetWorld()->GetTimeSeconds()<NextAllowedFireTime) return;
    NextAllowedFireTime=GetWorld()->GetTimeSeconds()+FMath::Max(.1f,FireInterval);
    bLeftPunch=!bLeftPunch;
    PunchTime=0.f;
    const auto* Camera=CharacterOwner->GetFirstPersonCameraComponent();
    const FVector Start=Camera->GetComponentLocation(), Direction=Camera->GetForwardVector();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(CSHFist),false,CharacterOwner);
    Query.AddIgnoredActor(this);
    FHitResult Hit;
    if (GetWorld()->SweepSingleByChannel(Hit,Start,Start+Direction*MeleeRange,FQuat::Identity,
        ECC_Visibility,FCollisionShape::MakeSphere(FMath::Max(1.f,MeleeRadius)),Query))
    {
        if (AActor* Target=Hit.GetActor())
        {
            UGameplayStatics::ApplyPointDamage(Target,MeleeDamage,Direction,Hit,CharacterOwner->GetController(),this,UDamageType::StaticClass());
            if (auto* Component=Hit.GetComponent(); Component && Component->IsSimulatingPhysics())
                Component->AddImpulseAtLocation(Direction*MeleeKnockback,Hit.ImpactPoint);
        }
    }
    BP_OnFired();
}

void ACSHFists::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    PunchTime+=DeltaSeconds;
    const float Duration=FMath::Max(.1f,FireInterval);
    const float Extension=FMath::Sin(FMath::Clamp(PunchTime/Duration,0.f,1.f)*PI)*24.f;
    WeaponMesh->SetRelativeLocation(FVector(bLeftPunch ? 0.f : Extension,5,-3));
    LeftGlove->SetRelativeLocation(FVector(bLeftPunch ? Extension : 0.f,-29,-3));
}
