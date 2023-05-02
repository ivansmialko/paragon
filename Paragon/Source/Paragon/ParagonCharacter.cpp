// Fill out your copyright notice in the Description page of Project Settings.


#include "ParagonCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/EngineTypes.h"

#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"

#include "DrawDebugHelpers.h"
#include "ItemBase.h"
#include "Weapon.h"
#include "Ammo.h"
#include "ImpactPoint.h"



// Sets default values
AParagonCharacter::AParagonCharacter() :
	//Base rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	//Aiming/not aiming controller sensitivity
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	//Aiming/not aiming mouse sensitivity
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),
	bIsAiming(false),
	//Combat camera settings
	CameraDefaultFOV(0.f), //Setting this in begin play
	CameraZoomFOV(25.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	CameraOffset(FVector(0.f, 50.f, 70.f)),
	//Crosshair spread settings
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityScaleFactor(0.f),
	CrosshairInAirScaleFactor(0.f),
	CrosshairAimScaleFactor(0.f),
	//Bullet fire timer
	ShootTimerDuration(0.05f),
	bIsFiringBullet(false),
	//Automatic gun fire settings
	bIsFireButtonPressed(false),
	bIsShouldFireAtThisFrame(true),
	AutomaticFireRate(0.1f),
	//Item trace variables
	bIsShouldTraceForItems(false),
	TraceHitLastFrame(nullptr),
	//Item to camera interpolationg setting
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	//Starting ammo amount
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	//Combat variables
	CurrentCombatState(ECombatState::ECS_Unoccupied),
	bIsCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	CurrentCapsuleHalfHeight(0.f),
	StandindCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),
	BaseGroundFriction(2.0f),
	CrouchingGroundFriction(100.f),
	bIsAimingButtonPressed(false),
	//Pickup sound timer properties
	bIsShouldPlayEquipSound(true),
	bIsShouldPlayPickUpSound(true),
	PickUpSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f),
	//Icon animation property
	CurrentHighlightedSlot(-1)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; //The camera foollows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller

	//Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);	//Attach camera to the end of boom
	FollowCamera->bUsePawnControlRotation = false; //Camera does not rotate relative to arm

	//Don't rotate mesh when the controller rotates. Let controller only rotate the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; //Character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //.. at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Create Hand Scene component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	// Create a scene component that will be used to interpolate weapon position to
	InterpPlaceWeapon = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation destination place"));
	InterpPlaceWeapon->SetupAttachment(GetFollowCamera());

	// Create a scene component that will be used to interpolate different items' position to
	InterpPlace1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation place #1"));
	InterpPlace1->SetupAttachment(GetFollowCamera());
	InterpPlace2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation place #2"));
	InterpPlace2->SetupAttachment(GetFollowCamera());
	InterpPlace3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation place #3"));
	InterpPlace3->SetupAttachment(GetFollowCamera());
	InterpPlace4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation place #4"));
	InterpPlace4->SetupAttachment(GetFollowCamera());
	InterpPlace5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation place #5"));
	InterpPlace5->SetupAttachment(GetFollowCamera());
	InterpPlace6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation place #6"));
	InterpPlace6->SetupAttachment(GetFollowCamera());
}

// Called when the game starts or when spawned
void AParagonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!FollowCamera)
		return;

	if (!CameraBoom)
		return;

	CameraDefaultFOV = FollowCamera->FieldOfView;
	CameraCurrentFOV = CameraDefaultFOV;
	CameraBoom->SocketOffset = CameraOffset;

	//Spawn and equip the default weapon and attach it to the mesh
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacterRef(this);

	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	//Create FInterpLocation structs for each interp location components
	InitializeInterpLocations();
}

void AParagonCharacter::MoveForward(float in_value)
{
	if ((Controller != nullptr) && in_value != 0.f)
	{
		//Find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };

		AddMovementInput(Direction, in_value);
	}
}

void AParagonCharacter::MoveRight(float in_value)
{
	if ((Controller != nullptr) && in_value != 0.f)
	{
		//Find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };

		AddMovementInput(Direction, in_value);
	}
}

void AParagonCharacter::TurnAtRate(float in_rate)
{
	//Calculate delta for current frame based on rate information
	AddControllerYawInput(in_rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); //deg/sec * sec/frame = deg/frame
}

void AParagonCharacter::LookUpAtRate(float in_rate)
{
	//Calculate delta for current frame based on rate information
	AddControllerPitchInput(in_rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); //deg/sec * sec/frame = deg/frame
}

void AParagonCharacter::Turn(float Value)
{
	float TurnScaleFactor = 1.f;

	if (bIsAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AParagonCharacter::LookUp(float Value)
{
	float LookUpScaleFactor = 1.f;

	if (bIsAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}

	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AParagonCharacter::FireWeapon()
{
	if (CurrentCombatState != ECombatState::ECS_Unoccupied)
		return;	

	if (!EquippedWeapon)
		return;

	if (!IsWeaponHasAmmo())
		return;

	//Launch trace, display particles
	FireSendBullet();

	//Play gunshot sound
	FirePlaySound();

	//Play firing animation montage
	FirePlayAnim();

	//Play vibration feedback to controller
	FirePlayFeedback();
	 
	//Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();

	//Remove one bullet from magazine
	EquippedWeapon->DecrementAmmoAmount();

	//Start timer to control fire rate
	StartFireTimer();
}

bool AParagonCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	//Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit) {
		//Tentatice beam location - still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else //No crosshairs hit
	{
		//Out beam location is the End location for the line trace
	}

	//Perform a second trace, this time from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };

	//Object between barrel and beam endpoint
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}

	return false;
}

void AParagonCharacter::SetAimingButtonPressed()
{
	bIsAimingButtonPressed = true;

	if (CurrentCombatState != ECombatState::ECS_ReloadingState)
	{
		StartAiming();
	}
}

void AParagonCharacter::SetAimingButtonReleased()
{
	bIsAimingButtonPressed = false;
	StopAiming();
}

void AParagonCharacter::ZoomCameraInterp(float DeltaTime)
{
	//Set current camera field of view. Interpolate from default to zoomed FOV or backwards
	CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, (bIsAiming ? CameraZoomFOV : CameraDefaultFOV), DeltaTime, ZoomInterpSpeed);
	FollowCamera->SetFieldOfView(CameraCurrentFOV);
}

void AParagonCharacter::UpdateLookRates()
{
	//Change look sensitivity based on aiming or not
	if (bIsAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AParagonCharacter::UpdateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, GetCharacterMovement()->MaxWalkSpeed };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };

	FVector CurrentVelocity(GetVelocity());
	CurrentVelocity.Z = 0.f;

	//Calculate crosshair velocity factor
	CrosshairVelocityScaleFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, CurrentVelocity.Size());

	//Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling())
	{
		//Spread the crosshairs slowly while in air
		CrosshairInAirScaleFactor = FMath::FInterpTo(CrosshairInAirScaleFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{	
		//Shrink the crosshairs radiply when on the ground
		CrosshairInAirScaleFactor = FMath::FInterpTo(CrosshairInAirScaleFactor, 0.f, DeltaTime, 30.f);
	}

	//Calculate crosshair aiming factor
	if (bIsAiming)
	{
		//Shrink the crosshairs slowly while aiming
		CrosshairAimScaleFactor = FMath::FInterpTo(CrosshairAimScaleFactor, -0.5f, DeltaTime, 5.f);
	}
	else
	{
		//Spread the crosshairs rapidly when not aiming
		CrosshairAimScaleFactor = FMath::FInterpTo(CrosshairAimScaleFactor, 0.f, DeltaTime, 30.f);
	}

	//True 0.05 seconds after firing
	if (bIsFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 10.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityScaleFactor + CrosshairInAirScaleFactor + CrosshairAimScaleFactor + CrosshairShootingFactor;
}

void AParagonCharacter::StartCrosshairBulletFire()
{
	bIsFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AParagonCharacter::FinishCrosshairBulletFire, ShootTimerDuration);
}

void AParagonCharacter::FinishCrosshairBulletFire()
{
	bIsFiringBullet = false;
}

void AParagonCharacter::FireButtonPressed()
{
	bIsFireButtonPressed = true;
	FireWeapon();
}

void AParagonCharacter::FireButtonReleased()
{
	bIsFireButtonPressed = false;
}

void AParagonCharacter::StartFireTimer()
{
	CurrentCombatState = ECombatState::ECS_FireTimerInProgress;

	//Timer to wait for a new bullet to fire
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AParagonCharacter::FireTimerCallback, AutomaticFireRate);
}

void AParagonCharacter::FireTimerCallback()
{
	CurrentCombatState = ECombatState::ECS_Unoccupied;

	if (!IsWeaponHasAmmo())
	{
		//Reloading if no ammo in the magazine
		ReloadWeapon();
		return;
	}

	if (!bIsFireButtonPressed)
		return;

	//If fire button still pressed - fire another bullet
	FireWeapon();
}

bool AParagonCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//Get viewport size
	if (!GEngine || !GEngine->GameViewport)
		return false;

	//Get current size of the viewport
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	//Get screen-space location of crosshair
	FVector2D CrosshairLocation(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f);

	//Get world position and direction 
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation,
		CrosshairWorldPosition, CrosshairWorldDirection);

	//Was deprojection successfull?
	if (!bScreenToWorld)
		return false;

	FHitResult ScreenTraceHit;
	const FVector LineStart{ CrosshairWorldPosition };
	const FVector LineEnd{ CrosshairWorldPosition + CrosshairWorldDirection * 5'000.f };
	OutHitLocation = LineEnd;

	GetWorld()->LineTraceSingleByChannel(OutHitResult, LineStart, LineEnd, ECollisionChannel::ECC_Visibility);

	if (OutHitResult.bBlockingHit)
	{
		OutHitLocation = OutHitResult.Location;
	}

	return OutHitResult.bBlockingHit;
}

void AParagonCharacter::TraceForItems()
{
	if (!bIsShouldTraceForItems)
	{
		if (TraceHitLastFrame)
		{
			TraceHitLastFrame->GetPickupWidget()->SetVisibility(false);

			//Disable border glow for an item
			TraceHitLastFrame->DisableCustomDepth();
		}

		return;
	}

	FHitResult ItemTraceResult;
	FVector HitLocation;
	if (TraceUnderCrosshairs(ItemTraceResult, HitLocation))
	{
		TraceHitItem = Cast<AItemBase>(ItemTraceResult.Actor);

		const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		if (TraceHitWeapon)
		{
			HighlightInventorySlot();
		}
		else
		{
			UnHighlightInventorySlot();
		}

		if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
		{
			TraceHitItem = nullptr;
		}

		if (TraceHitItem && TraceHitItem->GetPickupWidget())
		{
			//Shot Item's pick up widget
			TraceHitItem->GetPickupWidget()->SetVisibility(true);

			//Enable border glow for an item
			TraceHitItem->EnableCustomDepth();

			//Check if inventory is full
			TraceHitItem->SetIsCharacterInventoryFull(Inventory.Num() >= INVENTORY_CAPACITY);
		}
		
		//We hit an AItemBase last frame
		if (TraceHitLastFrame && (TraceHitLastFrame != TraceHitItem))
		{
			//We are hitting a different AItemBase this frame from last frame
			TraceHitLastFrame->GetPickupWidget()->SetVisibility(false);

			//Disable border glow for an item
			TraceHitLastFrame->DisableCustomDepth();
		}

		TraceHitLastFrame = TraceHitItem;
	}
}

AWeapon* AParagonCharacter::SpawnDefaultWeapon()
{
	if (!DefaultWeaponClass)
		return nullptr;

	//Spawn the weapon
	AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	return DefaultWeapon;
}

void AParagonCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bIsSwapping /*= false*/)
{
	if (!WeaponToEquip)
		return;

	//Get the hand socket
	const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("right_hand_socket"));
	if (!HandSocket)
		return;

	//Attach the weapon to the hand socket
	HandSocket->AttachActor(WeaponToEquip, GetMesh());

	if (!EquippedWeapon)
	{
		//-1 = no equipped weapon yet. No need to reverse the icon animation
		EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
	}
	else if(!bIsSwapping)
	{
		EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	EquippedWeapon->UpdateItemProperties();
}

void AParagonCharacter::DropWeapon()
{
	if (!EquippedWeapon)
		return;

	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
	EquippedWeapon->SetItemState(EItemState::EIS_Falling);
	EquippedWeapon->UpdateItemProperties();
	EquippedWeapon->ThrowWeapon();
}

void AParagonCharacter::SelectButtonPressed()
{
	if (CurrentCombatState != ECombatState::ECS_Unoccupied)
		return;

	if (!TraceHitItem)
		return;

	auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
	if (!TraceHitItem)
		return;

	TraceHitItem->StartItemFlying(this, true);
	TraceHitItem = nullptr;
}

void AParagonCharacter::SelectButtonReleased()
{

}

void AParagonCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}

	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitLastFrame = nullptr;
}

void AParagonCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AParagonCharacter::IsWeaponHasAmmo()
{
	if (!EquippedWeapon)
		return false;

	return (EquippedWeapon->GetAmmoAmount() > 0);
}

void AParagonCharacter::FirePlaySound()
{
	if (!FireSound)
		return;

	//Play fire sound
	UGameplayStatics::PlaySound2D(GetWorld(), FireSound);
}

void AParagonCharacter::FireSendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("barrel_socket");
	if (!BarrelSocket)
		return;

	//Get location of barrel, where the bullet is flying out
	const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

	if (!MuzzleFlash)
		return;

	//Spawn muzzle flash particles
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);

	//Calculate point of impact of the bullet
	FVector ImpactPoint;
	bool bResult = GetBeamEndLocation(SocketTransform.GetLocation(), ImpactPoint);

	if (bResult)
	{
		if (!ImpactParticles)
			return;

		//Spawn impact particles
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, ImpactPoint);

		SpawnImpactPoint(ImpactPoint);
	}

	if (!BeamParticles)
		return;

	//Spawn bullet trace particles
	UParticleSystemComponent* BulletBeamParticles = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
	if (!BulletBeamParticles)
		return;

	BulletBeamParticles->SetVectorParameter(FName("Target"), ImpactPoint);
}

void AParagonCharacter::FirePlayAnim()
{
	//Play player character animation of shooting
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !FireRecoilMontage)
		return;

	AnimInstance->Montage_Play(FireRecoilMontage);
	AnimInstance->Montage_JumpToSection(FName("WeaponFire"));
}

void AParagonCharacter::FirePlayFeedback()
{
	if (!FeedbackFire)
		return;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
		return;

	PlayerController->ClientPlayForceFeedback(FeedbackFire, false, FName(TEXT("FeedbackFire")));
}

void AParagonCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AParagonCharacter::ReloadWeapon()
{
	if (!EquippedWeapon)
		return;

	if (CurrentCombatState != ECombatState::ECS_Unoccupied)
		return;

	if (!IsHaveAmmo())
		return;

	if (EquippedWeapon->GetIsClipFull())
		return;

	CurrentCombatState = ECombatState::ECS_ReloadingState;

	if (!ReloadMontage)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	if (bIsAiming)
	{
		StopAiming();
	}

	AnimInstance->Montage_Play(ReloadMontage);
	AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());

}

void AParagonCharacter::FinishReloading()
{
	//Update the combat state
	CurrentCombatState = ECombatState::ECS_Unoccupied;

	if (!EquippedWeapon)
		return;

	const auto AmmoType{ EquippedWeapon->GetAmmoType() };

	//Update the AmmoMap
	if (!AmmoMap.Contains(AmmoType))
		return;

	//Amount of ammo the character is carrying of the equipped weapon type
	int32 CarriedAmmo = AmmoMap[AmmoType];

	//Space left in the magazine of EquippedWeapon
	const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmoAmount();
	
	if (MagEmptySpace > CarriedAmmo)
	{
		//Reload the magazine with all the weapon we are carrying
		EquippedWeapon->ReloadAmmo(CarriedAmmo);
		CarriedAmmo = 0;
	}
	else
	{
		//Fill the magazine
		EquippedWeapon->ReloadAmmo(MagEmptySpace);
		CarriedAmmo -= MagEmptySpace;
	}

	//Update amount of carried ammo
	AmmoMap.Add(AmmoType, CarriedAmmo);

	if (bIsAimingButtonPressed)
	{
		StartAiming();
	}
}

void AParagonCharacter::FinishEquipping()
{
	CurrentCombatState = ECombatState::ECS_Unoccupied;
}

bool AParagonCharacter::IsHaveAmmo()
{
	if (!EquippedWeapon)
		return false;

	auto CurrAmmoType = EquippedWeapon->GetAmmoType();

	if (!AmmoMap.Contains(CurrAmmoType))
		return false;

	return (AmmoMap[CurrAmmoType] > 0);

	return false;
}

void AParagonCharacter::CrouchButtonPressed()
{
	if (GetCharacterMovement()->IsFalling())
		return;

	bIsCrouching = !bIsCrouching;
	
	if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AParagonCharacter::Jump()
{
	if (bIsCrouching)
	{
		bIsCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}

}

void AParagonCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight = (bIsCrouching ? CrouchingCapsuleHalfHeight : StandindCapsuleHalfHeight);
	const float InterpHalfHeight{ FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f) };
	
	//Negative value if crouching, positive value if standing
	float DeltaCapsuleHalfHeight{ InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };

	const FVector MeshOffset(0.f, 0.f, -DeltaCapsuleHalfHeight);
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}


int32 AParagonCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;
	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AParagonCharacter::IncrementInterpLocationCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1)
		return;

	if (Index >= InterpLocations.Num())
		return;

	InterpLocations[Index].ItemCount += Amount;
}

void AParagonCharacter::StartPickUpSoundTimer()
{
	bIsShouldPlayPickUpSound = false;
	GetWorldTimerManager().SetTimer(PickUpSoundTimer, this, &AParagonCharacter::ResetPickUpSoundTimer, PickUpSoundResetTime);
}

void AParagonCharacter::StartEquipSoundTimer()
{
	bIsShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AParagonCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

void AParagonCharacter::AddInventoryItem(AItemBase* ItemToAdd)
{

}

// Called every frame
void AParagonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ZoomCameraInterp(DeltaTime);
	UpdateLookRates();
	UpdateCrosshairSpread(DeltaTime);
	TraceForItems();

	//Interpolate capsule's half height based on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AParagonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AParagonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AParagonCharacter::MoveRight);

	PlayerInputComponent->BindAxis("TurnRate", this, &AParagonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AParagonCharacter::LookUpAtRate);

	PlayerInputComponent->BindAxis("Turn", this, &AParagonCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AParagonCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AParagonCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AParagonCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AParagonCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AParagonCharacter::SetAimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AParagonCharacter::SetAimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AParagonCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AParagonCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AParagonCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AParagonCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AParagonCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AParagonCharacter::Key1Pressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AParagonCharacter::Key2Pressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AParagonCharacter::Key3Pressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AParagonCharacter::Key4Pressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AParagonCharacter::Key5Pressed);
}

void AParagonCharacter::GrabClip()
{
	if (!EquippedWeapon)
		return;

	if (!HandSceneComponent)
		return;

	//Index for the clip bone on the EquippedWeapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };

	if (ClipBoneIndex < 0)
		return;
	
	//Store the transform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AParagonCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AParagonCharacter::HideClip()
{
	if (!EquippedWeapon)
		return;

	if (!HandSceneComponent)
		return;

	//Index for the clip bone on the EquippedWeapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };

	if (ClipBoneIndex < 0)
		return;

	//Hide the clip bone to hide used magazine
	EquippedWeapon->GetItemMesh()->HideBone(ClipBoneIndex, EPhysBodyOp::PBO_None);
}

void AParagonCharacter::ShowClip()
{
	if (!EquippedWeapon)
		return;

	if (!HandSceneComponent)
		return;

	//Index for the clip bone on the EquippedWeapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };

	if (ClipBoneIndex < 0)
		return;

	//Hide the clip bone to show new magazine that have been taken from pocket
	EquippedWeapon->GetItemMesh()->UnHideBone(ClipBoneIndex);
}

void AParagonCharacter::StartAiming()
{
	bIsAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AParagonCharacter::StopAiming()
{
	bIsAiming = false;

	if (!bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AParagonCharacter::PickupAmmo(class AAmmo* Ammo)
{
	int32 AmmoCount{ 0 };

	//Check to see if AmmoMap contains Ammo's AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		AmmoCount = AmmoMap[Ammo->GetAmmoType()];
	}

	AmmoCount += Ammo->GetItemCount();
	AmmoMap[Ammo->GetAmmoType()] = AmmoCount;

	if (Ammo->GetAmmoType() == EquippedWeapon->GetAmmoType())
	{
		if (EquippedWeapon->GetAmmoAmount() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy(); 
}

void AParagonCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ InterpPlaceWeapon, 0 };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpPlace1, 0 };
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{ InterpPlace2, 0 };
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{ InterpPlace3, 0 };
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{ InterpPlace4, 0 };
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{ InterpPlace5, 0 };
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{ InterpPlace6, 0 };
	InterpLocations.Add(InterpLoc6);
}

void AParagonCharacter::SpawnImpactPoint(const FVector& ImpactPlace)
{
	if (!WeaponImpactPointClass)
		return;

	//Spawn the weapon
	AImpactPoint* ImpactPoint = GetWorld()->SpawnActor<AImpactPoint>(WeaponImpactPointClass);
	ImpactPoint->SetImpactAmount(50);
	ImpactPoint->SetActorLocation(ImpactPlace);
	ImpactPoint->SetLifeSpan(0.5f);
}

void AParagonCharacter::ResetPickUpSoundTimer()
{
	bIsShouldPlayPickUpSound = true;
}

void AParagonCharacter::ResetEquipSoundTimer()
{
	bIsShouldPlayEquipSound = true;
}

void AParagonCharacter::FKeyPressed()
{
	if (!EquippedWeapon)
		return;

	if (EquippedWeapon->GetSlotIndex() == 0)
		return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AParagonCharacter::Key1Pressed()
{
	if (!EquippedWeapon)
		return;

	if (EquippedWeapon->GetSlotIndex() == 1)
		return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AParagonCharacter::Key2Pressed()
{
	if (!EquippedWeapon)
		return;

	if (EquippedWeapon->GetSlotIndex() == 2)
		return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AParagonCharacter::Key3Pressed()
{
	if (!EquippedWeapon)
		return;

	if (EquippedWeapon->GetSlotIndex() == 3)
		return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AParagonCharacter::Key4Pressed()
{
	if (!EquippedWeapon)
		return;

	if (EquippedWeapon->GetSlotIndex() == 4)
		return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AParagonCharacter::Key5Pressed()
{
	if (!EquippedWeapon)
		return;

	if (EquippedWeapon->GetSlotIndex() == 5)
		return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AParagonCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if (!(CurrentCombatState == ECombatState::ECS_Unoccupied || CurrentCombatState == ECombatState::ECS_Equipping))
		return;

	if(CurrentItemIndex == NewItemIndex)
		return;

	if (NewItemIndex > Inventory.Num() - 1 || CurrentItemIndex > Inventory.Num() - 1)
		return;

	auto OldEquippedWeapon = EquippedWeapon;
	auto NewWeaponToEquip = Cast<AWeapon>(Inventory[NewItemIndex]);
	if (!NewWeaponToEquip)
		return;

	EquipWeapon(NewWeaponToEquip);
	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	OldEquippedWeapon->UpdateItemProperties();

	NewWeaponToEquip->SetItemState(EItemState::EIS_Equipped);
	NewWeaponToEquip->UpdateItemProperties();
	NewWeaponToEquip->PlayEquipSound(true);

	CurrentCombatState = ECombatState::ECS_Equipping;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	if (!EquipMontage)
		return;

	AnimInstance->Montage_Play(EquipMontage, 1.0f);
	AnimInstance->Montage_JumpToSection(FName("Equip"));
}

int32 AParagonCharacter::GetEmptyInventorySlotIndex()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (!Inventory[i])
			return i;
	}

	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}

	return -1; //Inventory is full
}

void AParagonCharacter::HighlightInventorySlot()
{
	const int32 EmptySlotIndex = GetEmptyInventorySlotIndex();

	if (EmptySlotIndex < 0 || CurrentHighlightedSlot > 0)
		return;

	HighlightIconDelegate.Broadcast(EmptySlotIndex, true);

	CurrentHighlightedSlot = EmptySlotIndex;
}

void AParagonCharacter::UnHighlightInventorySlot()
{
	if (CurrentHighlightedSlot < 0)
		return;

	HighlightIconDelegate.Broadcast(CurrentHighlightedSlot, false);

	CurrentHighlightedSlot = -1;
}

void AParagonCharacter::FireBeginEvent()
{
	FireButtonPressed();
}

void AParagonCharacter::FireEndEvent()
{
	FireButtonReleased();
}

void AParagonCharacter::FireSingleEvent()
{
	FireWeapon();
}

void AParagonCharacter::JumpEvent()
{
	Jump();
}

void AParagonCharacter::AimEvent()
{
	if (bIsAiming)
	{
		SetAimingButtonReleased();
	}
	else
	{
		SetAimingButtonPressed();
	}
}

void AParagonCharacter::UseBeginEvent()
{
	SelectButtonPressed();
}

void AParagonCharacter::UseEndEvent()
{
	SelectButtonReleased();
}

void AParagonCharacter::ChangeOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bIsShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bIsShouldTraceForItems = true;
	}
}

FVector AParagonCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForwardVector{ FollowCamera->GetForwardVector() };
	const FVector CameraUpwardVector(0.f, 0.f, CameraInterpElevation);

	//Desired = CameraWorldLocation + Forward * A + Upward * B
	return CameraWorldLocation + CameraForwardVector * CameraInterpDistance + CameraUpwardVector;
}

void AParagonCharacter::GetPickupItem(AItemBase* Item)
{
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		//Inventory is not full
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
			Weapon->UpdateItemProperties();
		}
		else //Inventory is full, swapping picked up item with equipped weapon
		{
			SwapWeapon(Weapon);
		}
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}

	Item->PlayEquipSound();
}

FInterpLocation AParagonCharacter::GetInterpLocation(int32 Index)
{
	if (Index >= InterpLocations.Num())
		return FInterpLocation();

	return InterpLocations[Index];
}
