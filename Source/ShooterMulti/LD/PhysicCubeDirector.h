// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicCube.h"
#include "PhysicCubeDirector.generated.h"

UCLASS()
class SHOOTERMULTI_API APhysicCubeDirector : public AActor
{
	GENERATED_BODY()
	
private:	

	TArray<bool> IsSpawnFullArray;
	int CurrentCubeIndex = 0;

	FTimerHandle TimerHandle;

	bool bIsFull = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	APhysicCubeDirector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Director, meta = (ClampMin = 0.1f))
	float SecondPerSpawn = 15.0f;

	UPROPERTY(EditInstanceOnly, BlueprintInternalUseOnly, Category = Director)
	TArray<AActor*> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintInternalUseOnly, Category = Director)
	TArray<TSubclassOf<APhysicCube>> PhysicCubeBPs;

	void SpawnTick();

	void SpawnPhysicCube(int cubeIndex, int spawnPointIndex);

	void SetFull(bool isFull);

	//void UpdateFrequencies(class ADeathMatchGS* GameState);

	virtual void Reset() override;

};
