#include "DeathMatchGS.h"
#include "ShooterPS.h"
#include "TimerManager.h"
#include "DeathMatchGM.h"
#include "../Characters/ShooterCharacter.h"
#include "../LD/Pickup.h"
#include "../Controllers/ShooterController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

void ADeathMatchGS::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() != ROLE_Authority)
		return;

	OnTeamWin.AddLambda([this](ETeam Team) { ShowTeamWinHUDMulticast(Team); });

	OnGameRestart.AddLambda([this]() { Reset(); });

	OnResetAfterDelay.AddLambda([this]() { EndResetMulticast(); });

	GameMode = Cast<ADeathMatchGM>(AuthorityGameMode);

	check(GameMode && "GameMode nullptr: Cast as ADeathMatchGM failed.");

	CurrentTime = GameMode->GameTime;
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ADeathMatchGS::AdvanceTimer, 1.0f, true);
}

void ADeathMatchGS::AdvanceTimer()
{
	--CurrentTime;
	
	if (CurrentTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		if (RedTeamScore < BlueTeamScore)
			EndGame(ETeam::Blue);
		else if (RedTeamScore > BlueTeamScore)
			EndGame(ETeam::Red);
		else
			EndGame(ETeam::None);
	}
}

void ADeathMatchGS::AddScoreOnServer_Implementation(ETeam Team)
{
	if (Team == ETeam::Red && ++RedTeamScore == GameMode->MaxKill)
		EndGame(ETeam::Red);
	else if (Team == ETeam::Blue && ++BlueTeamScore == GameMode->MaxKill)
		EndGame(ETeam::Blue);
}

void ADeathMatchGS::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnPlayerNum.Broadcast(this);
}

void ADeathMatchGS::RemovePlayerState(APlayerState* PlayerState)
{
	OnPlayerNum.Broadcast(this);

	Super::RemovePlayerState(PlayerState);
}

bool ADeathMatchGS::CanAddAI()
{
	return Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->MaxAIPerPlayer* PlayerArray.Num() > CurrentAICount;

	return false;
}

void ADeathMatchGS::AddAI()
{
	CurrentAICount++;
}

void ADeathMatchGS::RemoveAI()
{
	CurrentAICount--;
}

int ADeathMatchGS::GetNbplayer()
{
	return PlayerArray.Num();
}

void ADeathMatchGS::ShowTeamWinHUDMulticast_Implementation(ETeam Team)
{
	ShowTeamWinHUD(Team);
}

void ADeathMatchGS::EndGame(ETeam Team)
{
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	// display end game's HUD for the clients
	OnTeamWin.Broadcast(Team);

	// reset the game
	OnGameRestart.Broadcast();

	// reset after a delay
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &ADeathMatchGS::ResetAfterDelay, 4.5f, false);
}

void ADeathMatchGS::Reset()
{
	TArray<AActor*> Resetables;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UResetable::StaticClass(), Resetables);

	for (auto& res : Resetables)
		Cast<IResetable>(res)->Reset();
}

void ADeathMatchGS::ResetAfterDelay()
{
	CurrentTime = GameMode->GameTime;
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ADeathMatchGS::AdvanceTimer, 1.0f, true);

	RedTeamScore = 0;
	BlueTeamScore = 0;
	CurrentAICount = 0;

	OnResetAfterDelay.Broadcast();
}

void ADeathMatchGS::EndResetMulticast_Implementation()
{
	EndReset();
}

int ADeathMatchGS::NewFrequency(int Sec)
{
	return 0;
}

void ADeathMatchGS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeathMatchGS, CurrentTime);
	DOREPLIFETIME(ADeathMatchGS, CurrentAICount);
	DOREPLIFETIME(ADeathMatchGS, RedTeamScore);
	DOREPLIFETIME(ADeathMatchGS, BlueTeamScore);
}