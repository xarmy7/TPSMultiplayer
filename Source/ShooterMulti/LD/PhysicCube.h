// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicCube.generated.h"


UCLASS()
class SHOOTERMULTI_API APhysicCube : public AActor
{
	GENERATED_BODY()

private:

	friend class APhysicCubeDirector;
public:	
	// Sets default values for this actor's properties
	APhysicCube();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void Tick(float DeltaTime) override;
};
