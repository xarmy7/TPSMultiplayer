// Fill out your copyright notice in the Description page of Project Settings.


#include "../GameFramework/LobbyGameMode.h"
#include "../GameFramework/LobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"

bool ALobbyGameMode::IsEveryPlayerReady() const
{
	if (GetLocalRole() != ROLE_Authority)
		return false;

	AGameStateBase* gs = GetGameState<AGameStateBase>();
	if (!gs)
		return false;

	for (auto ps : gs->PlayerArray)
	{
		ALobbyPlayerState* lps = Cast<ALobbyPlayerState>(ps);
		if (!lps || !lps->bIsReady)
			return false;
	}

	return true;
}

ALobbyGameMode::ALobbyGameMode()
{
	bReplicates = true;
}

void ALobbyGameMode::TryStartGame()
{
	if (IsEveryPlayerReady())
	{
		bUseSeamlessTravel = true;
		GetWorld()->ServerTravel("Highrise");
	}
}
