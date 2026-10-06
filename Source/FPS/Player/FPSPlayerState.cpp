// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerState.h"

#include "FPS/Elimination/SpecialEliminationsData.h"
#include "FPS/UI/FPSSpecialEliminationWidget.h"

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
	
	bIsProcessingEliminationQueue = false;
	SpecialEliminationDisplayDelay = 0.5f;
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

void AFPSPlayerState::SetOnEliminationStreak(bool bActive)
{
	bOnEliminationStreak = bActive;
}

void AFPSPlayerState::SetLastAttacker(APlayerState* Attacker)
{
	LastAttacker = Attacker;
}

void AFPSPlayerState::Client_ScoredSpecialElimination_Implementation(const ESpecialEliminationType& SpecialEliminationType, int32 SequentialEliminationCount, int32 StreakCount)
{
	ensure(SpecialEliminationsData);
	
	auto EliminationTypes = DecodeSpecialEliminationBitMask(SpecialEliminationType);
	for (const ESpecialEliminationType EliminationType : EliminationTypes)
	{
		UE_LOG(LogTemp, Warning, TEXT("EliminationType=%d"), static_cast<uint16>(EliminationType));
		auto& EliminationInfo = SpecialEliminationsData->SpecialEliminations.FindChecked(EliminationType);
		EliminationInfo.EliminationType = EliminationType;
		if (EliminationType == ESpecialEliminationType::Sequential)
		{
			EliminationInfo.SequentialEliminationCount = SequentialEliminationCount;
		}
		else if (EliminationType == ESpecialEliminationType::Streak)
		{
			EliminationInfo.StreakCount = StreakCount;
		}
		
		SpecialEliminationQueue.Enqueue(EliminationInfo);
	}
	
	if (!bIsProcessingEliminationQueue)
	{
		ProcessSpecialElimination();
	}
}

void AFPSPlayerState::Client_ScoredElimination_Implementation(int32 EliminationCount)
{
	ensure(SpecialEliminationsData);
	
	//@Eric TODO: Implement delegate dispatching 
}

void AFPSPlayerState::Client_LostTheLead_Implementation()
{
	ensure(SpecialEliminationsData);
	
	auto& EliminationInfo = SpecialEliminationsData->SpecialEliminations.FindChecked(ESpecialEliminationType::LostTheLead);
	if (IsValid(SpecialEliminationsWidgetClass))
	{
		auto* EliminationWidget = CreateWidget<UFPSSpecialEliminationWidget>(GetPlayerController(), SpecialEliminationsWidgetClass);
		check(EliminationWidget);
		
		EliminationWidget->InitializeWidget(EliminationInfo.Message, EliminationInfo.IconTexture);
		EliminationWidget->AddToViewport();
	}
}


TArray<ESpecialEliminationType> AFPSPlayerState::DecodeSpecialEliminationBitMask(const ESpecialEliminationType BitMask)
{
	TArray<ESpecialEliminationType> Result;
	const auto BitMaskValue = static_cast<uint16>(BitMask);
	for (uint16 i = 0; i < 16; i++)
	{
		if (BitMaskValue & (1 << i))
		{
			ESpecialEliminationType EnumValue = static_cast<ESpecialEliminationType>((1 << i));
			Result.Add(EnumValue);
		}
	}
	
	return Result;
}

void AFPSPlayerState::ProcessSpecialElimination()
{
	FSpecialEliminationInfo EliminationInfo;
	if (SpecialEliminationQueue.Dequeue(EliminationInfo))
	{
		bIsProcessingEliminationQueue = true;
		DisplaySpecialElimination(EliminationInfo);
		
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			FTimerHandle TimerHandle;
			const float Delay = SpecialEliminationDisplayDelay;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::ProcessSpecialElimination, Delay);
		});
	}
	else
	{
		bIsProcessingEliminationQueue = false;
	}
}

void AFPSPlayerState::DisplaySpecialElimination(const FSpecialEliminationInfo& EliminationInfo) const
{
	if (IsValid(SpecialEliminationsWidgetClass))
	{
		auto* EliminationWidget = CreateWidget<UFPSSpecialEliminationWidget>(GetPlayerController(), SpecialEliminationsWidgetClass);
		check(EliminationWidget);
	
		FString Message = EliminationInfo.Message;
		if (EliminationInfo.EliminationType == ESpecialEliminationType::Sequential)
		{
			
			if (EliminationInfo.SequentialEliminationCount > 4)
			{
				Message = FString("Rampage");
			}
			else if (EliminationInfo.SequentialEliminationCount == 4)
			{
				Message = FString("Quadra Kill");
			}
			else if (EliminationInfo.SequentialEliminationCount == 3)
			{
				Message = FString("Triple Kill");
			}
			else if (EliminationInfo.SequentialEliminationCount == 2)
			{
				Message = FString("Double Kill");
			}
		}
		else if (EliminationInfo.EliminationType == ESpecialEliminationType::Streak)
		{
			Message = FString::Printf(TEXT("%d Streak!"), EliminationInfo.StreakCount);
		}
		
		EliminationWidget->InitializeWidget(Message, EliminationInfo.IconTexture);
		EliminationWidget->AddToViewport();
	}
}
