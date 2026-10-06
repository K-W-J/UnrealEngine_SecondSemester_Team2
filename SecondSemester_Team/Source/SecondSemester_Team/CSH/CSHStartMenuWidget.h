#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSHStartMenuWidget.generated.h"

class UBorder;
class UButton;
class UImage;
class UTextBlock;
class UTexture2D;
class UVerticalBox;

/** Full-screen title menu displayed before gameplay begins. */
UCLASS()
class SECONDSEMESTER_TEAM_API UCSHStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCSHStartMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

private:
	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION() void HandleTutorialClicked();
	UFUNCTION() void HandleTutorialClosed();
	UFUNCTION() void HandleAchievementsClicked();
	UFUNCTION() void HandleAchievementsClosed();
	UFUNCTION() void HandlePreviousPage();
	UFUNCTION() void HandleNextPage();

	void SetTutorialOpen(bool bOpen);
	void SetAchievementsOpen(bool bOpen);
	void SetTutorialPage(int32 NewPageIndex);
	void HandleSwipeEnd(const FVector2D& EndPosition);

	UPROPERTY(Transient) TObjectPtr<UVerticalBox> TitleMenu;
	UPROPERTY(Transient) TObjectPtr<UBorder> TutorialPanel;
	UPROPERTY(Transient) TObjectPtr<UBorder> AchievementsPanel;
	UPROPERTY(Transient) TObjectPtr<UImage> TutorialImage;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TutorialTitle;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TutorialBody;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TutorialPageIndicator;
	UPROPERTY(Transient) TObjectPtr<UButton> PreviousPageButton;
	UPROPERTY(Transient) TObjectPtr<UButton> NextPageButton;
	UPROPERTY(Transient) TArray<TObjectPtr<UTexture2D>> TutorialTextures;

	int32 CurrentTutorialPage = 0;
	FVector2D SwipeStartPosition = FVector2D::ZeroVector;
	bool bTrackingSwipe = false;
};
