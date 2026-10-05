// Fill out your copyright notice in the Description page of Project Settings.


#include "EliminationComponent.h"

#include "FPS/FPSTypes.h"
#include "FPS/Game/FPSGameState.h"
#include "FPS/Player/FPSPlayerState.h"
#include "Kismet/GameplayStatics.h"


UEliminationComponent::UEliminationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SequentialEliminationInterval = 2.0f;
	SequentialEliminationCount = 0;
	LastEliminationTime = 0.0f;
	
	StreakEliminationCount = 5;
	StreakCount = 0;
}

void UEliminationComponent::OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal)
{
	auto* AttackerPS = GetPlayerStateFromActor(Attacker);
	check(AttackerPS);
	
	if (!bHit) return;
	ProcessHitOrMiss(AttackerPS, bHit, bHeadShot);
	
	auto* VictimPS = GetPlayerStateFromActor(Victim);
	check(VictimPS);
	
	if (bLethal)
	{
		ProcessElimination(AttackerPS, VictimPS, bHeadShot);
	}
}

AFPSPlayerState* UEliminationComponent::GetPlayerStateFromActor(AActor* Actor) const
{
	const APawn* Pawn = CastChecked<APawn>(Actor);
	return Pawn->GetPlayerState<AFPSPlayerState>();
}

void UEliminationComponent::ProcessHitOrMiss(AFPSPlayerState* AttackerPS, bool bHit, bool bHeadShot)
{
	if (bHit)
	{
		AttackerPS->AddHit();
	}
	else
	{
		AttackerPS->AddMiss();
	}
}

void UEliminationComponent::ProcessElimination(AFPSPlayerState* AttackerPS, AFPSPlayerState* VictimPS, bool bHeadShot)
{
	AttackerPS->AddElimination(bHeadShot);
	VictimPS->AddDefeat();
	
	ESpecialEliminationType EliminationType {};
	ProcessHeadShot(bHeadShot, EliminationType);
	ProcessSequentialElimination(AttackerPS, EliminationType);
	ProcessStreakRevengeShowStopper(AttackerPS, VictimPS, EliminationType);
	
	auto* FPSGameState = CastChecked<AFPSGameState>(UGameplayStatics::GetGameState(GetWorld()));
	HandleFirstBlood(FPSGameState, AttackerPS, EliminationType);
	UpdateLeaderStatus(FPSGameState, AttackerPS, VictimPS, EliminationType);
	
	if (HasSpecialEliminationTypes(EliminationType))
	{
		AttackerPS->Client_ScoredSpecialElimination(EliminationType, SequentialEliminationCount, StreakCount, AttackerPS->GetEliminations());
	}
	else
	{
		AttackerPS->Client_ScoredElimination(AttackerPS->GetEliminations());
	}
}

void UEliminationComponent::ProcessHeadShot(bool bHeadShot, ESpecialEliminationType& OutEliminationType)
{
	if (bHeadShot)
	{
		OutEliminationType |= ESpecialEliminationType::HeadShot;
	}
}

void UEliminationComponent::ProcessSequentialElimination(AFPSPlayerState* AttackerPS, ESpecialEliminationType& OutEliminationType)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	
	if ((CurrentTime - LastEliminationTime) <= SequentialEliminationInterval)
	{
		++SequentialEliminationCount;
	}
	else
	{
		SequentialEliminationCount = 1;
	}
	
	LastEliminationTime = CurrentTime;
	
	if (SequentialEliminationCount > 1)
	{
		OutEliminationType |= ESpecialEliminationType::Sequential;
		AttackerPS->AddSequentialElimination(SequentialEliminationCount);
	}
}

void UEliminationComponent::ProcessStreakRevengeShowStopper(AFPSPlayerState* AttackerPS, AFPSPlayerState* VictimPS, ESpecialEliminationType& OutEliminationType)
{
	++StreakCount;
	if (StreakCount >= StreakEliminationCount)
	{
		OutEliminationType |= ESpecialEliminationType::Streak;
		AttackerPS->SetOnEliminationStreak(true);
		AttackerPS->UpdateHighestStreak(StreakCount);
	}
	
	if (VictimPS->IsOnEliminationStreak())
	{
		OutEliminationType |= ESpecialEliminationType::ShowStopper;
		AttackerPS->AddShowStopper();
	}
	
	VictimPS->SetOnEliminationStreak(false);
	
	if (AttackerPS->GetLastAttacker() == VictimPS)
	{
		OutEliminationType |= ESpecialEliminationType::Revenge;
		AttackerPS->AddRevengeElimination();
		AttackerPS->SetLastAttacker(nullptr);
	}
	
	VictimPS->SetLastAttacker(AttackerPS);
}

void UEliminationComponent::HandleFirstBlood(AFPSGameState* GameState, AFPSPlayerState* AttackerPS, ESpecialEliminationType& OutEliminationType)
{
	if (!GameState->HasFirstBloodTriggered())
	{
		OutEliminationType |= ESpecialEliminationType::FirstBlood;
		AttackerPS->FirstBlood();
		GameState->UpdateLeader();
	}
}

void UEliminationComponent::UpdateLeaderStatus(AFPSGameState* GameState, AFPSPlayerState* AttackerPS, AFPSPlayerState* VictimPS, ESpecialEliminationType& OutEliminationType)
{
	auto* LastLeader = GameState->GetSoleLeader();
	const bool bAttackerWasTiedForTheLead = GameState->IsTiedForTheLead(AttackerPS);
	
	GameState->UpdateLeader();
	if (!bAttackerWasTiedForTheLead && GameState->IsTiedForTheLead(AttackerPS))
	{
		OutEliminationType |= ESpecialEliminationType::TiedTheLeader;
	}
	
	if (IsValid(LastLeader) && LastLeader != GameState->GetSoleLeader())
	{
		LastLeader->Client_LostTheLead();
		
		if (VictimPS == LastLeader)
		{
			OutEliminationType |= ESpecialEliminationType::Dethrone;
			AttackerPS->AddDethroneElimination();
		}
	}
	
	if (AttackerPS != LastLeader && AttackerPS ==GameState->GetSoleLeader())
	{
		OutEliminationType |= ESpecialEliminationType::GainedTheLead;
	}
}

bool UEliminationComponent::HasSpecialEliminationTypes(const ESpecialEliminationType& SpecialEliminationType) const
{
	return static_cast<uint16>(SpecialEliminationType) != 0;
}


