// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup.h"
#include "FastAndLow.generated.h"

UCLASS()
class SHOOTERMULTI_API AFastAndLow : public APickup
{
	GENERATED_BODY()

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	FTimerHandle TimerHandle;

public:
	UFUNCTION(Server, Reliable)
	void FastAndLowOnServer(AShooterCharacter* shooterCharacter);
};
