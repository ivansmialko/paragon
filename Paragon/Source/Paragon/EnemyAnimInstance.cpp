// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!Enemy)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}

	if (!Enemy)
		return;

	FVector Velocity{ Enemy->GetVelocity() };
	Velocity.Z = 0.f;

	CurrentSpeed = Velocity.Size();
}
