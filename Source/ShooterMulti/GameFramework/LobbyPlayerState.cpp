// Fill out your copyright notice in the Description page of Project Settings.


#include "../GameFramework/LobbyPlayerState.h"
#include "../GameFramework/LobbyGameMode.h"
#include "Net/UnrealNetwork.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	bReplicates = true;
}

void ALobbyPlayerState::ReadyUpOnServer_Implementation(bool ready)
{
	bIsReady = ready;

	AGameModeBase* gm = GetWorld()->GetAuthGameMode();
	if (!gm)
		return;

	ALobbyGameMode* agm = Cast<ALobbyGameMode>(gm);
	if (!agm)
		return;

	agm->TryStartGame();
}

void ALobbyPlayerState::SetNicknameOnServer_Implementation(const FString& str)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("server : %s"), *str));
	SetNicknameMulticast(str);
}

void ALobbyPlayerState::SetNicknameMulticast_Implementation(const FString& str)
{
	nickname = str;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("multicast : %s"), *str));
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bIsReady);
	DOREPLIFETIME(ALobbyPlayerState, nickname);
	DOREPLIFETIME(ALobbyPlayerState, team);
}
