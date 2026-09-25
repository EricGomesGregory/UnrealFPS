// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFPSHealthChanged, UHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

/**
 * EDeathState
 * 
 */
UENUM(BlueprintType)
enum class EDeathState : uint8
{
	NotDead,
	DeathStarted,
	DeathFinished,
};


/**
 * UHealthComponent
 * 
 */
UCLASS(ClassGroup=(FPS), meta=(BlueprintSpawnableComponent))
class FPS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UFUNCTION(BlueprintPure, Category = "FPS|Health")
	static UHealthComponent* FindHealthComponent(AActor* Actor) { return IsValid(Actor) ? Actor->FindComponentByClass<UHealthComponent>() : nullptr; }
	
	UFUNCTION(BlueprintCallable, Category="FPS|Health")
	float GetHealthNormalized() const;
	
	bool ChangeHealthByAmount(float Amount, AActor* Instigator);
	void ChangeMaxHealthByAmount(float Amount, AActor* Instigator);
	
public:
	UPROPERTY(BlueprintAssignable)
	FFPSHealthChanged OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FFPSHealthChanged OnMaxHealthChanged;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_DeathState(EDeathState OldDeathState);

	UFUNCTION()
	void OnRep_Health(float OldHealth);
	
	UFUNCTION()
	void OnRep_MaxHealth(float OldMaxHealth);

public:
	UPROPERTY(ReplicatedUsing=OnRep_DeathState)
	EDeathState DeathState;
	
	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Health;
	
	UPROPERTY(ReplicatedUsing=OnRep_MaxHealth)
	float MaxHealth;
	
};
