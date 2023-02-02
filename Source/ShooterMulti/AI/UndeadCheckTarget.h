// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UndeadCheckTarget.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API UUndeadCheckTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
