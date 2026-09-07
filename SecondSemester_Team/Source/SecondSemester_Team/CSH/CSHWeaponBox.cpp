#include "CSHWeaponBox.h"
#include "CSHWeaponBase.h"
#include "SecondSemester_TeamCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
ACSHWeaponBox::ACSHWeaponBox()
{
    PrimaryActorTick.bCanEverTick = false;
    PickupTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupTrigger")); SetRootComponent(PickupTrigger);
    PickupTrigger->SetBoxExtent(FVector(90.0f)); PickupTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupTrigger->SetGenerateOverlapEvents(true); PickupTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    PickupTrigger->SetCollisionResponseToAllChannels(ECR_Ignore); PickupTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &ACSHWeaponBox::OnPickupOverlap);
    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh")); BoxMesh->SetupAttachment(PickupTrigger); BoxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PreviewWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewWeaponMesh")); PreviewWeaponMesh->SetupAttachment(PickupTrigger);
    PreviewWeaponMesh->SetRelativeLocation(FVector(0, 0, 55)); PreviewWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ACSHWeaponBox::BeginPlay()
{
    Super::BeginPlay();
    CheckNearbyPlayer();
    GetWorldTimerManager().SetTimer(PickupCheckTimer, this, &ACSHWeaponBox::CheckNearbyPlayer, 0.05f, true);
}

void ACSHWeaponBox::CheckNearbyPlayer()
{
    if (bPickupConsumed || !WeaponClass) return;

    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        const FVector PickupCenter = PickupTrigger ? PickupTrigger->GetComponentLocation() : GetActorLocation();
        if (FVector::DistSquared(Player->GetActorLocation(), PickupCenter) <= FMath::Square(PickupRadius))
        {
            TryPickup(Player);
        }
    }
}
void ACSHWeaponBox::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);
    TryPickup(OtherActor);
}

void ACSHWeaponBox::TryPickup(AActor* OtherActor)
{
    if (bPickupConsumed || !WeaponClass) return;

    if (ASecondSemester_TeamCharacter* Character = Cast<ASecondSemester_TeamCharacter>(OtherActor))
    {
        if (Character->EquipWeapon(WeaponClass))
        {
            bPickupConsumed = true;
            if (bDestroyAfterPickup) Destroy();
        }
    }
}

void ACSHWeaponBox::OnPickupOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TryPickup(OtherActor);
}
