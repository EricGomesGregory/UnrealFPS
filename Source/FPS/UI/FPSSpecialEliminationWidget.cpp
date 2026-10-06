// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSSpecialEliminationWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"


void UFPSSpecialEliminationWidget::InitializeWidget(const FString& EliminationMessage, UTexture2D* IconTexture)
{
	check(IconTexture);
	
	Text_Message->SetText(FText::FromString(EliminationMessage));
	Image_Icon->SetBrushFromTexture(IconTexture);
}

void UFPSSpecialEliminationWidget::CenterWidget(UUserWidget* Widget, float VerticalOffset)
{
	check(Widget);

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
	const float VerticalFraction = ((VerticalOffset == 0.0f) ? 1.0f : VerticalOffset * 2.0f);
	const FVector2D CenterPosition = FVector2D(ViewportSize.X / 2.0f, VerticalFraction * (ViewportSize.Y / 2.0f));
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widget->SetPositionInViewport(CenterPosition, true);
}
