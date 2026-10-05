// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerState.h"

AFPSPlayerState::AFPSPlayerState()
{
	SetNetUpdateFrequency(100.0f);
	
	Eliminations = 0;
	HeadShotEliminations = 0;
	Defeats = 0;
	RoundsHit = 0;
	RoundsMissed = 0;
	HighestEliminationStreak = 0;
	RevengeEliminations = 0;
	DethroneEliminations = 0;
	ShowStopperEliminations = 0;
	
	bOnEliminationStreak = false;
	bFirstBlood = false;
	bWinner = false;
}

void AFPSPlayerState::AddElimination(bool bFromHeadShot)
{
	++Eliminations;
	
	if (bFromHeadShot)
	{
		++HeadShotEliminations;
	}
}

void AFPSPlayerState::AddDefeat()
{
	++Defeats;
}

void AFPSPlayerState::AddHit()
{
	++RoundsHit;
}

void AFPSPlayerState::AddMiss()
{
	++RoundsMissed;
}

void AFPSPlayerState::AddSequentialElimination(int32 Count)
{
	if (SequentialEliminations.Contains(Count))
	{
		++SequentialEliminations[Count];
	}
	else
	{
		SequentialEliminations.Add(Count, 1);
	}
	
	// Reduce all previous eliminations to prevent awarding a double kill as part of a triple kill
	for (auto& Entry : SequentialEliminations)
	{
		if (Entry.Key < Count && Entry.Value > 0)
		{
			Entry.Value--;
		}
	}
}

void AFPSPlayerState::UpdateHighestStreak(const int32 StreakCount)
{
	if (HighestEliminationStreak < StreakCount)
	{
		HighestEliminationStreak = StreakCount;
	}
}

void AFPSPlayerState::AddRevengeElimination()
{
	++RevengeEliminations;
}

void AFPSPlayerState::AddDethroneElimination()
{
	++DethroneEliminations;
}

void AFPSPlayerState::AddShowStopper()
{
	++ShowStopperEliminations;
}

void AFPSPlayerState::FirstBlood()
{
	bFirstBlood = true;
}

void AFPSPlayerState::SetWinner()
{
	bWinner = true;
}
