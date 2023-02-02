#include "PhysicCubeDirector.h"
#include "PhysicCube.h"
#include "Engine/World.h"
#include "../GameFramework/DeathMatchGS.h"

APhysicCubeDirector::APhysicCubeDirector()
{
	//bReplicates = true;
}

void APhysicCubeDirector::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() != ENetRole::ROLE_Authority)
		return;

	IsSpawnFullArray.SetNum(SpawnPoints.Num());

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APhysicCubeDirector::SpawnTick, SecondPerSpawn, true);
}

void APhysicCubeDirector::SpawnTick()
{
	if (bIsFull)
		return;

	int MaxPoints = SpawnPoints.Num();
	int RandomPoint = FMath::RandRange(0, MaxPoints - 1);
	int PrevPoint = RandomPoint;

	while (IsSpawnFullArray[RandomPoint])
	{
		RandomPoint = (RandomPoint + 1) % MaxPoints;
		if (RandomPoint == PrevPoint)
		{
			bIsFull = true;
			return;
		}
	}

	IsSpawnFullArray[RandomPoint] = true;
	SpawnPhysicCube(0, RandomPoint);
	CurrentCubeIndex = (CurrentCubeIndex + 1) % PhysicCubeBPs.Num();
}

void APhysicCubeDirector::SpawnPhysicCube(int cubeIndex, int spawnPointIndex)
{
	if (GetLocalRole() != ENetRole::ROLE_Authority)
		return;

	auto cubeBP = PhysicCubeBPs[cubeIndex];
	FVector cubeLocation = SpawnPoints[spawnPointIndex]->GetActorLocation();
	FRotator cubeRotation = SpawnPoints[spawnPointIndex]->GetActorRotation();

	APhysicCube* Cube = GetWorld()->SpawnActor<APhysicCube>(cubeBP, cubeLocation, cubeRotation);
}

void APhysicCubeDirector::SetFull(bool isFull)
{
	bIsFull = isFull;
}

void APhysicCubeDirector::Reset()
{
	bIsFull = false;

	for (int i = 0; i < IsSpawnFullArray.Num(); i++)
		IsSpawnFullArray[i] = false;
}
