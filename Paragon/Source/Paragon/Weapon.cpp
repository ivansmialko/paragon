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

	ResetPulseTimer();
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	const FString WeaponTablePath = TEXT("DataTable'/Game/_Game/DataTables/DT_Weapons.DT_Weapons'");
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (!WeaponTableObject)
		return;

	FWeaponDataTableRow* WeaponDataRow{ nullptr };

	switch (WeaponType)
	{
	case EWeaponType::EWT_SMG:
	{
		WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTableRow>(FName("SubmachineGun"), TEXT(""));
		break;
	}
	case EWeaponType::EWT_AR:
	{
		WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTableRow>(FName("AssaultRifle"), TEXT(""));
		break;
	}
	default:
		break;
	case EWeaponType::EWT_PISTOL:
	{
		WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTableRow>(FName("Pistol"), TEXT(""));
		break;
	}
	}

	if (!WeaponDataRow)
		return;

	AmmoType = WeaponDataRow->AmmoType;
	AmmoAmount = WeaponDataRow->WeaponAmmo;
	MagazineCapacity = WeaponDataRow->MagazineCapacity;

	SetEquipSound(WeaponDataRow->EquipSound);
	SetPickupSound(WeaponDataRow->PickupSound);

	GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);

	SetItemName(WeaponDataRow->ItemName);

	SetIconItem(WeaponDataRow->InventoryIcon);
	SetIconAmmo(WeaponDataRow->AmmoIcon);

	SetLastMaterialIndex(GetMaterialIndex());
	GetItemMesh()->SetMaterial(GetLastMaterialIndex(), nullptr);

	SetMaterialInstance(WeaponDataRow->MaterialInstance);
	SetMaterialIndex(WeaponDataRow->MaterialIndex);

	if (GetMaterialInstance())
	{
		SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
		GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
		GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
		EnableGlowMaterial();
	}

	SetClipBoneName(WeaponDataRow->ClipBoneName);
	SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
	GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);

	CrosshairsMiddle = WeaponDataRow->CrosshairsMiddle;
	CrosshairsLeft = WeaponDataRow->CrosshairsLeft;
	CrosshairsRight = WeaponDataRow->CrosshairsRight;
	CrosshairsTop = WeaponDataRow->CrosshairsTop;
	CrosshairsBottom = WeaponDataRow->CrosshairsBottom;

	FireRate = WeaponDataRow->FireRate;
	MuzzleFlash = WeaponDataRow->MuzzleFlash;
	FireSound = WeaponDataRow->FireSound;
	BoneToHide = WeaponDataRow->BoneToHide;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (BoneToHide == FName())
		return;

	GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
}
