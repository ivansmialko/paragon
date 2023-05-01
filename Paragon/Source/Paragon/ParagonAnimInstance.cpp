// Fill out your copyright notice in the Description page of Project Settings.


#include "ParagonAnimInstance.h"
#include "ParagonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon.h"
#include "WeaponType.h"
#include "Kismet/KismetMathLibrary.h"

UParagonAnimInstance::UParagonAnimInstance() :
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffset(0.f),
	LastMovementOffset(0.f),
	bIsAiming(false),
	TIPCharacterYaw(0.f),
	TIPCharacterYawLastFrame(0.f),
	YawDelta(0.f),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	RootYawOffset(0.f),
	Pitch(0.f),
	bIsReloading(false),
	CurrentOffsetState(EOffsetState::EOS_Hip),
	WeaponRecoilWeight(1.f),
	bIsTurningInPlace(false)
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

	bIsCrouching = ParagonCharacter->GetIsCrouching();

	TurnInPlace();
	Lean(DeltaTime);

	if (!ParagonCharacter->GetEquippedWeapon())
		return;

	EquippedWeaponType = ParagonCharacter->GetEquippedWeapon()->GetWeaponType();
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
	bIsEquipping = ParagonCharacter->GetCombatState() == ECombatState::ECS_Equipping;

	if (Speed > 0 || bIsInAir)
	{
		RootYawOffset = 0.f;

		TIPCharacterYaw = ParagonCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;

		RotationCurveValue = 0.f;
		RotationCurveValueLastFrame = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ParagonCharacter->GetActorRotation().Yaw;

		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

		//Root bone Yaw Offset, updated and clamped to [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		//1.0 if turning animation is playing, 0.0 if not
		const float TurningCurveValue{ GetCurveValue(TEXT("Turning")) };

		bIsTurningInPlace = (TurningCurveValue > 0);
		if (bIsTurningInPlace)
		{
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
	}

	if (bIsTurningInPlace)
	{
		WeaponRecoilWeight = 0.f;

		if (bIsReloading || bIsEquipping)
		{
			WeaponRecoilWeight = 1.f;
		}
	}
	else
	{
		if (bIsCrouching)
		{
			if (bIsReloading || bIsEquipping)
			{
				WeaponRecoilWeight = 1.f;
			}
			else
			{
				WeaponRecoilWeight = 0.1f;
			}
		}
		else
		{
			if (bIsAiming || bIsReloading || bIsEquipping)
			{
				WeaponRecoilWeight = 1.f;
			}
			else
			{
				WeaponRecoilWeight = 0.5f;
			}
		}
	}
}

void UParagonAnimInstance::Lean(float DeltaTime)
{
	if (!ParagonCharacter)
		return;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ParagonCharacter->GetActorRotation();

	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };

	const float Target{ Delta.Yaw / DeltaTime };
	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	if (!GEngine)
		return;
}
