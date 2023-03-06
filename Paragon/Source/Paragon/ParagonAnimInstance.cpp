// Fill out your copyright notice in the Description page of Project Settings.


#include "ParagonAnimInstance.h"
#include "ParagonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
}

void UParagonAnimInstance::NativeInitializeAnimation()
{
	ParagonCharacter = Cast<AParagonCharacter>(TryGetPawnOwner());

}
