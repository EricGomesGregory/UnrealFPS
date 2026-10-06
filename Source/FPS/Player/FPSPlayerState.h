// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/FPSTypes.h"
#include "FPS/Elimination/SpecialEliminationsData.h"
#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

class UFPSSpecialEliminationWidget;
class USpecialEliminationsData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreEliminationDelegate, int32, Eliminations);


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
	
	int32 GetEliminations() const { return Eliminations; }
	
	void AddDefeat();
	
	int32 GetDefeats() const { return Defeats; }
	
	void AddHit();
	
	int32 GetHits() const { return RoundsHit; }
	
	void AddMiss();
	
	int32 GetMiss() const { return RoundsMissed; }
	
	void AddSequentialElimination(int32 Count);
	
	void UpdateHighestStreak(int32 StreakCount);
	
	void AddRevengeElimination();
	
	int32 GetRevengeEliminations() const { return RevengeEliminations; }
	
	void AddDethroneElimination();
	
	int32 GetDethroneEliminations() const { return DethroneEliminations; }
	
	void AddShowStopper();
	
	int32 GetShowStopper() const { return ShowStopperEliminations; }
	
	void FirstBlood();
	
	void SetWinner();
	
	bool IsOnEliminationStreak() const { return bOnEliminationStreak; }
	
	void SetOnEliminationStreak(bool bActive);
	
	void SetLastAttacker(APlayerState* Attacker);
	
	APlayerState* GetLastAttacker() const { return LastAttacker.IsValid() ? LastAttacker.Get() : nullptr; }
	
	UFUNCTION(Client, Reliable)
	void Client_LostTheLead();
	
	UFUNCTION(Client, Reliable)
	void Client_ScoredElimination(int32 EliminationCount);
	
	UFUNCTION(Client, Reliable)
	void Client_ScoredSpecialElimination(const ESpecialEliminationType& SpecialEliminationType, int32 SequentialEliminationCount, int32 StreakCount);
	
	UFUNCTION(BlueprintPure, Category="FPS|Eliminations")
	USpecialEliminationsData* GetSpecialEliminationsData() const { return SpecialEliminationsData; }
	
public:
	UPROPERTY(BlueprintAssignable)
	FScoreEliminationDelegate OnScoreEliminationChanged;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Eliminations")
	TObjectPtr<USpecialEliminationsData> SpecialEliminationsData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|UI")
	TSubclassOf<UFPSSpecialEliminationWidget> SpecialEliminationsWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|UI")
	float SpecialEliminationDisplayDelay;
	
private:
	static TArray<ESpecialEliminationType> DecodeSpecialEliminationBitMask(const ESpecialEliminationType BitMask);
	
	void ProcessSpecialElimination();
	
	void DisplaySpecialElimination(const FSpecialEliminationInfo& EliminationInfo) const;
	
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
	
	TWeakObjectPtr<APlayerState> LastAttacker;
	
	TQueue<FSpecialEliminationInfo> SpecialEliminationQueue;
	
	bool bIsProcessingEliminationQueue;
};
