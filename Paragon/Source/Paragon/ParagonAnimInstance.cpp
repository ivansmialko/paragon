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
	RootYawOffset(0.f),
	Pitch(0.f),
	bIsReloading(false),
	CurrentOffsetState(EOffsetState::EOS_Hip)
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

	if (bIsReloading)
	{
		CurrentOffsetState = EOffsetState::EOS_Reloading;
	}
	else if (bIsInAir)
	{
		CurrentOffsetState = EOffsetState::EOS_InAir;
	}
	else if (ParagonCharacter->GetIsAiming())
	{
		CurrentOffsetState = EOffsetState::EOS_Aiming;
	}
	else
	{
		CurrentOffsetState = EOffsetState::EOS_Hip;
	}

	TurnInPlace();
	Lean(DeltaTime);
}

void UParagonAnimInstance::NativeInitializeAnimation()
{
	ParagonCharacter = Cast<AParagonCharacter>(TryGetPawnOwner());

}

void UParagonAnimInstance::TurnInPlace()
{
	if (!ParagonCharacter)
		return;

	Pitch = ParagonCharacter->GetBaseAimRotation().Pitch;
	bIsReloading = ParagonCharacter->GetCombatState() == ECombatState::ECS_ReloadingState;

	if (Speed > 0 || bIsInAir)
	{
		RootYawOffset = 0.f;

		TIPCharacterYaw = ParagonCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;

		RotationCurveValue = 0.f;
		RotationCurveValueLastFrame = 0.f;
		return;
	}

	TIPCharacterYawLastFrame = TIPCharacterYaw;
	TIPCharacterYaw = ParagonCharacter->GetActorRotation().Yaw;

	const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

	//Root bone Yaw Offset, updated and clamped to [-180, 180]
	RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);
	
	//1.0 if turning animation is playing, 0.0 if not
	const float TurningCurveValue{ GetCurveValue(TEXT("Turning")) };
	if (TurningCurveValue <= 0)
		return;

	RotationCurveValueLastFrame = RotationCurveValue;
	RotationCurveValue = GetCurveValue(TEXT("Rotation"));

	const float RotationDelta{ RotationCurveValue - RotationCurveValueLastFrame };

	//RootYawOffset > 0, -> Turning left
	//RootYawOffset < 0, -> Turning right
	(RootYawOffset > 0 ? RootYawOffset -= RotationDelta : RootYawOffset += RotationDelta);

	const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
	if (ABSRootYawOffset > 90)
	{
		const float YawExcess{ ABSRootYawOffset - 90.f };
		(RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess);
	}
}

void UParagonAnimInstance::Lean(float DeltaTime)
{
	if (!ParagonCharacter)
		return;

	CharacterYawLastFrame = CharacterYaw;
	CharacterYaw = ParagonCharacter->GetActorRotation().Yaw;

	const float Target{ (CharacterYaw - CharacterYawLastFrame) / DeltaTime };
	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	if (!GEngine)
		return;

	GEngine->AddOnScreenDebugMessage(1, -1.f, FColor::Green, FString::Printf(TEXT("Current yaw delta: %f"), YawDelta));
}
