// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"


void AFPSGameMode::RequestRespawn(ACharacter* Character, AController* Controller)
{
	check(Character);
	check(Controller);
	
	Character->Reset();
	Character->Destroy();
	
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), SpawnPoints);
	ensure(SpawnPoints.Num() > 0);
	
	int32 Selection = FMath::RandRange(0, SpawnPoints.Num() - 1);
	RestartPlayerAtPlayerStart(Controller, SpawnPoints[Selection]);
}
