#include "CSHDamageBorderWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Brushes/SlateColorBrush.h"

TSharedRef<SWidget> UCSHDamageBorderWidget::RebuildWidget()
{
	if (!WidgetTree || !EdgeBorders.IsEmpty())
	{
		return Super::RebuildWidget();
	}
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("DamageBorderCanvas"));
	WidgetTree->RootWidget = Canvas;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// Three translucent bands on each edge soften the screen border.
	const float Thicknesses[] = {10.0f, 20.0f, 36.0f};
	const float Offsets[] = {0.0f, 10.0f, 30.0f};
	for (int32 Band = 0; Band < 3; ++Band)
	{
		for (int32 Edge = 0; Edge < 4; ++Edge)
		{
			UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
			Border->SetBrush(FSlateColorBrush(FLinearColor::White));
			Border->SetVisibility(ESlateVisibility::HitTestInvisible);
			UCanvasPanelSlot* BorderSlot = Canvas->AddChildToCanvas(Border);
			const float Thickness = Thicknesses[Band];
			const float Offset = Offsets[Band];
			switch (Edge)
			{
			case 0: // top
				BorderSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 0.0f));
				BorderSlot->SetOffsets(FMargin(0.0f, Offset, 0.0f, Thickness));
				break;
			case 1: // bottom
				BorderSlot->SetAnchors(FAnchors(0.0f, 1.0f, 1.0f, 1.0f));
				BorderSlot->SetOffsets(FMargin(0.0f, -Offset - Thickness, 0.0f, Thickness));
				break;
			case 2: // left
				BorderSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
				BorderSlot->SetOffsets(FMargin(Offset, 0.0f, Thickness, 0.0f));
				break;
			default: // right
				BorderSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 1.0f));
				BorderSlot->SetOffsets(FMargin(-Offset - Thickness, 0.0f, Thickness, 0.0f));
				break;
			}
			EdgeBorders.Add(Border);
		}
	}
	RefreshColors();
	return Super::RebuildWidget();
}

void UCSHDamageBorderWidget::SetFlashOpacity(float InOpacity)
{
	FlashOpacity = FMath::Clamp(InOpacity, 0.0f, 1.0f);
	RefreshColors();
}

void UCSHDamageBorderWidget::SetFlashTint(const FLinearColor& InTint)
{
	FlashTint = InTint;
	RefreshColors();
}

void UCSHDamageBorderWidget::RefreshColors()
{
	const float BandOpacities[] = {0.55f, 0.22f, 0.07f};
	for (int32 Index = 0; Index < EdgeBorders.Num(); ++Index)
	{
		if (UBorder* Border = EdgeBorders[Index])
		{
			FLinearColor Color = FlashTint;
			Color.A = FlashTint.A * FlashOpacity * BandOpacities[Index / 4];
			Border->SetBrushColor(Color);
		}
	}
}
