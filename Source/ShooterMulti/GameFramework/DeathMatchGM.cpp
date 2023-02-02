#include "DeathMatchGM.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

void ADeathMatchGM::RespawnOnServer_Implementation(APlayerController* PlayerController)
{
	if (GetLocalRole() != ENetRole::ROLE_Authority)
		return;

	RespawnMulticast(PlayerController);
}

void ADeathMatchGM::RespawnMulticast_Implementation(APlayerController* PlayerController)
{
	RestartPlayerAtPlayerStart(PlayerController, ChoosePlayerStart(PlayerController));
}

void ADeathMatchGM::Quit()
{
	FGenericPlatformMisc::RequestExit(false);
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}