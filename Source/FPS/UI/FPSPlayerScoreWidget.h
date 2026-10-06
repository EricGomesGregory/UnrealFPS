// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSPlayerScoreWidget.generated.h"

class UTextBlock;
class AFPSPlayerState;
/**
 * 
 */
UCLASS()
class FPS_API UFPSPlayerScoreWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	
protected:
	UFUNCTION()
	void OnPlayerStateReplicated();
	
	UFUNCTION()
	void OnScoreEliminationChanged(int32 NewEliminationCount);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Score;
	
private:
	AFPSPlayerState* GetPlayerState() const;
};
