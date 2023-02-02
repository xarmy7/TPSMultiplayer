// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicCube.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APhysicCube::APhysicCube()
{
	bReplicates = true;
	SetReplicates(true);
	SetReplicateMovement(true);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APhysicCube::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
		SetActorEnableCollision(false);

	if (GetLocalRole() != ENetRole::ROLE_Authority)
		return;
}

// Called every frame
void APhysicCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() != ENetRole::ROLE_Authority)
		return;
}