#include "CSHPauseMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SecondSemester_TeamCharacter.h"

namespace
{
	UTextBlock* MakePauseText(UWidgetTree* Tree, const TCHAR* Name, const TCHAR* Text, int32 Size)
	{
		UTextBlock* TextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		TextBlock->SetText(FText::FromString(Text));
		TextBlock->SetJustification(ETextJustify::Center);
		TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		TextBlock->SetShadowOffset(FVector2D(2.0f, 2.0f));
		TextBlock->SetShadowColorAndOpacity(FLinearColor::Black);
		FSlateFontInfo Font = TextBlock->GetFont();
		Font.Size = Size;
		TextBlock->SetFont(Font);
		return TextBlock;
	}
}

TSharedRef<SWidget> UCSHPauseMenuWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("PauseCanvas"));
		WidgetTree->RootWidget = Canvas;

		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), TEXT("PauseBackground"));
		Background->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));
		UCanvasPanelSlot* BackgroundSlot = Canvas->AddChildToCanvas(Background);
		BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackgroundSlot->SetOffsets(FMargin(0.0f));

		UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("PauseMenu"));
		UCanvasPanelSlot* MenuSlot = Canvas->AddChildToCanvas(Menu);
		MenuSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MenuSlot->SetSize(FVector2D(460.0f, 360.0f));

		UTextBlock* Title = MakePauseText(WidgetTree, TEXT("PauseTitle"), TEXT("PAUSED"), 52);
		Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.12f, 0.05f, 1.0f)));
		UVerticalBoxSlot* TitleSlot = Menu->AddChildToVerticalBox(Title);
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 36.0f));

		auto AddButton = [this, Menu](const TCHAR* Name, const TCHAR* Label, int32 Action)
		{
			UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
			Button->SetBackgroundColor(Action == 0
				? FLinearColor(0.48f, 0.06f, 0.025f, 0.98f)
				: FLinearColor(0.10f, 0.10f, 0.12f, 0.98f));
			if (Action == 0)
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHPauseMenuWidget::HandleContinueClicked);
			}
			else if (Action == 1)
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHPauseMenuWidget::HandleTitleClicked);
			}
			else
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHPauseMenuWidget::HandleQuitClicked);
			}
			Button->AddChild(MakePauseText(
				WidgetTree, *FString::Printf(TEXT("%sLabel"), Name), Label, 25));
			UVerticalBoxSlot* ButtonSlot = Menu->AddChildToVerticalBox(Button);
			ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
			ButtonSlot->SetPadding(FMargin(45.0f, 0.0f, 45.0f, 14.0f));
			ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		};

		AddButton(TEXT("ContinueButton"), TEXT("CONTINUE"), 0);
		AddButton(TEXT("TitleButton"), TEXT("RETURN TO TITLE"), 1);
		AddButton(TEXT("QuitButton"), TEXT("QUIT GAME"), 2);
	}

	return Super::RebuildWidget();
}

void UCSHPauseMenuWidget::HandleContinueClicked()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ASecondSemester_TeamCharacter* Character =
			Cast<ASecondSemester_TeamCharacter>(PlayerController->GetPawn()))
		{
			Character->ResumeFromPauseMenu();
		}
	}
}

void UCSHPauseMenuWidget::HandleTitleClicked()
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (LevelName.IsEmpty())
	{
		return;
	}

	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

void UCSHPauseMenuWidget::HandleQuitClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
