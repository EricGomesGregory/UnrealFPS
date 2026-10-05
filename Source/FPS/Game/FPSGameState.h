// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FPSGameState.generated.h"

class AFPSPlayerState;


/**
 * 
 */
UCLASS()
class FPS_API AFPSGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AFPSGameState();
	
	bool HasFirstBloodTriggered() const { return bHasFirstBloodTriggered; }
	
	void UpdateLeader();
	
	AFPSPlayerState* GetSoleLeader() const;
	
	bool IsTiedForTheLead(AFPSPlayerState* PlayerState) const;
	
private:
	bool bHasFirstBloodTriggered;
	
	UPROPERTY()
	TArray<TObjectPtr<AFPSPlayerState>> Leaders;
};
