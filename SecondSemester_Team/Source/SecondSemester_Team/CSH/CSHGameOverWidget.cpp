#include "CSHGameOverWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

TSharedRef<SWidget> UCSHGameOverWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("GameOverCanvas"));
		WidgetTree->RootWidget = Canvas;

		UBorder* DimBackground = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), TEXT("DimBackground"));
		DimBackground->SetBrushColor(FLinearColor(0.005f, 0.005f, 0.008f, 0.82f));
		UCanvasPanelSlot* BackgroundSlot = Canvas->AddChildToCanvas(DimBackground);
		BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackgroundSlot->SetOffsets(FMargin(0.0f));

		UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("GameOverMenu"));
		UCanvasPanelSlot* MenuSlot = Canvas->AddChildToCanvas(Menu);
		MenuSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MenuSlot->SetPosition(FVector2D::ZeroVector);
		MenuSlot->SetSize(FVector2D(420.0f, 310.0f));

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("GameOverTitle"));
		Title->SetText(FText::FromString(TEXT("GAME OVER")));
		Title->SetJustification(ETextJustify::Center);
		Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.025f, 0.025f, 1.0f)));
		Title->SetShadowOffset(FVector2D(2.0f, 3.0f));
		Title->SetShadowColorAndOpacity(FLinearColor::Black);
		FSlateFontInfo TitleFont = Title->GetFont();
		TitleFont.Size = 56;
		Title->SetFont(TitleFont);
		UVerticalBoxSlot* TitleSlot = Menu->AddChildToVerticalBox(Title);
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 42.0f));

		auto AddMenuButton = [this, Menu](const TCHAR* Name, const TCHAR* Label,
			void (UCSHGameOverWidget::*Handler)())
		{
			UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
			Button->SetBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 0.98f));
			Button->SetColorAndOpacity(FLinearColor::White);
			if (Handler == &UCSHGameOverWidget::HandleRestartClicked)
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHGameOverWidget::HandleRestartClicked);
			}
			else
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHGameOverWidget::HandleQuitClicked);
			}

			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>();
			ButtonText->SetText(FText::FromString(Label));
			ButtonText->SetJustification(ETextJustify::Center);
			ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			FSlateFontInfo ButtonFont = ButtonText->GetFont();
			ButtonFont.Size = 24;
			ButtonText->SetFont(ButtonFont);
			Button->AddChild(ButtonText);

			UVerticalBoxSlot* ButtonSlot = Menu->AddChildToVerticalBox(Button);
			ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
			ButtonSlot->SetPadding(FMargin(30.0f, 0.0f, 30.0f, 14.0f));
			ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		};

		AddMenuButton(TEXT("RestartButton"), TEXT("RESTART"),
			&UCSHGameOverWidget::HandleRestartClicked);
		AddMenuButton(TEXT("QuitButton"), TEXT("QUIT"),
			&UCSHGameOverWidget::HandleQuitClicked);
	}

	return Super::RebuildWidget();
}

void UCSHGameOverWidget::HandleRestartClicked()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (CurrentLevelName.IsEmpty())
	{
		return;
	}

	UGameplayStatics::SetGamePaused(this, false);
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName), true, TEXT("SkipTitle=1"));
}

void UCSHGameOverWidget::HandleQuitClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
