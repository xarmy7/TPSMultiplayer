// Fill out your copyright notice in the Description page of Project Settings.


#include "../AI/UndeadCheckTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Characters/ShooterCharacter.h"

bool UUndeadCheckTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* bb = OwnerComp.GetBlackboardComponent();
	if (!bb)
		return false;

	UObject* target = bb->GetValueAsObject("Target");
	if (!target)
		return false;

	AShooterCharacter* chara = Cast<AShooterCharacter>(target);
	if (!chara)
		return false;

	return !chara->IsDead();
}
