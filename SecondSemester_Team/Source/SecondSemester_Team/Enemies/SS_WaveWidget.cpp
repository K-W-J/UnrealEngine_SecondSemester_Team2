#include "Enemies/SS_WaveWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> USS_WaveWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("WaveCanvas"));
		WidgetTree->RootWidget = Canvas;
		WaveText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("WaveText"));
		WaveText->SetJustification(ETextJustify::Center);
		WaveText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		WaveText->SetShadowOffset(FVector2D(1.0f, 2.0f));
		WaveText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
		FSlateFontInfo Font = WaveText->GetFont();
		Font.Size = 32;
		WaveText->SetFont(Font);
		WaveText->SetText(FText::FromString(CurrentLabel));
		WaveText->SetVisibility(ESlateVisibility::HitTestInvisible);

		UCanvasPanelSlot* TextSlot = Canvas->AddChildToCanvas(WaveText);
		TextSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		TextSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		TextSlot->SetPosition(FVector2D(0.0f, 24.0f));
		TextSlot->SetSize(FVector2D(420.0f, 52.0f));
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	return Super::RebuildWidget();
}

void USS_WaveWidget::SetWaveLabel(const FString& Label)
{
	CurrentLabel = Label;
	if (WaveText)
	{
		WaveText->SetText(FText::FromString(CurrentLabel));
	}
}
