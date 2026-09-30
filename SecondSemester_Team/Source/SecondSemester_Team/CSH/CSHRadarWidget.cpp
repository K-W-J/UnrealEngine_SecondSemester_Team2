#include "CSHRadarWidget.h"
#include "CSHWeaponBox.h"
#include "Enemies/SS_Enemy.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SLeafWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/UnrealType.h"

class SCSHRadar : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SCSHRadar) {} SLATE_ARGUMENT(UCSHRadarWidget*, Owner) SLATE_END_ARGS()
    void Construct(const FArguments& Args)
    {
        Owner = Args._Owner;
        Screen.SetResourceObject(Args._Owner->ScreenTexture);
        Screen.ImageSize = FVector2D(256);
        Screen.DrawAs = ESlateBrushDrawType::Image;
    }
    virtual FVector2D ComputeDesiredSize(float) const override
    { return FVector2D(Owner.IsValid() ? Owner->RadarSize : 240.f); }
    virtual void Tick(const FGeometry& Geometry, double Time, float Delta) override
    {
        SLeafWidget::Tick(Geometry, Time, Delta);
        if (!Owner.IsValid() || !Owner->GetWorld()) return;
        Owner->UpdateRadarSystem();
        Screen.SetResourceObject(Owner->GetRadarMaterial());
        Refresh -= Delta;
        if (Refresh <= 0.f)
        {
            Refresh = .25f;
            Boxes.Reset();
            for (TActorIterator<ACSHWeaponBox> It(Owner->GetWorld()); It; ++It)
                if (It->IsAvailableForPickup()) Boxes.Add(*It);
            // Tag eligibility only. Detection and target drawing run in BP_InvisibleRadar.
            for (TActorIterator<ASS_Enemy> It(Owner->GetWorld()); It; ++It)
            {
                if (!It->bIsDead && !It->ActorHasTag(TEXT("CSH_Towed"))) It->Tags.AddUnique(TEXT("CSHRadarEnemy"));
                else It->Tags.Remove(TEXT("CSHRadarEnemy"));
            }
        }
        Invalidate(EInvalidateWidgetReason::Paint);
        const APawn* Player=Owner->GetOwningPlayerPawn();
        if (Player && Owner->bShowWalls)
        {
            // Sample a horizontal collision slice in small batches, not the whole map each frame.
            if (ScanIndex == 0)
            {
                ScanOrigin=Player->GetActorLocation();
                Cell=FMath::Max(Owner->WallCellSize,Owner->DetectionRange/32.f);
                HalfCells=FMath::CeilToInt(Owner->DetectionRange/Cell);
                PendingWalls.Reset();
            }
            const int32 Side=HalfCells*2+1;
            FCollisionQueryParams Query(SCENE_QUERY_STAT(RadarWalls),false,Player);
            FCollisionObjectQueryParams Objects(ECC_WorldStatic);
            for (int32 Budget=0; Budget<64 && ScanIndex<Side*Side; ++Budget,++ScanIndex)
            {
                const FVector Offset((ScanIndex%Side-HalfCells)*Cell,(ScanIndex/Side-HalfCells)*Cell,0);
                if (Offset.SizeSquared2D()>FMath::Square(Owner->DetectionRange)) continue;
                const FVector Position=ScanOrigin+Offset+FVector(0,0,Owner->WallScanHeight);
                TArray<FOverlapResult> Hits;
                Owner->GetWorld()->OverlapMultiByObjectType(Hits,Position,FQuat::Identity,Objects,
                    FCollisionShape::MakeBox(FVector(Cell*.45f,Cell*.45f,25.f)),Query);
                for (const auto& Hit : Hits)
                {
                    const UPrimitiveComponent* Component=Hit.GetComponent();
                    if (Component && Component->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Block
                        && !Component->ComponentHasTag(TEXT("RadarIgnore"))
                        && (!Hit.GetActor() || !Hit.GetActor()->ActorHasTag(TEXT("RadarIgnore"))))
                    { PendingWalls.Add(Position); break; }
                }
            }
            if (ScanIndex>=Side*Side) { Walls=MoveTemp(PendingWalls); DisplayCell=Cell; ScanIndex=0; }
        }
        else { Walls.Reset(); ScanIndex=0; }
    }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&,
        FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle&, bool) const override
    {
        if (!Owner.IsValid()) return Layer;
        const FVector2D Size = G.GetLocalSize(), Center = Size * .5f;
        const float Radius = FMath::Min(Size.X, Size.Y) * .44f;
        const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
        FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(), White, ESlateDrawEffect::None, FLinearColor(.005,.02,.025,.9));
        if (Screen.GetResourceObject())
        {
            const APlayerController* View=Owner->GetOwningPlayer();
            const float Angle=FMath::DegreesToRadians(-90.f-(View ? View->GetControlRotation().Yaw : 0.f));
            FSlateDrawElement::MakeRotatedBox(Out,Layer+2,
                G.ToPaintGeometry(FVector2D(Radius*2),FSlateLayoutTransform(Center-FVector2D(Radius))),
                &Screen,ESlateDrawEffect::None,Angle,TOptional<FVector2D>(),FSlateDrawElement::RelativeToElement,FLinearColor::White);
        }
        const auto Line = [&](const TArray<FVector2D>& Points, FLinearColor Color, float Width)
        { FSlateDrawElement::MakeLines(Out, Layer+2, G.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, Width); };
        const APlayerController* PC=Owner->GetOwningPlayer();
        const APawn* Player=PC ? PC->GetPawn() : nullptr;
        if (Player)
        {
            const FRotator Heading(0,PC->GetControlRotation().Yaw,0);
            const FVector Forward=Heading.Vector(), Right=FRotationMatrix(Heading).GetUnitAxis(EAxis::Y);
            const float Range=FMath::Max(100.f,Owner->DetectionRange);
            for (const auto& WeakBox : Boxes)
            {
                const ACSHWeaponBox* Box=WeakBox.Get();
                if (!Box || !Box->IsAvailableForPickup() || Box->IsHidden()) continue;
                const FVector Delta=Box->GetActorLocation()-Player->GetActorLocation();
                if (Delta.SizeSquared2D()>Range*Range) continue;
                const FVector2D P=Center+FVector2D(FVector::DotProduct(Delta,Right),-FVector::DotProduct(Delta,Forward))*(Radius-5.f)/Range;
                const TArray<FVector2D> Diamond={P+FVector2D(0,-5),P+FVector2D(5,0),P+FVector2D(0,5),P+FVector2D(-5,0),P+FVector2D(0,-5)};
                FSlateDrawElement::MakeLines(Out,Layer+3,G.ToPaintGeometry(),Diamond,ESlateDrawEffect::None,FLinearColor(.1f,.8f,1.f,1.f),true,2.f);
            }
            // Outline occupied cells rather than drawing disconnected tiny squares.
            TSet<FIntPoint> Occupied;
            const FVector Origin=Walls.IsEmpty() ? FVector::ZeroVector : Walls[0];
            for (const FVector& Wall : Walls)
            {
                Occupied.Add(FIntPoint(FMath::RoundToInt((Wall.X-Origin.X)/DisplayCell),FMath::RoundToInt((Wall.Y-Origin.Y)/DisplayCell)));
            }
            const auto Project=[&](FVector Position)
            {
                const FVector Delta=Position-Player->GetActorLocation();
                return Center+FVector2D(FVector::DotProduct(Delta,Right),-FVector::DotProduct(Delta,Forward))*(Radius-5.f)/Range;
            };
            const FIntPoint Neighbors[]={{-1,0},{1,0},{0,-1},{0,1}};
            // Flood from outside the sampled footprint. Enclosed empty rooms are
            // not exterior space, so their walls must not appear on the radar.
            TSet<FIntPoint> Exterior;
            if (!Occupied.IsEmpty())
            {
                FIntPoint MinCell(MAX_int32,MAX_int32), MaxCell(MIN_int32,MIN_int32);
                for (const FIntPoint& Position : Occupied)
                {
                    MinCell.X=FMath::Min(MinCell.X,Position.X);
                    MinCell.Y=FMath::Min(MinCell.Y,Position.Y);
                    MaxCell.X=FMath::Max(MaxCell.X,Position.X);
                    MaxCell.Y=FMath::Max(MaxCell.Y,Position.Y);
                }
                MinCell-=FIntPoint(1,1);
                MaxCell+=FIntPoint(1,1);
                TArray<FIntPoint> Queue;
                Queue.Add(MinCell);
                Exterior.Add(MinCell);
                for (int32 Index=0; Index<Queue.Num(); ++Index)
                {
                    const FIntPoint Current=Queue[Index];
                    for (const FIntPoint& Direction : Neighbors)
                    {
                        const FIntPoint Next=Current+Direction;
                        if (Next.X<MinCell.X || Next.Y<MinCell.Y || Next.X>MaxCell.X || Next.Y>MaxCell.Y
                            || Occupied.Contains(Next) || Exterior.Contains(Next)) continue;
                        Exterior.Add(Next);
                        Queue.Add(Next);
                    }
                }
            }
            for (const FIntPoint& CellPosition : Occupied)
            {
                const FVector Position=Origin+FVector(CellPosition.X*DisplayCell,CellPosition.Y*DisplayCell,0);
                const float Half=DisplayCell*.5f;
                const FVector Corners[]={Position+FVector(-Half,-Half,0),Position+FVector(-Half,Half,0),Position+FVector(Half,Half,0),Position+FVector(Half,-Half,0)};
                const int32 Ends[][2]={{0,1},{3,2},{0,3},{1,2}};
                for (int32 Side=0; Side<4; ++Side)
                {
                    if (!Exterior.Contains(CellPosition+Neighbors[Side])) continue;
                    const FVector2D A=Project(Corners[Ends[Side][0]]), B=Project(Corners[Ends[Side][1]]);
                    if ((A-Center).SizeSquared()>FMath::Square(Radius-2.f) || (B-Center).SizeSquared()>FMath::Square(Radius-2.f)) continue;
                    FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),{A,B},ESlateDrawEffect::None,Owner->WallColor,true,1.6f);
                }
            }
        }
        Line({Center+FVector2D(-5,5),Center+FVector2D(0,-7),Center+FVector2D(5,5),Center+FVector2D(-5,5)},FLinearColor(0,1,1,1),2.f);
        FSlateDrawElement::MakeText(Out,Layer+4,G.ToPaintGeometry(FVector2D(100,16),FSlateLayoutTransform(FVector2D(8,5))),
            FString::Printf(TEXT("RADAR  %.0fm"),Owner->DetectionRange/100.f),FCoreStyle::GetDefaultFontStyle("Bold",10),ESlateDrawEffect::None,FLinearColor(.4,1,.8,1));
        return Layer+4;
    }
private:
    TWeakObjectPtr<UCSHRadarWidget> Owner;
    FSlateBrush Screen;
    float Refresh=0;
    TArray<TWeakObjectPtr<ACSHWeaponBox>> Boxes;
    TArray<FVector> Walls, PendingWalls;
    FVector ScanOrigin=FVector::ZeroVector;
    float Cell=200.f, DisplayCell=200.f;
    int32 ScanIndex=0, HalfCells=0;
};

UCSHRadarWidget::UCSHRadarWidget(const FObjectInitializer& Initializer) : Super(Initializer)
{
    static ConstructorHelpers::FObjectFinder<UTexture2D> Texture(TEXT("/Game/RadarSystem/Textures/T_Screen.T_Screen"));
    if (Texture.Succeeded()) ScreenTexture=Texture.Object;
}
TSharedRef<SWidget> UCSHRadarWidget::RebuildWidget() { return SNew(SCSHRadar).Owner(this); }

void UCSHRadarWidget::UpdateRadarSystem()
{
    APawn* Player=GetOwningPlayerPawn();
    if (!Player || !GetWorld() || IsDesignTime()) return;
    if (!IsValid(RadarActor))
    {
        UClass* RadarClass=LoadClass<AActor>(nullptr,TEXT("/Game/RadarSystem/Blueprints/BP_InvisibleRadar.BP_InvisibleRadar_C"));
        UMaterialInterface* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/CSH/Materials/M_CSH_RadarSystemUI.M_CSH_RadarSystemUI"));
        if (!RadarClass || !Material) return;
        RadarTarget=NewObject<UTextureRenderTarget2D>(this);
        RadarTarget->ClearColor=FLinearColor::Black;
        RadarTarget->InitAutoFormat(512,512);
        RadarTarget->UpdateResourceImmediate(true);
        RadarMaterial=UMaterialInstanceDynamic::Create(Material,this);
        RadarMaterial->SetTextureParameterValue(TEXT("Render Target"),RadarTarget);
        RadarMaterial->SetScalarParameterValue(TEXT("Rotate Detetion Dots"),0.f);
        RadarMaterial->SetScalarParameterValue(TEXT("Rotation Rate"),1.f);
        RadarMaterial->SetScalarParameterValue(TEXT("Enable Line"),1.f);
        const FTransform Transform(FRotator::ZeroRotator,Player->GetActorLocation());
        RadarActor=GetWorld()->SpawnActorDeferred<AActor>(RadarClass,Transform,Player,Player,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!RadarActor) return;
        auto* Range=FindFProperty<FDoubleProperty>(RadarClass,TEXT("Range"));
        auto* Tag=FindFProperty<FNameProperty>(RadarClass,TEXT("Detect Tag"));
        auto* Target=FindFProperty<FObjectPropertyBase>(RadarClass,TEXT("Render Target"));
        auto* Screens=FindFProperty<FArrayProperty>(RadarClass,TEXT("Screen Actors"));
        if (!Range || !Tag || !Target || !Screens)
        { RadarActor->Destroy(); RadarActor=nullptr; return; }
        // The vendor construction macro indexes element zero even for an empty array.
        // Supply a hidden mesh endpoint that shares the actual UI material instance.
        FScriptArrayHelper ScreenArray(Screens,Screens->ContainerPtrToValuePtr<void>(RadarActor));
        ScreenArray.EmptyValues();
        auto* ScreenStruct=CastField<FStructProperty>(Screens->Inner);
        FObjectPropertyBase* ScreenActorProperty=nullptr;
        if (ScreenStruct)
            for (TFieldIterator<FObjectPropertyBase> It(ScreenStruct->Struct); It; ++It)
                if (It->GetName().StartsWith(TEXT("Actor_"))) { ScreenActorProperty=*It; break; }
        if (!ScreenActorProperty) { RadarActor->Destroy(); RadarActor=nullptr; return; }
        UStaticMeshComponent* ScreenEndpoint=NewObject<UStaticMeshComponent>(RadarActor,TEXT("CSHUIScreenEndpoint"));
        RadarActor->AddInstanceComponent(ScreenEndpoint);
        ScreenEndpoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ScreenEndpoint->SetVisibility(false);
        ScreenEndpoint->SetHiddenInGame(true);
        ScreenEndpoint->SetMaterial(0,RadarMaterial);
        ScreenEndpoint->RegisterComponent();
        ScreenArray.AddValue();
        ScreenActorProperty->SetObjectPropertyValue_InContainer(ScreenArray.GetRawPtr(0),RadarActor);
        Range->SetPropertyValue_InContainer(RadarActor,DetectionRange);
        Tag->SetPropertyValue_InContainer(RadarActor,TEXT("CSHRadarEnemy"));
        Target->SetObjectPropertyValue_InContainer(RadarActor,RadarTarget);
        if (auto* DotRadius=FindFProperty<FDoubleProperty>(RadarClass,TEXT("Dot Radius")))
            DotRadius->SetPropertyValue_InContainer(RadarActor,8.0); // Slightly smaller than the previous 9.0.
        if (auto* Volume=FindFProperty<FFloatProperty>(RadarClass,TEXT("Volume Multiplier"))) Volume->SetPropertyValue_InContainer(RadarActor,.15f);
        RadarActor->FinishSpawning(Transform);
    }
    RadarActor->SetActorLocation(Player->GetActorLocation());
    if (auto* Range=FindFProperty<FDoubleProperty>(RadarActor->GetClass(),TEXT("Range")))
        Range->SetPropertyValue_InContainer(RadarActor,FMath::Max(100.f,DetectionRange));
}

void UCSHRadarWidget::NativeDestruct()
{
    if (IsValid(RadarActor)) RadarActor->Destroy();
    RadarActor=nullptr;
    RadarMaterial=nullptr;
    RadarTarget=nullptr;
    Super::NativeDestruct();
}
