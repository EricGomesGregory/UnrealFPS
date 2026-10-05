// Fill out your copyright notice in the Description page of Project Settings.


#include "EliminationComponent.h"


UEliminationComponent::UEliminationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
}

void UEliminationComponent::OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal)
{
	if (bHit)
	{
		if (bLethal)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s eliminated %s with headshot? %d"), *Attacker->GetName(), *Victim->GetName(), bHeadShot);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s hit %s with headshot? %d"), *Attacker->GetName(), *Victim->GetName(), bHeadShot);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s missed shot!"), *Attacker->GetName());
	}
	
	//@Eric TODO: Implement this!
}


