#include "CSHStartMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CSH/SecondSemester_TeamCharacter.h"
#include "CSHScoreSaveGame.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr int32 TutorialPageCount = 4;
	constexpr float SwipeThreshold = 90.0f;

	const TCHAR* TutorialTitles[TutorialPageCount] =
	{
		TEXT("김진호 · 마지막 정비공"), TEXT("살아남아라"),
		TEXT("무기 상자를 찾아라"), TEXT("웨이브를 돌파하라")
	};

	const TCHAR* TutorialBodies[TutorialPageCount] =
	{
		TEXT("2077년, 원인 불명의 바이러스로 AI 자동차들이 사람을 공격하기 시작했습니다.\n\n자동차 정비공 김진호가 되어 무너진 도시에서 살아남으세요."),
		TEXT("WASD  이동    마우스  시점\nSHIFT  달리기    SPACE  점프\n마우스 왼쪽  공격    R  재장전 / 특수 기능\n\n자동차는 플레이어를 향해 돌진합니다. 충돌 직전에 옆으로 피하고 빈틈을 노려 공격하세요. 무기가 없을 때는 주먹으로 싸웁니다."),
		TEXT("웨이브가 진행되는 동안 무기 상자를 최대한 많이 열어 높은 포인트를 얻는 것이 핵심 목표입니다. 레이더를 확인해 상자를 계속 찾아가세요.\n\n상자를 열면 점수와 무작위 무기 하나를 얻습니다. 새 무기를 얻으면 기존 무기는 교체됩니다. 총, 근접 무기, 자동차를 꽂아 날리는 꼬챙이의 특성을 활용하세요."),
		TEXT("화면 위 웨이브 바는 남은 적의 수를 보여줍니다. 웨이브가 올라갈수록 더 강하고 다양한 자동차가 등장합니다.\n\n보스가 등장하면 웨이브 바 아래 체력바를 확인하세요. 모든 웨이브와 보스를 처치하면 게임을 클리어합니다.")
	};

	UTextBlock* MakeText(UWidgetTree* Tree, const TCHAR* Name, const FString& Text,
		int32 Size, const FLinearColor& Color, ETextJustify::Type Justification = ETextJustify::Left)
	{
		UTextBlock* Result = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Result->SetText(FText::FromString(Text));
		Result->SetColorAndOpacity(FSlateColor(Color));
		Result->SetJustification(Justification);
		Result->SetAutoWrapText(true);
		FSlateFontInfo Font = Result->GetFont();
		Font.Size = Size;
		Result->SetFont(Font);
		return Result;
	}
}

UCSHStartMenuWidget::UCSHStartMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> Story(TEXT("/Game/CSH/UI/Tutorial/T_Tutorial_Story.T_Tutorial_Story"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> Survival(TEXT("/Game/CSH/UI/Tutorial/T_Tutorial_Survival.T_Tutorial_Survival"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> Weapons(TEXT("/Game/CSH/UI/Tutorial/T_Tutorial_Weapons.T_Tutorial_Weapons"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> Waves(TEXT("/Game/CSH/UI/Tutorial/T_Tutorial_Waves.T_Tutorial_Waves"));
	TutorialTextures = { Story.Object, Survival.Object, Weapons.Object, Waves.Object };
}

TSharedRef<SWidget> UCSHStartMenuWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TitleCanvas"));
		WidgetTree->RootWidget = Canvas;

		UBorder* DimOverlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TitleDimOverlay"));
		DimOverlay->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.36f));
		UCanvasPanelSlot* DimSlot = Canvas->AddChildToCanvas(DimOverlay);
		DimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		DimSlot->SetOffsets(FMargin(0.0f));

		TitleMenu = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TitleMenu"));
		UCanvasPanelSlot* MenuSlot = Canvas->AddChildToCanvas(TitleMenu);
		MenuSlot->SetAnchors(FAnchors(0.5f, 0.48f));
		MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MenuSlot->SetSize(FVector2D(520.0f, 570.0f));

		UTextBlock* Title = MakeText(WidgetTree, TEXT("TitleText"), TEXT("THE MAD CARS"), 64,
			FLinearColor(0.92f, 0.92f, 0.90f, 1.0f), ETextJustify::Center);
		Title->SetShadowOffset(FVector2D(3.0f, 4.0f));
		Title->SetShadowColorAndOpacity(FLinearColor(0.45f, 0.015f, 0.005f, 1.0f));
		UVerticalBoxSlot* TitleSlot = TitleMenu->AddChildToVerticalBox(Title);
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));

		UTextBlock* BestScore = MakeText(WidgetTree, TEXT("BestScoreText"),
			FString::Printf(TEXT("BEST SCORE  %d POINT"), UCSHScoreSaveGame::LoadHighestPointCount()), 28,
			FLinearColor(0.95f, 0.72f, 0.12f, 1.0f), ETextJustify::Center);
		BestScore->SetShadowOffset(FVector2D(1.0f, 2.0f));
		BestScore->SetShadowColorAndOpacity(FLinearColor::Black);
		UVerticalBoxSlot* BestScoreSlot = TitleMenu->AddChildToVerticalBox(BestScore);
		BestScoreSlot->SetHorizontalAlignment(HAlign_Fill);
		BestScoreSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 36.0f));

		auto AddMenuButton = [this](const TCHAR* Name, const TCHAR* Label, int32 Kind)
		{
			UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
			Button->SetBackgroundColor(Kind == 0 ? FLinearColor(0.48f, 0.06f, 0.025f, 0.96f)
				: FLinearColor(0.08f, 0.08f, 0.095f, 0.94f));
			if (Kind == 0) Button->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleStartClicked);
			else if (Kind == 1) Button->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleTutorialClicked);
			else if (Kind == 2) Button->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleAchievementsClicked);
			else Button->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleQuitClicked);
			Button->AddChild(MakeText(WidgetTree, *FString::Printf(TEXT("%sLabel"), Name), Label, 25,
				FLinearColor::White, ETextJustify::Center));
			UVerticalBoxSlot* ButtonSlot = TitleMenu->AddChildToVerticalBox(Button);
			ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
			ButtonSlot->SetPadding(FMargin(70.0f, 0.0f, 70.0f, 14.0f));
			ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		};

		AddMenuButton(TEXT("StartButton"), TEXT("START"), 0);
		AddMenuButton(TEXT("TutorialButton"), TEXT("TUTORIAL"), 1);
		AddMenuButton(TEXT("AchievementsButton"), TEXT("ACHIEVEMENTS"), 2);
		AddMenuButton(TEXT("QuitButton"), TEXT("QUIT"), 3);

		TutorialPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TutorialPanel"));
		TutorialPanel->SetBrushColor(FLinearColor(0.012f, 0.012f, 0.018f, 0.97f));
		TutorialPanel->SetPadding(FMargin(34.0f));
		UCanvasPanelSlot* TutorialSlot = Canvas->AddChildToCanvas(TutorialPanel);
		TutorialSlot->SetAnchors(FAnchors(0.07f, 0.07f, 0.93f, 0.93f));
		TutorialSlot->SetOffsets(FMargin(0.0f));
		TutorialPanel->SetVisibility(ESlateVisibility::Collapsed);

		UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TutorialLayout"));
		TutorialPanel->AddChild(Layout);

		UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TutorialHeader"));
		UVerticalBoxSlot* HeaderSlot = Layout->AddChildToVerticalBox(Header);
		HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
		Header->AddChildToHorizontalBox(MakeText(WidgetTree, TEXT("TutorialHeaderText"), TEXT("SURVIVAL GUIDE"), 31,
			FLinearColor(0.92f, 0.12f, 0.06f, 1.0f)))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		UButton* CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("TutorialCloseButton"));
		CloseButton->SetBackgroundColor(FLinearColor(0.14f, 0.14f, 0.16f, 1.0f));
		CloseButton->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleTutorialClosed);
		CloseButton->AddChild(MakeText(WidgetTree, TEXT("TutorialCloseLabel"), TEXT("BACK"), 20,
			FLinearColor::White, ETextJustify::Center));
		UHorizontalBoxSlot* CloseSlot = Header->AddChildToHorizontalBox(CloseButton);
		CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		CloseSlot->SetPadding(FMargin(24.0f, 0.0f, 0.0f, 0.0f));

		UHorizontalBox* Content = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TutorialContent"));
		UVerticalBoxSlot* ContentSlot = Layout->AddChildToVerticalBox(Content);
		ContentSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		USizeBox* ImageBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TutorialImageBox"));
		ImageBox->SetWidthOverride(840.0f);
		ImageBox->SetHeightOverride(472.5f);
		TutorialImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("TutorialImage"));
		TutorialImage->SetDesiredSizeOverride(FVector2D(840.0f, 472.5f));
		ImageBox->AddChild(TutorialImage);
		UHorizontalBoxSlot* ImageSlot = Content->AddChildToHorizontalBox(ImageBox);
		ImageSlot->SetPadding(FMargin(0.0f, 0.0f, 34.0f, 0.0f));
		ImageSlot->SetVerticalAlignment(VAlign_Center);

		UVerticalBox* Copy = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TutorialCopy"));
		UHorizontalBoxSlot* CopySlot = Content->AddChildToHorizontalBox(Copy);
		CopySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CopySlot->SetVerticalAlignment(VAlign_Center);
		TutorialTitle = MakeText(WidgetTree, TEXT("TutorialPageTitle"), TEXT(""), 34,
			FLinearColor(0.98f, 0.82f, 0.68f, 1.0f));
		Copy->AddChildToVerticalBox(TutorialTitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
		TutorialBody = MakeText(WidgetTree, TEXT("TutorialPageBody"), TEXT(""), 21,
			FLinearColor(0.88f, 0.88f, 0.90f, 1.0f));
		TutorialBody->SetLineHeightPercentage(1.25f);
		Copy->AddChildToVerticalBox(TutorialBody)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		UHorizontalBox* Footer = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TutorialFooter"));
		Layout->AddChildToVerticalBox(Footer)->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 0.0f));
		PreviousPageButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PreviousPageButton"));
		PreviousPageButton->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandlePreviousPage);
		PreviousPageButton->AddChild(MakeText(WidgetTree, TEXT("PreviousPageLabel"), TEXT("<  PREV"), 20,
			FLinearColor::White, ETextJustify::Center));
		Footer->AddChildToHorizontalBox(PreviousPageButton)->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

		TutorialPageIndicator = MakeText(WidgetTree, TEXT("TutorialPageIndicator"), TEXT(""), 18,
			FLinearColor(0.72f, 0.72f, 0.76f, 1.0f), ETextJustify::Center);
		UHorizontalBoxSlot* IndicatorSlot = Footer->AddChildToHorizontalBox(TutorialPageIndicator);
		IndicatorSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		IndicatorSlot->SetPadding(FMargin(20.0f, 4.0f));

		NextPageButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NextPageButton"));
		NextPageButton->SetBackgroundColor(FLinearColor(0.48f, 0.06f, 0.025f, 1.0f));
		NextPageButton->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleNextPage);
		NextPageButton->AddChild(MakeText(WidgetTree, TEXT("NextPageLabel"), TEXT("NEXT  >"), 20,
			FLinearColor::White, ETextJustify::Center));
		Footer->AddChildToHorizontalBox(NextPageButton)->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

		AchievementsPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AchievementsPanel"));
		AchievementsPanel->SetBrushColor(FLinearColor(0.012f, 0.012f, 0.018f, 0.97f));
		AchievementsPanel->SetPadding(FMargin(54.0f));
		UCanvasPanelSlot* AchievementsSlot = Canvas->AddChildToCanvas(AchievementsPanel);
		AchievementsSlot->SetAnchors(FAnchors(0.18f, 0.11f, 0.82f, 0.89f));
		AchievementsSlot->SetOffsets(FMargin(0.0f));
		AchievementsPanel->SetVisibility(ESlateVisibility::Collapsed);

		UVerticalBox* AchievementsLayout = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("AchievementsLayout"));
		AchievementsPanel->AddChild(AchievementsLayout);

		UHorizontalBox* AchievementsHeader = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("AchievementsHeader"));
		UVerticalBoxSlot* AchievementsHeaderSlot = AchievementsLayout->AddChildToVerticalBox(AchievementsHeader);
		AchievementsHeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
		AchievementsHeader->AddChildToHorizontalBox(MakeText(WidgetTree, TEXT("AchievementsHeaderText"),
			TEXT("ACHIEVEMENTS"), 38, FLinearColor(0.95f, 0.72f, 0.12f, 1.0f)))->SetSize(
				FSlateChildSize(ESlateSizeRule::Fill));
		UButton* AchievementsBack = WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(), TEXT("AchievementsBackButton"));
		AchievementsBack->SetBackgroundColor(FLinearColor(0.14f, 0.14f, 0.16f, 1.0f));
		AchievementsBack->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleAchievementsClosed);
		AchievementsBack->AddChild(MakeText(WidgetTree, TEXT("AchievementsBackLabel"), TEXT("BACK"), 20,
			FLinearColor::White, ETextJustify::Center));
		AchievementsHeader->AddChildToHorizontalBox(AchievementsBack)->SetSize(
			FSlateChildSize(ESlateSizeRule::Automatic));

		auto AddAchievement = [this, AchievementsLayout](const TCHAR* Name, const TCHAR* TitleText,
			const TCHAR* Description)
		{
			UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
			Card->SetBrushColor(FLinearColor(0.065f, 0.065f, 0.08f, 0.98f));
			Card->SetPadding(FMargin(24.0f, 18.0f));
			UVerticalBox* CardLayout = WidgetTree->ConstructWidget<UVerticalBox>();
			Card->AddChild(CardLayout);
			UTextBlock* AchievementTitle = MakeText(WidgetTree,
				*FString::Printf(TEXT("%sTitle"), Name), FString::Printf(TEXT("◇  %s"), TitleText), 26,
				FLinearColor(0.96f, 0.82f, 0.42f, 1.0f));
			CardLayout->AddChildToVerticalBox(AchievementTitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
			CardLayout->AddChildToVerticalBox(MakeText(WidgetTree,
				*FString::Printf(TEXT("%sDescription"), Name), Description, 20,
				FLinearColor(0.88f, 0.88f, 0.90f, 1.0f)));
			UVerticalBoxSlot* CardSlot = AchievementsLayout->AddChildToVerticalBox(Card);
			CardSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
			CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		};

		AddAchievement(TEXT("FistsOnlyAchievement"), TEXT("맨주먹의 생존자"),
			TEXT("주먹만 사용해 모든 웨이브를 클리어하고 엔딩 보기"));
		AddAchievement(TEXT("WeaponBoxAchievement"), TEXT("상자 사냥꾼"),
			TEXT("무기 상자를 총 100개 열기"));
		AddAchievement(TEXT("CollisionOnlyAchievement"), TEXT("충돌의 지배자"),
			TEXT("자동차 충돌 데미지만 이용해 모든 웨이브를 클리어하고 엔딩 보기"));

		SetTutorialPage(0);
	}
	return Super::RebuildWidget();
}

void UCSHStartMenuWidget::SetTutorialOpen(bool bOpen)
{
	if (TitleMenu) TitleMenu->SetVisibility(bOpen ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (TutorialPanel) TutorialPanel->SetVisibility(bOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (bOpen) SetTutorialPage(0);
}

void UCSHStartMenuWidget::SetAchievementsOpen(bool bOpen)
{
	if (TitleMenu) TitleMenu->SetVisibility(bOpen ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (AchievementsPanel) AchievementsPanel->SetVisibility(bOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCSHStartMenuWidget::SetTutorialPage(int32 NewPageIndex)
{
	CurrentTutorialPage = FMath::Clamp(NewPageIndex, 0, TutorialPageCount - 1);
	if (TutorialTitle) TutorialTitle->SetText(FText::FromString(TutorialTitles[CurrentTutorialPage]));
	if (TutorialBody) TutorialBody->SetText(FText::FromString(TutorialBodies[CurrentTutorialPage]));
	if (TutorialImage && TutorialTextures.IsValidIndex(CurrentTutorialPage) && TutorialTextures[CurrentTutorialPage])
		TutorialImage->SetBrushFromTexture(TutorialTextures[CurrentTutorialPage], true);
	if (TutorialPageIndicator)
		TutorialPageIndicator->SetText(FText::FromString(FString::Printf(
			TEXT("%d / %d    ·    화면을 왼쪽/오른쪽으로 넘기세요"), CurrentTutorialPage + 1, TutorialPageCount)));
	if (PreviousPageButton) PreviousPageButton->SetIsEnabled(CurrentTutorialPage > 0);
	if (NextPageButton) NextPageButton->SetIsEnabled(CurrentTutorialPage < TutorialPageCount - 1);
}

void UCSHStartMenuWidget::HandleStartClicked()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ASecondSemester_TeamCharacter* Character = Cast<ASecondSemester_TeamCharacter>(PlayerController->GetPawn()))
			Character->FinishTitleScreen();
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
	UGameplayStatics::SetGamePaused(this, false);
	RemoveFromParent();
}

void UCSHStartMenuWidget::HandleQuitClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UCSHStartMenuWidget::HandleTutorialClicked() { SetTutorialOpen(true); }
void UCSHStartMenuWidget::HandleTutorialClosed() { SetTutorialOpen(false); }
void UCSHStartMenuWidget::HandleAchievementsClicked() { SetAchievementsOpen(true); }
void UCSHStartMenuWidget::HandleAchievementsClosed() { SetAchievementsOpen(false); }
void UCSHStartMenuWidget::HandlePreviousPage() { SetTutorialPage(CurrentTutorialPage - 1); }
void UCSHStartMenuWidget::HandleNextPage() { SetTutorialPage(CurrentTutorialPage + 1); }

FReply UCSHStartMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (TutorialPanel && TutorialPanel->GetVisibility() == ESlateVisibility::Visible
		&& InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SwipeStartPosition = InMouseEvent.GetScreenSpacePosition();
		bTrackingSwipe = true;
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCSHStartMenuWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bTrackingSwipe)
	{
		bTrackingSwipe = false;
		HandleSwipeEnd(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled().ReleaseMouseCapture();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UCSHStartMenuWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (TutorialPanel && TutorialPanel->GetVisibility() == ESlateVisibility::Visible)
	{
		SwipeStartPosition = InGestureEvent.GetScreenSpacePosition();
		bTrackingSwipe = true;
		return FReply::Handled();
	}
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply UCSHStartMenuWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bTrackingSwipe)
	{
		bTrackingSwipe = false;
		HandleSwipeEnd(InGestureEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

void UCSHStartMenuWidget::HandleSwipeEnd(const FVector2D& EndPosition)
{
	const float Distance = EndPosition.X - SwipeStartPosition.X;
	if (Distance <= -SwipeThreshold) HandleNextPage();
	else if (Distance >= SwipeThreshold) HandlePreviousPage();
}
