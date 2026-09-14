#include "CSHWeaponBox.h"
#include "CSHWeaponBase.h"
#include "SecondSemester_TeamCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
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
    BoxMesh->SetRenderCustomDepth(true); BoxMesh->SetCustomDepthStencilValue(41);
    PreviewWeaponMesh->SetRenderCustomDepth(true); PreviewWeaponMesh->SetCustomDepthStencilValue(41);
    BoxGlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxGlowMesh")); BoxGlowMesh->SetupAttachment(PickupTrigger);
    BoxGlowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); BoxGlowMesh->SetCastShadow(false);
    PreviewWeaponGlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewWeaponGlowMesh")); PreviewWeaponGlowMesh->SetupAttachment(PickupTrigger);
    PreviewWeaponGlowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); PreviewWeaponGlowMesh->SetCastShadow(false);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMaterial(TEXT("/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2.M_CSH_WeaponBoxWallsOnlyV2"));
    if (GlowMaterial.Succeeded()) ThroughWallGlowMaterial = GlowMaterial.Object;
}
void ACSHWeaponBox::BeginPlay()
{
    Super::BeginPlay();
    if (BoxGlowMesh && BoxMesh)
    {
        BoxGlowMesh->SetStaticMesh(BoxMesh->GetStaticMesh());
        BoxGlowMesh->SetRelativeTransform(BoxMesh->GetRelativeTransform());
        BoxGlowMesh->SetRelativeScale3D(BoxGlowMesh->GetRelativeScale3D() * 1.03f);
        BoxGlowMesh->SetVisibility(true, true);
        BoxGlowMesh->SetHiddenInGame(false, true);
        BoxGlowMesh->SetRenderInMainPass(true);
        BoxGlowMesh->SetTranslucentSortPriority(100);
        if (ThroughWallGlowMaterial)
        {
            BoxGlowMesh->SetMaterial(0, ThroughWallGlowMaterial);
            for (int32 Index = 1; Index < BoxGlowMesh->GetNumMaterials(); ++Index) BoxGlowMesh->SetMaterial(Index, ThroughWallGlowMaterial);
        }
    }
    if (PreviewWeaponGlowMesh && PreviewWeaponMesh)
    {
        PreviewWeaponGlowMesh->SetStaticMesh(PreviewWeaponMesh->GetStaticMesh());
        PreviewWeaponGlowMesh->SetRelativeTransform(PreviewWeaponMesh->GetRelativeTransform());
        PreviewWeaponGlowMesh->SetRelativeScale3D(PreviewWeaponGlowMesh->GetRelativeScale3D() * 1.03f);
        PreviewWeaponGlowMesh->SetVisibility(true, true);
        PreviewWeaponGlowMesh->SetHiddenInGame(false, true);
        PreviewWeaponGlowMesh->SetRenderInMainPass(true);
        PreviewWeaponGlowMesh->SetTranslucentSortPriority(101);
        if (ThroughWallGlowMaterial)
        {
            PreviewWeaponGlowMesh->SetMaterial(0, ThroughWallGlowMaterial);
            for (int32 Index = 1; Index < PreviewWeaponGlowMesh->GetNumMaterials(); ++Index) PreviewWeaponGlowMesh->SetMaterial(Index, ThroughWallGlowMaterial);
        }
    }
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
