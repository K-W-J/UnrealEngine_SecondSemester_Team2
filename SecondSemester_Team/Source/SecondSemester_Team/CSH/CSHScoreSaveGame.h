#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CSHScoreSaveGame.generated.h"

/** Persistent local record for the highest weapon-box point score. */
UCLASS()
class SECONDSEMESTER_TEAM_API UCSHScoreSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	int32 HighestPointCount = 0;

	static int32 LoadHighestPointCount();
	static void SaveIfHigher(int32 PointCount);
};
