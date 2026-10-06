// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSSpecialEliminationWidget.generated.h"

class UImage;
class UTextBlock;


/**
 * 
 */
UCLASS()
class FPS_API UFPSSpecialEliminationWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Message;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;
	
	void InitializeWidget(const FString& EliminationMessage, UTexture2D* IconTexture);
	
	UFUNCTION(BlueprintCallable)
	static void CenterWidget(UUserWidget* Widget, float VerticalOffset = 0.0f);
};
