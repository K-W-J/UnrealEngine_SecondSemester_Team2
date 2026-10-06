#include "CSHEndingWidget.h"

#include "SecondSemester_TeamPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

TSharedRef<SWidget> UCSHEndingWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("EndingCanvas"));
		WidgetTree->RootWidget = Canvas;

		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), TEXT("EndingBackground"));
		Background->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.012f, 0.9f));
		UCanvasPanelSlot* BackgroundSlot = Canvas->AddChildToCanvas(Background);
		BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackgroundSlot->SetOffsets(FMargin(0.0f));

		UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("EndingMenu"));
		UCanvasPanelSlot* MenuSlot = Canvas->AddChildToCanvas(Menu);
		MenuSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MenuSlot->SetSize(FVector2D(560.0f, 410.0f));

		auto AddText = [this, Menu](const TCHAR* Name, const FString& Label,
			int32 FontSize, const FLinearColor& Color, float BottomPadding)
		{
			UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
			Text->SetText(FText::FromString(Label));
			Text->SetJustification(ETextJustify::Center);
			Text->SetColorAndOpacity(FSlateColor(Color));
			Text->SetShadowOffset(FVector2D(2.0f, 3.0f));
			Text->SetShadowColorAndOpacity(FLinearColor::Black);
			FSlateFontInfo Font = Text->GetFont();
			Font.Size = FontSize;
			Text->SetFont(Font);
			UVerticalBoxSlot* Slot = Menu->AddChildToVerticalBox(Text);
			Slot->SetHorizontalAlignment(HAlign_Fill);
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, BottomPadding));
		};

		AddText(TEXT("EndingTitle"), TEXT("ALL WAVES CLEARED"), 52,
			FLinearColor(0.95f, 0.72f, 0.12f, 1.0f), 14.0f);
		AddText(TEXT("EndingSubtitle"), TEXT("THE CITY SURVIVED"), 25,
			FLinearColor::White, 24.0f);

		int32 PointCount = 0;
		if (const ASecondSemester_TeamPlayerController* PlayerController =
			Cast<ASecondSemester_TeamPlayerController>(GetOwningPlayer()))
		{
			PointCount = PlayerController->GetWeaponBoxPointCount();
		}
		AddText(TEXT("EndingPoint"), FString::Printf(TEXT("FINAL SCORE  %d POINT"), PointCount), 40,
			FLinearColor(0.95f, 0.82f, 0.18f, 1.0f), 34.0f);

		auto AddButton = [this, Menu](const TCHAR* Name, const TCHAR* Label, bool bRestart)
		{
			UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
			Button->SetBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 0.98f));
			if (bRestart)
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHEndingWidget::HandleRestartClicked);
			}
			else
			{
				Button->OnClicked.AddUniqueDynamic(this, &UCSHEndingWidget::HandleQuitClicked);
			}
			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>();
			ButtonText->SetText(FText::FromString(Label));
			ButtonText->SetJustification(ETextJustify::Center);
			ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			FSlateFontInfo Font = ButtonText->GetFont();
			Font.Size = 24;
			ButtonText->SetFont(Font);
			Button->AddChild(ButtonText);
			UVerticalBoxSlot* Slot = Menu->AddChildToVerticalBox(Button);
			Slot->SetHorizontalAlignment(HAlign_Fill);
			Slot->SetPadding(FMargin(55.0f, 0.0f, 55.0f, 14.0f));
			Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		};

		AddButton(TEXT("EndingRestartButton"), TEXT("RESTART"), true);
		AddButton(TEXT("EndingQuitButton"), TEXT("QUIT"), false);
	}
	return Super::RebuildWidget();
}

void UCSHEndingWidget::HandleRestartClicked()
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (LevelName.IsEmpty())
	{
		return;
	}
	UGameplayStatics::SetGamePaused(this, false);
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
	UGameplayStatics::OpenLevel(this, FName(*LevelName), true, TEXT("SkipTitle=1"));
}

void UCSHEndingWidget::HandleQuitClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
