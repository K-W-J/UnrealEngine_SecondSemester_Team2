#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHRadarWidget.generated.h"
class UTexture2D;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API UCSHRadarWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UCSHRadarWidget(const FObjectInitializer& Initializer);
    void UpdateRadarSystem();
    UMaterialInstanceDynamic* GetRadarMaterial() const { return RadarMaterial; }
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Radar", meta=(ClampMin="100", Units="cm")) float DetectionRange = 5000.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Radar", meta=(ClampMin="120")) float RadarSize = 240.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Radar") TObjectPtr<UTexture2D> ScreenTexture;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Radar|Walls") bool bShowWalls = true;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Radar|Walls", meta=(ClampMin="50", Units="cm")) float WallCellSize = 200.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Radar|Walls", meta=(Units="cm")) float WallScanHeight = 50.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Radar|Walls") FLinearColor WallColor = FLinearColor(.42f,.48f,.5f,.85f);
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;
private:
    UPROPERTY(Transient) TObjectPtr<AActor> RadarActor;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> RadarMaterial;
    UPROPERTY(Transient) TObjectPtr<UTextureRenderTarget2D> RadarTarget;
};
