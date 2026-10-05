// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AFPSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AFPSPlayerState();
	
	void AddElimination(bool bFromHeadShot = false);
	
	void AddDefeat();
	
	void AddHit();
	
	void AddMiss();
	
	void AddSequentialElimination(int32 Count);
	
	void UpdateHighestStreak(int32 StreakCount);
	
	void AddRevengeElimination();
	
	void AddDethroneElimination();
	
	void AddShowStopper();
	
	void FirstBlood();
	
	void SetWinner();
	
private:
	int32 Eliminations;
	
	int32 HeadShotEliminations;
	
	int32 Defeats;
	
	int32 RoundsHit;
	
	int32 RoundsMissed;
	
	bool bOnEliminationStreak;
	
	int32 HighestEliminationStreak;
	
	TMap<int32, int32> SequentialEliminations;
	
	int32 RevengeEliminations;
	
	int32 DethroneEliminations;
	
	int ShowStopperEliminations;
	
	bool bFirstBlood;
	
	bool bWinner;
};
