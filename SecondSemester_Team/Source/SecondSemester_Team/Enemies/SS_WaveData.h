#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SS_WaveData.generated.h"

class ASS_Enemy;

USTRUCT(BlueprintType)
struct FSS_WaveEnemyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSubclassOf<ASS_Enemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0"))
	int32 Count = 1;
};

UCLASS(BlueprintType)
class SECONDSEMESTER_TEAM_API USS_WaveData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (TitleProperty = "EnemyClass"))
	TArray<FSS_WaveEnemyEntry> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0.05", Units = "s"))
	float SpawnInterval = 2.0f;
};
