#pragma once
#include "CoreMinimal.h"
#include "CSHWeaponBase.h"
#include "CSHFists.generated.h"
UCLASS(Blueprintable)
class SECONDSEMESTER_TEAM_API ACSHFists : public ACSHWeaponBase
{
    GENERATED_BODY()
public:
    ACSHFists();
    virtual void StartFiring() override;
    virtual void Tick(float DeltaSeconds) override;
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") TObjectPtr<UStaticMeshComponent> LeftGlove;
private:
    bool bLeftPunch=false;
    float PunchTime=1.f;
};
