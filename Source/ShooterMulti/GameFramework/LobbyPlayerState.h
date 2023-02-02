// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "../GameFramework/DeathMatchGM.h"
#include "LobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

private:

public:
	UPROPERTY(ReplicatedUsing = Readied, BlueprintReadOnly)
	bool bIsReady = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	FString nickname;
	UPROPERTY(Replicated, BlueprintReadWrite)
	ETeam team;

	ALobbyPlayerState();

	UFUNCTION(BlueprintNativeEvent)
	void Readied();
	void Readied_Implementation() {}

	// set the variable to tell if the player is ready
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ReadyUpOnServer(bool ready);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetNicknameOnServer(const FString& str);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void SetNicknameMulticast(const FString& str);

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};
