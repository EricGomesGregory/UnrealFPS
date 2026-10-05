// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EliminationComponent.generated.h"

class AFPSGameState;
enum class ESpecialEliminationType : uint16;
class AFPSPlayerState;


UCLASS(ClassGroup=(FPS), meta=(BlueprintSpawnableComponent))
class FPS_API UEliminationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEliminationComponent();
	
	UFUNCTION()
	void OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal);
	
protected:
	AFPSPlayerState* GetPlayerStateFromActor(AActor* Actor) const;
	
	void ProcessHitOrMiss(AFPSPlayerState* AttackerPS, bool bHit, bool bHeadShot);
	
	void ProcessElimination(AFPSPlayerState* AttackerPS, AFPSPlayerState* VictimPS, bool bHeadShot);
	
	void ProcessHeadShot(bool bHeadShot, ESpecialEliminationType& OutEliminationType);
	
	void ProcessSequentialElimination(AFPSPlayerState* AttackerPS, ESpecialEliminationType& OutEliminationType);
	
	void ProcessStreakRevengeShowStopper(AFPSPlayerState* AttackerPS, AFPSPlayerState* VictimPS, ESpecialEliminationType& OutEliminationType);
	
	void HandleFirstBlood(AFPSGameState* GameState, AFPSPlayerState* AttackerPS, ESpecialEliminationType& OutEliminationType);
	
	void UpdateLeaderStatus(AFPSGameState* GameState, AFPSPlayerState* AttackerPS, AFPSPlayerState* VictimPS, ESpecialEliminationType& OutEliminationType);
	
	bool HasSpecialEliminationTypes(const ESpecialEliminationType& SpecialEliminationType) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="FPS|Elimination")
	float SequentialEliminationInterval;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Elimination")
	int32 StreakEliminationCount;
	
private:
	float LastEliminationTime;
	
	int32 SequentialEliminationCount;
	
	int32 StreakCount;
};
