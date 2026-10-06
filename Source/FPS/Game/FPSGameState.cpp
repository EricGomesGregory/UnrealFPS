// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameState.h"
#include "FPS/Player/FPSPlayerState.h"

AFPSGameState::AFPSGameState()
{
	bHasFirstBloodTriggered = false;
}

void AFPSGameState::UpdateLeader()
{
	TArray<APlayerState*> SortedPlayerStates = PlayerArray;
	SortedPlayerStates.Sort([](const APlayerState& PlayerA, const APlayerState& PlayerB)
	{
		const auto* FPSPlayerA = CastChecked<AFPSPlayerState>(&PlayerA);
		const auto* FPSPlayerB = CastChecked<AFPSPlayerState>(&PlayerB);
		
		return FPSPlayerA->GetEliminations() >= FPSPlayerB->GetEliminations();
	});
	
	Leaders.Empty();
	if (SortedPlayerStates.Num() > 0)
	{
		int32 HighestScore = 0;
		for (auto* PlayerState : SortedPlayerStates)
		{
			AFPSPlayerState* FPSPlayerState = CastChecked<AFPSPlayerState>(PlayerState);
			
			if (Leaders.Num() == 0)
			{
				HighestScore = FPSPlayerState->GetEliminations();
				Leaders.Add(FPSPlayerState);
			}
			else if (FPSPlayerState->GetEliminations() == HighestScore)
			{
				HighestScore = FPSPlayerState->GetEliminations();
				Leaders.Add(FPSPlayerState);
			}
			else
			{
				break;
			}
		}
	}
	
	if (bHasFirstBloodTriggered == false)
	{
		bHasFirstBloodTriggered = true;
	}
}

AFPSPlayerState* AFPSGameState::GetSoleLeader() const
{
	if (Leaders.Num() == 1)
	{
		return Leaders[0];
	}
	return nullptr;
}

bool AFPSGameState::IsTiedForTheLead(AFPSPlayerState* PlayerState) const
{
	return Leaders.Contains(PlayerState) && Leaders.Num() > 1;
}
