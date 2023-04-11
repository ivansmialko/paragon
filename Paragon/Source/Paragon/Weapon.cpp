// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon():
	ThrowWeaponTime(0.7f),
	bIsFalling(false),
	AmmoAmount(35),
	WeaponType(EWeaponType::EWT_SMG),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(FName(TEXT("Reload SMG"))),
	MagazineCapacity(35),
	ClipBoneName(TEXT("smg_clip"))
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Keep weapon upright
	if (GetItemState() == EItemState::EIS_Falling && bIsFalling)
	{
		const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
	const FVector MeshRight{ GetItemMesh()->GetRightVector() };

	//Direction in which we throw the weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation{ 30.f };
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 2'000.f;
	
	UE_LOG(LogTemp, Warning, TEXT("Current impulse: %f, %f, %f"), ImpulseDirection.X, ImpulseDirection.Y, ImpulseDirection.Z);

	GetItemMesh()->AddImpulse(ImpulseDirection);

	bIsFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime); 

	EnableGlowMaterial();
}

void AWeapon::DecrementAmmoAmount()
{
	if (AmmoAmount - 1 <= 0)
	{
		AmmoAmount = 0;
		return;
	}

	AmmoAmount--;
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	AmmoAmount += Amount;
}

bool AWeapon::GetIsClipFull()
{
	return (AmmoAmount == MagazineCapacity);
}

void AWeapon::StopFalling()
{
	bIsFalling = false;
	SetItemState(EItemState::EIS_PickUp);
	UpdateItemProperties();
}
