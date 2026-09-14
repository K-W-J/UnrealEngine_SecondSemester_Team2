#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "CSHTowSword.h"
#include "CSHTowedCar.h"
#include "SecondSemester_TeamCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCSHTowTest, "CSH.Weapons.TowSwordAndVisibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCSHTowTest::RunTest(const FString& Parameters)
{
    const UWorld::InitializationValues Values = UWorld::InitializationValues().AllowAudioPlayback(false)
        .CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("CSHTowTestWorld"),
        nullptr, true, ERHIFeatureLevel::Num, &Values);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    ON_SCOPE_EXIT { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); };
    UClass* PlayerClass = LoadClass<ASecondSemester_TeamCharacter>(nullptr,
        TEXT("/Game/CSH/Buleprint/BP_CSH_Player.BP_CSH_Player_C"));
    if (!TestNotNull(TEXT("Player BP loads"), PlayerClass)) return false;
    auto* Player = World->SpawnActor<ASecondSemester_TeamCharacter>(PlayerClass, FVector(0,0,300), FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Player spawns"), Player)) return false;
    Player->DispatchBeginPlay();
    TestFalse(TEXT("First-person body hidden at BeginPlay"), Player->GetMesh()->IsVisible());
    TestFalse(TEXT("Head/arms duplicate hidden"), Player->GetFirstPersonMesh()->IsVisible());
    TestTrue(TEXT("First-person camera active"), Player->GetFirstPersonCameraComponent()->IsActive());
    TArray<UCameraComponent*> Cameras;
    Player->GetComponents(Cameras);
    for (auto* Camera : Cameras)
        if (Camera->GetFName() == TEXT("CSHDeathCamera")) TestFalse(TEXT("Death camera inactive"), Camera->IsActive());
    Player->UpdateBodyVisibility(false);
    TestTrue(TEXT("Third-person body visible"), Player->GetMesh()->IsVisible());
    TestFalse(TEXT("Third-person owner-no-see cleared"), Player->GetMesh()->bOwnerNoSee);
    Player->UpdateBodyVisibility(true);

    UClass* SwordClass = LoadClass<ACSHTowSword>(nullptr,
        TEXT("/Game/CSH/Buleprint/Weapons/TowSword/BP_CSH_TowSword.BP_CSH_TowSword_C"));
    if (!TestNotNull(TEXT("Sword BP loads"), SwordClass)) return false;
    TestTrue(TEXT("Sword equips"), Player->EquipWeapon(SwordClass));
    auto* Sword = Cast<ACSHTowSword>(Player->CurrentWeapon);
    if (!Sword) return false;
    Player->UpdateBodyVisibility(true);
    auto* SwordMesh = Sword->FindComponentByClass<UStaticMeshComponent>();
    TestTrue(TEXT("Weapon stays visible when body hidden"), SwordMesh && SwordMesh->IsVisible());
    auto* Car = World->SpawnActor<AStaticMeshActor>();
    Car->SetMobility(EComponentMobility::Movable);
    Car->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Car->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    Car->Tags.Add(TEXT("Towable"));
    const auto* Camera = Player->GetFirstPersonCameraComponent();
    Car->SetActorLocation(Camera->GetComponentLocation() + Camera->GetForwardVector()*400.f);
    Car->SetActorScale3D(FVector(2,1,1));
    const FVector OriginalScale = Car->GetActorScale3D();
    Sword->StartFiring();
    TestEqual(TEXT("LMB captures one car"), Sword->GetCapturedCarCount(), 1);
    TestFalse(TEXT("Held car collision disabled"), Car->GetActorEnableCollision());
    TestTrue(TEXT("Held car shrunk"), Car->GetActorScale3D().X < OriginalScale.X);
    Sword->StartFiring();
    TestEqual(TEXT("Cooldown prevents duplicate capture"), Sword->GetCapturedCarCount(), 1);
    Sword->Reload();
    TestEqual(TEXT("R empties stack"), Sword->GetCapturedCarCount(), 0);
    TestTrue(TEXT("Thrown car returns to full scale"), Car->GetActorScale3D().Equals(OriginalScale));
    for (TActorIterator<ACSHTowedCar> It(World); It; ++It)
    {
        It->Tick(6.f);
    }
    TestTrue(TEXT("Flight timeout restores collision"), Car->GetActorEnableCollision());
    TestFalse(TEXT("Flight timeout removes captured tag"), Car->ActorHasTag(TEXT("CSH_Towed")));
    return true;
}
#endif
