#include "CSHWeaponBox.h"
#include "CSHWeaponBase.h"
#include "SecondSemester_TeamCharacter.h"
#include "SecondSemester_TeamPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundBase.h"
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
    static ConstructorHelpers::FObjectFinder<USoundBase> CartoonLaugh(TEXT("/Game/Audio/FunnyVoices/SFX_Voice_CartoonLaugh.SFX_Voice_CartoonLaugh"));
    static ConstructorHelpers::FObjectFinder<USoundBase> FunnyKid(TEXT("/Game/Audio/FunnyVoices/SFX_Voice_FunnyKid.SFX_Voice_FunnyKid"));
    static ConstructorHelpers::FObjectFinder<USoundBase> FairySuccess(TEXT("/Game/Audio/FunnyVoices/SFX_Voice_FairySuccess.SFX_Voice_FairySuccess"));
    static ConstructorHelpers::FObjectFinder<USoundBase> YesVictory(TEXT("/Game/Audio/FunnyVoices/SFX_Voice_YesVictory.SFX_Voice_YesVictory"));
    PickupSounds = { CartoonLaugh.Object, FunnyKid.Object, FairySuccess.Object, YesVictory.Object };
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
    if (bPickupConsumed || IsActorBeingDestroyed()) return;
    GetWorldTimerManager().SetTimer(PickupCheckTimer, this, &ACSHWeaponBox::CheckNearbyPlayer, 0.05f, true);
}

void ACSHWeaponBox::CheckNearbyPlayer()
{
    if (bPickupConsumed) return;

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
    if (bPickupConsumed) return;

    if (ASecondSemester_TeamCharacter* Character = Cast<ASecondSemester_TeamCharacter>(OtherActor))
    {
        if (Character->GetHealth() <= 0.f) return;
        TSubclassOf<ACSHWeaponBase> SelectedClass = WeaponClass;
        if (bRandomWeapon)
        {
            TArray<TSubclassOf<ACSHWeaponBase>> Candidates;
            for (const auto& Candidate : RandomWeaponClasses)
            {
                if (Candidate && !Candidate->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
                    Candidates.AddUnique(Candidate);
            }
            if (Candidates.IsEmpty()) return;
            SelectedClass = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
        }
        if (!SelectedClass) return;
        // Guard against overlap callbacks during weapon spawning.
        bPickupConsumed = true;
        if (Character->EquipWeapon(SelectedClass))
        {
            GetWorldTimerManager().ClearTimer(PickupCheckTimer);
            if (ASecondSemester_TeamPlayerController* PlayerController =
                Cast<ASecondSemester_TeamPlayerController>(Character->GetController()))
            {
                PlayerController->AddWeaponBoxPoints(PickupPointValue);
            }
            TArray<USoundBase*> ValidSounds;
            for (USoundBase* Sound : PickupSounds)
            {
                if (Sound) ValidSounds.Add(Sound);
            }
            if (!ValidSounds.IsEmpty())
            {
                UGameplayStatics::PlaySound2D(this,
                    ValidSounds[FMath::RandRange(0, ValidSounds.Num() - 1)], PickupSoundVolume);
            }
            if (bDestroyAfterPickup) Destroy();
        }
        else bPickupConsumed = false;
    }
}

void ACSHWeaponBox::OnPickupOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TryPickup(OtherActor);
}
