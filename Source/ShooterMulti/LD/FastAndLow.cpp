// Fill out your copyright notice in the Description page of Project Settings.


#include "FastAndLow.h"
#include "../GameFramework/DeathMatchGS.h"
#include "../Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void AFastAndLow::NotifyActorBeginOverlap(AActor* OtherActor)
{
	AShooterCharacter* Player = Cast<AShooterCharacter>(OtherActor);

	FastAndLowOnServer(Player);

	// After because of Destroy
	Super::NotifyActorBeginOverlap(OtherActor);
}

void AFastAndLow::FastAndLowOnServer_Implementation(AShooterCharacter* shooterCharacter)
{
	AGameStateBase* gameState = GetWorld()->GetAuthGameMode()->GetGameState<ADeathMatchGS>();

	shooterCharacter->movementMultiplyer = 2.f;
	GetWorldTimerManager().SetTimer(TimerHandle, shooterCharacter, &AShooterCharacter::ResetEffectPickupOnServer, 15, false);

	//for (auto player : gameState->PlayerArray)
	//{
	//	if (!player)
	//		return;

	//	AShooterCharacter* myPlayer;
	//	myPlayer = Cast<AShooterCharacter>(player->GetPawn());


	//	if (!myPlayer)
	//		return;

	//	GetWorldTimerManager().SetTimer(TimerHandle, myPlayer, &AShooterCharacter::ResetEffectPickupOnServer, 15, false);

	//	if (myPlayer != shooterCharacter)
	//		myPlayer->movementMultiplyer = 0.5f;
	//}
}