// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"	
#include "ParagonCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"

// Sets default values
AItemBase::AItemBase() :
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_PickUp),
	//Item interp variables
	InterpTimerDuration(0.7f),
	ItemInterpStartLocation(FVector(0.f)),
	CameraTargetLocation(FVector(0.f)),
	bIsInterping(false),
	ItemInterpX(0.f),
	ItemInterpY(0.f),
	InterpInitalYawOffset(0.f),
	ItemType(EItemType::EIT_MAX),
	InterpLocationIndex(0),
	MaterialIndex(0),
	bIsCanChangeCustomDepth(true),
	// Dymanic material parameters
	GlowAmount(50.f),
	FresnelExponent(3.f),
	FresnelReflectFraction(4.f),
	PulseCurveTime(4.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupInfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickupInfoWidget->SetupAttachment(RootComponent);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Hide pick up widget by default
	if (!PickupInfoWidget)
		return;

	PickupInfoWidget->SetVisibility(false);

	//Setup overlap for AreaSphere
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnSphereBeginOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItemBase::OnSphereEndOverlap);

	//Initialize stars array
	UpdateActiveStars();

	//Set Item properties based on ItemState
	SetItemProperties(ItemState);

	// Set CustomDepth to disabled by default
	InitializeCustomDepth();

	// Start infinite looping timer to pulse dynamic material of glow
	ResetPulseTimer();
}

void AItemBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;

	AParagonCharacter* ParagonCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!ParagonCharacter)
		return;

	ParagonCharacter->ChangeOverlappedItemCount(1);

}

void AItemBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent,int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;

	AParagonCharacter* ParagonCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!ParagonCharacter)
		return;

	ParagonCharacter->ChangeOverlappedItemCount(-1);
}

void AItemBase::UpdateActiveStars()
{
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
	{
		ActiveStars[1] = true;
	}
		break;
	case EItemRarity::EIR_Common:
	{
		ActiveStars[1] = true;
		ActiveStars[2] = true;
	}
		break;
	case EItemRarity::EIR_Uncommon:
	{
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
	}
		break;
	case EItemRarity::EIR_Rare:
	{
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
	}
		break;
	case EItemRarity::EIR_Legendary:
	{
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
	}
		break;
	default:
		break;
	}

}

void AItemBase::SetItemProperties(EItemState State)
{
	switch (State)
	{
	default:
		break;
	case EItemState::EIS_PickUp:
	{
		//Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set area sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		//Set collision box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		break;
	}
	case EItemState::EIS_EquipInterping:
	{
		//Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set area sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set collision box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Hide weapon widget
		PickupInfoWidget->SetVisibility(false);
		break;
	}
	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Equipped:
	{
		//Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set area sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set collision box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Hide weapon widget
		PickupInfoWidget->SetVisibility(false);

		break;
	}
	case EItemState::EIS_Falling:
	{
		//Set mesh properties
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		//Set area sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set collision box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;
	}
	case EItemState::EIS_MAX:
		break;
	}
}

void AItemBase::FinishFlying()
{
	if (!PlayerCharacter)
		return;

	PlayerCharacter->IncrementInterpLocationCount(InterpLocationIndex, -1);

	PlayerCharacter->GetPickupItem(this);
	bIsInterping = false;

	//Set scale back to normal
	SetActorScale3D(FVector(1.f));

	bIsCanChangeCustomDepth = true;

	DisableGlowMaterial();
	DisableCustomDepth();
}

void AItemBase::ItemInterp(float DeltaTime)
{
	if (!bIsInterping)
		return;

	if (!PlayerCharacter)
		return;

	if (!ItemZCurve)
		return;
	
	//Elapsed time since we started ItemInterp timer
	const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);

	//Get curve value corresponding to ElapsedTime
	const float CurrentCurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

	//Get item's initial location when the curve started
	FVector ItemLocation = ItemInterpStartLocation;

	//Get location in front of the camera
	const FVector CameraInterpLocation{ GetInterpLocation() };

	//Vector from Item to CameraInterpLocation, X and Y are zeroed out
	const FVector ItemToCameraDelta{ FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z) };

	//Scale factor to multiply with CurrentCurveValue
	const float DeltaZ = ItemToCameraDelta.Size();

	//Adding curve value to the Z component of the initial location (scaled by DeltaZ)
	ItemLocation.Z += CurrentCurveValue * DeltaZ;

	const FVector CurrentItemLocation = GetActorLocation();

	//Interpolated X and Y values
	const float InterpXValue = FMath::FInterpTo(CurrentItemLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
	const float InterpYValue = FMath::FInterpTo(CurrentItemLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);

	//Set X and Y of ItemLocation to Interped values
	ItemLocation.X = InterpXValue;
	ItemLocation.Y = InterpYValue;

	SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

	//Camera rotation this frame
	const FRotator CurrCameraRotation = PlayerCharacter->GetFollowCamera()->GetComponentRotation();

	//Camera rotation + inital Yaw offset
	const FRotator ItemRotation{ 0.f, CurrCameraRotation.Yaw + InterpInitalYawOffset, 0.f };

	SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

	if (!ItemScaleCurve)
		return;

	const float CurrScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
	SetActorScale3D(FVector(CurrScaleCurveValue, CurrScaleCurveValue, CurrScaleCurveValue));
}

void AItemBase::PlayPickupSound()
{
	if (!PlayerCharacter)
		return;

	if (!GetPickupSound())
		return;

	if (!PlayerCharacter->GetIsShouldPlayPickUpSound())
		return;

	UGameplayStatics::PlaySound2D(GetWorld(), GetPickupSound());
	PlayerCharacter->StartPickUpSoundTimer();
}

void AItemBase::PlayEquipSound()
{
	if (!PlayerCharacter)
		return;

	if (!GetEquipSound())
		return;

	if (!PlayerCharacter->GetIsShouldPlayEquipSound())
		return;

	UGameplayStatics::PlaySound2D(GetWorld(), GetEquipSound());
	PlayerCharacter->StartEquipSoundTimer();
}

FVector AItemBase::GetInterpLocation()
{
	if (!PlayerCharacter)
		return FVector();

	switch (ItemType)
	{
	case EItemType::EIT_Weapon:
	{
		return PlayerCharacter->GetInterpLocation(0).SceneComponent->GetComponentLocation();
	}
	break;
	case EItemType::EIT_Ammo:
	{
		return PlayerCharacter->GetInterpLocation(InterpLocationIndex).SceneComponent->GetComponentLocation();
	}
	break;
	}

	return FVector();
}

void AItemBase::OnConstruction(const FTransform& Transform)
{
	if (!MaterialInstance)
		return;

	DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
	ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
	EnableGlowMaterial();
}

void AItemBase::ResetPulseTimer()
{
	if (ItemState != EItemState::EIS_PickUp)
		return;

	GetWorldTimerManager().SetTimer(PulseTimer, this, &AItemBase::ResetPulseTimer, PulseCurveTime);
}

void AItemBase::UpdatePulse()
{
	if (ItemState != EItemState::EIS_PickUp)
		return;

	if (!PulseCurve)
		return;

	const float ElapsedTime{ GetWorldTimerManager().GetTimerElapsed(PulseTimer) };
	
	const FVector CurrentCurveValue{ PulseCurve->GetVectorValue(ElapsedTime) };

	const FName GlowParameterName = TEXT("GlowAmount");
	const FName FresnelExponentParameterName = TEXT("FresnelExponent");
	const FName FresnelReflectFractionParameterName = TEXT("FresnelReflectFraction");

	DynamicMaterialInstance->SetScalarParameterValue(GlowParameterName, CurrentCurveValue.X * GlowAmount);
	DynamicMaterialInstance->SetScalarParameterValue(FresnelExponentParameterName, CurrentCurveValue.Y * FresnelExponent);
	DynamicMaterialInstance->SetScalarParameterValue(FresnelReflectFractionParameterName, CurrentCurveValue.Z * FresnelReflectFraction);
}

void AItemBase::EnableGlowMaterial()
{
	if (!DynamicMaterialInstance)
		return;

	const FName& ParameterName{ TEXT("GlowBlendAlpha") };

	DynamicMaterialInstance->SetScalarParameterValue(ParameterName, 0);
}

void AItemBase::DisableGlowMaterial()
{
	if (!DynamicMaterialInstance)
		return;

	const FName& ParameterName{ TEXT("GlowBlendAlpha") };

	DynamicMaterialInstance->SetScalarParameterValue(ParameterName, 1);
}

void AItemBase::EnableCustomDepth()
{
	if (!bIsCanChangeCustomDepth)
		return;

	ItemMesh->SetRenderCustomDepth(true);
}

void AItemBase::DisableCustomDepth()
{
	if (!bIsCanChangeCustomDepth)
		return;

	ItemMesh->SetRenderCustomDepth(false);
}

void AItemBase::InitializeCustomDepth()
{
	DisableCustomDepth();
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Handle item interping when in the EquipInterping state
	ItemInterp(DeltaTime);

	//Get curve values from PulseCurve and set dynamic material parameters
	UpdatePulse();
}

void AItemBase::UpdateItemProperties()
{
	SetItemProperties(ItemState);
}

void AItemBase::StartItemFlying(AParagonCharacter* Character)
{
	PlayerCharacter = Character;

	/// Get free or the less busy place to interp to
	InterpLocationIndex = Character->GetInterpLocationIndex();
	Character->IncrementInterpLocationCount(InterpLocationIndex, 1);

	//Store initial location of the item
	ItemInterpStartLocation = GetActorLocation();
	bIsInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItemBase::FinishFlying, InterpTimerDuration);

	//Get initial Yaw of the Camera
	const float CameraRotationYaw = PlayerCharacter->GetFollowCamera()->GetComponentRotation().Yaw;

	//Get inital Yaw of the Item
	const float ItemRotationYaw = GetActorRotation().Yaw;

	//Inital Yaw offste between Camera and Item
	InterpInitalYawOffset = ItemRotationYaw - CameraRotationYaw;

	PlayPickupSound();

	bIsCanChangeCustomDepth = false;
} 

