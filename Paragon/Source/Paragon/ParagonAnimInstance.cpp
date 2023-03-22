// Fill out your copyright notice in the Description page of Project Settings.


#include "ParagonAnimInstance.h"
#include "ParagonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UParagonAnimInstance::UParagonAnimInstance() :
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffset(0.f),
	LastMovementOffset(0.f),
	bIsAiming(false),
	CharacterYaw(0.f),
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f)
{

}

void UParagonAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ParagonCharacter == nullptr)
	{
		ParagonCharacter = Cast<AParagonCharacter>(TryGetPawnOwner());
	}

	if (ParagonCharacter == nullptr)
		return;

	//Get the lateral speed of the character from velocity
	FVector Velocity{ ParagonCharacter->GetVelocity() };
	Velocity.Z = 0;
	Speed = Velocity.Size();

	//Is the character in the air?
	bIsInAir = ParagonCharacter->GetCharacterMovement()->IsFalling();

	//Is the character is accelerating?
	bIsAccelerating = (ParagonCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0);

	FRotator AimRotation = ParagonCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ParagonCharacter->GetVelocity());

	MovementOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	if (ParagonCharacter->GetVelocity().Size() > 0.f)
	{
		LastMovementOffset = MovementOffset;
	}

	bIsAiming = ParagonCharacter->GetIsAiming();

	TurnInPlace();
}

void UParagonAnimInstance::NativeInitializeAnimation()
{
	ParagonCharacter = Cast<AParagonCharacter>(TryGetPawnOwner());

}

void UParagonAnimInstance::TurnInPlace()
{
	if (!ParagonCharacter)
		return;

	if (Speed > 0)
		return;

	CharacterYawLastFrame = CharacterYaw;
	CharacterYaw = ParagonCharacter->GetActorRotation().Yaw;

	const float YawDelta{ CharacterYaw - CharacterYawLastFrame };

	RootYawOffset -= YawDelta;

	if (!GEngine)
		return;

	GEngine->AddOnScreenDebugMessage(1, -1, FColor::Blue, FString::Printf(TEXT("CharacterYaw: %f"), CharacterYaw));
	GEngine->AddOnScreenDebugMessage(2, -1, FColor::Red, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));
}
