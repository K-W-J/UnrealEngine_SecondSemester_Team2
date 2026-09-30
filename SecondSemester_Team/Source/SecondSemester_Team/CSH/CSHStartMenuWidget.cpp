#include "CSHStartMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CSH/SecondSemester_TeamCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

TSharedRef<SWidget> UCSHStartMenuWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("TitleCanvas"));
		WidgetTree->RootWidget = Canvas;

		UBorder* DimOverlay = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), TEXT("TitleDimOverlay"));
		DimOverlay->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.36f));
		UCanvasPanelSlot* DimSlot = Canvas->AddChildToCanvas(DimOverlay);
		DimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		DimSlot->SetOffsets(FMargin(0.0f));

		UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("TitleMenu"));
		UCanvasPanelSlot* MenuSlot = Canvas->AddChildToCanvas(Menu);
		MenuSlot->SetAnchors(FAnchors(0.5f, 0.48f));
		MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MenuSlot->SetSize(FVector2D(520.0f, 410.0f));

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("TitleText"));
		Title->SetText(FText::FromString(TEXT("LAST DRIVE")));
		Title->SetJustification(ETextJustify::Center);
		Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.92f, 0.90f, 1.0f)));
		Title->SetShadowOffset(FVector2D(3.0f, 4.0f));
		Title->SetShadowColorAndOpacity(FLinearColor(0.45f, 0.015f, 0.005f, 1.0f));
		FSlateFontInfo TitleFont = Title->GetFont();
		TitleFont.Size = 64;
		Title->SetFont(TitleFont);
		UVerticalBoxSlot* TitleSlot = Menu->AddChildToVerticalBox(Title);
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 76.0f));

		auto AddMenuButton = [this, Menu](const TCHAR* Name, const TCHAR* Label, const bool bStartButton)
		{
			UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
			Button->SetBackgroundColor(bStartButton
				? FLinearColor(0.48f, 0.06f, 0.025f, 0.96f)
				: FLinearColor(0.08f, 0.08f, 0.095f, 0.94f));
			if (bStartButton)
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleStartClicked);
			}
			else
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHStartMenuWidget::HandleQuitClicked);
			}

			UTextBlock* LabelWidget = WidgetTree->ConstructWidget<UTextBlock>();
			LabelWidget->SetText(FText::FromString(Label));
			LabelWidget->SetJustification(ETextJustify::Center);
			LabelWidget->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			FSlateFontInfo LabelFont = LabelWidget->GetFont();
			LabelFont.Size = 25;
			LabelWidget->SetFont(LabelFont);
			Button->AddChild(LabelWidget);

			UVerticalBoxSlot* ButtonSlot = Menu->AddChildToVerticalBox(Button);
			ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
			ButtonSlot->SetPadding(FMargin(70.0f, 0.0f, 70.0f, 16.0f));
			ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		};

		AddMenuButton(TEXT("StartButton"), TEXT("START"), true);
		AddMenuButton(TEXT("QuitButton"), TEXT("QUIT"), false);
	}

	return Super::RebuildWidget();
}

void UCSHStartMenuWidget::HandleStartClicked()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ASecondSemester_TeamCharacter* Character =
			Cast<ASecondSemester_TeamCharacter>(PlayerController->GetPawn()))
		{
			Character->FinishTitleScreen();
		}
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
