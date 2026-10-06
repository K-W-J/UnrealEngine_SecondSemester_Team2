#include "Enemies/SS_WaveWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

TSharedRef<SWidget> USS_WaveWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("WaveCanvas"));
		WidgetTree->RootWidget = Canvas;

		UOverlay* WaveBarOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(), TEXT("WaveBarOverlay"));
		UCanvasPanelSlot* WaveBarSlot = Canvas->AddChildToCanvas(WaveBarOverlay);
		WaveBarSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		WaveBarSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		WaveBarSlot->SetPosition(FVector2D(0.0f, 24.0f));
		WaveBarSlot->SetSize(FVector2D(520.0f, 44.0f));

		WaveProgressBar = WidgetTree->ConstructWidget<UProgressBar>(
			UProgressBar::StaticClass(), TEXT("WaveProgressBar"));
		WaveProgressBar->SetPercent(FMath::Clamp(CurrentWaveProgress, 0.0f, 1.0f));
		WaveProgressBar->SetFillColorAndOpacity(FLinearColor(0.08f, 0.55f, 0.9f, 1.0f));
		UOverlaySlot* ProgressSlot = WaveBarOverlay->AddChildToOverlay(WaveProgressBar);
		ProgressSlot->SetHorizontalAlignment(HAlign_Fill);
		ProgressSlot->SetVerticalAlignment(VAlign_Fill);

		WaveText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("WaveText"));
		WaveText->SetJustification(ETextJustify::Center);
		WaveText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		WaveText->SetShadowOffset(FVector2D(1.0f, 2.0f));
		WaveText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
		FSlateFontInfo Font = WaveText->GetFont();
		Font.Size = 25;
		WaveText->SetFont(Font);
		WaveText->SetText(FText::FromString(CurrentLabel));
		WaveText->SetVisibility(ESlateVisibility::HitTestInvisible);
		UOverlaySlot* TextSlot = WaveBarOverlay->AddChildToOverlay(WaveText);
		TextSlot->SetHorizontalAlignment(HAlign_Fill);
		TextSlot->SetVerticalAlignment(VAlign_Center);

		BossBarsBox = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("BossBarsBox"));
		BossBarsBox->SetVisibility(ESlateVisibility::Collapsed);
		UCanvasPanelSlot* BossSlot = Canvas->AddChildToCanvas(BossBarsBox);
		BossSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		BossSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		BossSlot->SetPosition(FVector2D(0.0f, 78.0f));
		BossSlot->SetSize(FVector2D(560.0f, 260.0f));
		RebuildBossBars();
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	return Super::RebuildWidget();
}

void USS_WaveWidget::SetWaveProgress(float Progress)
{
	CurrentWaveProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	if (WaveProgressBar)
	{
		WaveProgressBar->SetPercent(CurrentWaveProgress);
	}
}

void USS_WaveWidget::SetBossHealthValues(const TArray<float>& HealthValues)
{
	CurrentBossHealthValues = HealthValues;
	if (!BossBarsBox)
	{
		return;
	}
	if (BossHealthBars.Num() != CurrentBossHealthValues.Num())
	{
		RebuildBossBars();
	}
	for (int32 Index = 0; Index < BossHealthBars.Num(); ++Index)
	{
		BossHealthBars[Index]->SetPercent(FMath::Clamp(CurrentBossHealthValues[Index], 0.0f, 1.0f));
	}
	BossBarsBox->SetVisibility(BossHealthBars.IsEmpty()
		? ESlateVisibility::Collapsed
		: ESlateVisibility::HitTestInvisible);
}

void USS_WaveWidget::RebuildBossBars()
{
	if (!BossBarsBox || !WidgetTree)
	{
		return;
	}
	BossBarsBox->ClearChildren();
	BossHealthBars.Reset();

	for (int32 Index = 0; Index < CurrentBossHealthValues.Num(); ++Index)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		UVerticalBoxSlot* RowSlot = BossBarsBox->AddChildToVerticalBox(Row);
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
		RowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 7.0f));
		RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
		Label->SetText(FText::FromString(FString::Printf(TEXT("BOSS %d"), Index + 1)));
		Label->SetJustification(ETextJustify::Right);
		Label->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.08f, 1.0f)));
		Label->SetShadowOffset(FVector2D(1.0f, 1.0f));
		Label->SetShadowColorAndOpacity(FLinearColor::Black);
		FSlateFontInfo LabelFont = Label->GetFont();
		LabelFont.Size = 18;
		Label->SetFont(LabelFont);
		UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label);
		LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		LabelSlot->SetPadding(FMargin(0.0f, 1.0f, 12.0f, 0.0f));
		LabelSlot->SetVerticalAlignment(VAlign_Center);

		UProgressBar* HealthBar = WidgetTree->ConstructWidget<UProgressBar>();
		HealthBar->SetPercent(FMath::Clamp(CurrentBossHealthValues[Index], 0.0f, 1.0f));
		HealthBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.025f, 0.01f, 1.0f));
		UHorizontalBoxSlot* HealthSlot = Row->AddChildToHorizontalBox(HealthBar);
		HealthSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		HealthSlot->SetVerticalAlignment(VAlign_Center);
		HealthSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 4.0f));
		BossHealthBars.Add(HealthBar);
	}

	BossBarsBox->SetVisibility(BossHealthBars.IsEmpty()
		? ESlateVisibility::Collapsed
		: ESlateVisibility::HitTestInvisible);
}

void USS_WaveWidget::SetWaveLabel(const FString& Label)
{
	CurrentLabel = Label;
	if (WaveText)
	{
		WaveText->SetText(FText::FromString(CurrentLabel));
	}
}
