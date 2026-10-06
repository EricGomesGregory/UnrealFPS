// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerScoreWidget.h"

#include "Components/TextBlock.h"
#include "FPS/Player/FPSPlayerController.h"
#include "FPS/Player/FPSPlayerState.h"


void UFPSPlayerScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();


	if (auto* ShooterPS = GetPlayerState())
	{
		ShooterPS->OnScoreEliminationChanged.AddUniqueDynamic(this, &ThisClass::OnScoreEliminationChanged);
	}
	else
	{
		const auto* ShooterPC = CastChecked<AFPSPlayerController>(GetOwningPlayer());
		ShooterPC->OnPlayerStateReplicated.AddUniqueDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}

void UFPSPlayerScoreWidget::OnPlayerStateReplicated()
{
	auto* ShooterPS = GetPlayerState();
	check(ShooterPS);
	
	ShooterPS->OnScoreEliminationChanged.AddUniqueDynamic(this, &ThisClass::OnScoreEliminationChanged);
	
	const auto* ShooterPC = CastChecked<AFPSPlayerController>(GetOwningPlayer());
	ShooterPC->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
}

void UFPSPlayerScoreWidget::OnScoreEliminationChanged(int32 NewEliminationCount)
{
	check(Text_Score);
	
	Text_Score->SetText(FText::AsNumber(NewEliminationCount));
}

AFPSPlayerState* UFPSPlayerScoreWidget::GetPlayerState() const
{
	const auto* PC = GetOwningPlayer();
	check(PC);
	
	return PC->GetPlayerState<AFPSPlayerState>();
}
