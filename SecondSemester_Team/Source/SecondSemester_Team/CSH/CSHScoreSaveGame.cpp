#include "CSHScoreSaveGame.h"

#include "Kismet/GameplayStatics.h"

namespace
{
	const FString HighScoreSlot = TEXT("CSH_HighScore");
	constexpr int32 HighScoreUserIndex = 0;
}

int32 UCSHScoreSaveGame::LoadHighestPointCount()
{
	if (const UCSHScoreSaveGame* Save = Cast<UCSHScoreSaveGame>(
		UGameplayStatics::LoadGameFromSlot(HighScoreSlot, HighScoreUserIndex)))
	{
		return FMath::Max(0, Save->HighestPointCount);
	}
	return 0;
}

void UCSHScoreSaveGame::SaveIfHigher(const int32 PointCount)
{
	if (PointCount <= LoadHighestPointCount())
	{
		return;
	}
	SaveHighestPointCount(PointCount);
}

void UCSHScoreSaveGame::SaveHighestPointCount(const int32 PointCount)
{
	if (PointCount <= 0)
	{
		return;
	}

	UCSHScoreSaveGame* Save = Cast<UCSHScoreSaveGame>(
		UGameplayStatics::CreateSaveGameObject(StaticClass()));
	if (!Save)
	{
		return;
	}

	Save->HighestPointCount = PointCount;
	UGameplayStatics::SaveGameToSlot(Save, HighScoreSlot, HighScoreUserIndex);
}
