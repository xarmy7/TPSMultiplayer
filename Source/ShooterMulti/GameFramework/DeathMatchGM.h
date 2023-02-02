#pragma once

#include "GameFramework/GameModeBase.h"
#include "DeathMatchGM.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None = 0,
	Blue,
	Red,
	AI UMETA(Hidden),
};

UCLASS()
class SHOOTERMULTI_API ADeathMatchGM : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float InvincibilityTime = 3.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GameTime = 300;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxKill = 20;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxAIPerPlayer = 10;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void RespawnOnServer(APlayerController* PlayerController);

	UFUNCTION(NetMulticast, Reliable)
	void RespawnMulticast(APlayerController* PlayerController);

	void Quit();
};
