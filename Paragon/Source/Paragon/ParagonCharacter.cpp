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

#include "DrawDebugHelpers.h"
#include "ItemBase.h"
#include "Weapon.h"



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
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	bIsAiming(false),
	//Combat camera settings
	CameraDefaultFOV(0.f), //Setting this in begin play
	CameraZoomFOV(35.f),
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
	CameraInterpElevation(65.f)
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

	//UGameplayStatics::GetPlayerController(GetWorld(), 0)->bShowMouseCursor = true;
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
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("barrel_socket");
	if (!BarrelSocket)
		return;

	//Get location of barrel, where the bullet is flying out
	const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

	if (!MuzzleFlash)
		return;

	//Calculate point of impact of the bullet
	FVector ImpactPoint;
	bool bResult = GetBeamEndLocation(SocketTransform.GetLocation(), ImpactPoint);
	if (!bResult)
		return;

	if (!FireSound)
		return;

	//Play fire sound
	UGameplayStatics::PlaySound2D(GetWorld(), FireSound);

	//Spawn muzzle flash particles
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);

	if (!ImpactParticles)
		return;

	//Spawn impact particles
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, ImpactPoint);

	if (!BeamParticles)
		return;
		
	//Spawn bullet trace particles
	UParticleSystemComponent* BulletBeamParticles = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
	if (!BulletBeamParticles)
		return;

	BulletBeamParticles->SetVectorParameter(FName("Target"), ImpactPoint);

	//Play player character animation of shooting
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !FireRecoilMontage)
		return;

	AnimInstance->Montage_Play(FireRecoilMontage);
	AnimInstance->Montage_JumpToSection(FName("WeaponFire"));

	if (!FeedbackFire)
		return;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
		return;

	PlayerController->ClientPlayForceFeedback(FeedbackFire, false, FName(TEXT("FeedbackFire")));

	//Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();
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
	bIsAiming = true;
}

void AParagonCharacter::SetAimingButtonReleased()
{
	bIsAiming = false;
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

	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, FString::Printf(TEXT("Current crosshair spread: %f"), CrosshairSpreadMultiplier));
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
	StartFireTimer();
}

void AParagonCharacter::FireButtonReleased()
{
	bIsFireButtonPressed = false;
}

void AParagonCharacter::StartFireTimer()
{
	if (!bIsShouldFireAtThisFrame)
		return;

	FireWeapon();
	bIsShouldFireAtThisFrame = false;

	//Timer to wait for a new bullet to fire
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AParagonCharacter::AutoFireReset, AutomaticFireRate);
}

void AParagonCharacter::AutoFireReset()
{
	bIsShouldFireAtThisFrame = true;

	if (!bIsFireButtonPressed)
		return;

	//If fire button still pressed - fire another bullet
	StartFireTimer();
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
		}

		return;
	}

	FHitResult ItemTraceResult;
	FVector HitLocation;
	if (TraceUnderCrosshairs(ItemTraceResult, HitLocation))
	{
		TraceHitItem = Cast<AItemBase>(ItemTraceResult.Actor);
		if (TraceHitItem && TraceHitItem->GetPickupWidget())
		{
			//Shot Item's pick up widget
			TraceHitItem->GetPickupWidget()->SetVisibility(true);
		}
		
		//We hit an AItemBase last frame
		if (TraceHitLastFrame && (TraceHitLastFrame != TraceHitItem))
		{
			//We are hitting a different AItemBase this frame from last frame
			TraceHitLastFrame->GetPickupWidget()->SetVisibility(false);
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

void AParagonCharacter::EquipWeapon(class AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip)
		return;

	//Get the hand socket
	const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("right_hand_socket"));
	if (!HandSocket)
		return;

	//Attach the weapon to the hand socket
	HandSocket->AttachActor(WeaponToEquip, GetMesh());

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

	EquippedWeapon = nullptr;
}

void AParagonCharacter::SelectButtonPressed()
{
	if (!TraceHitItem)
		return;

	auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
	if (!TraceHitItem)
		return;

	TraceHitItem->StartItemFlying(this);
}

void AParagonCharacter::SelectButtonReleased()
{

}

void AParagonCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitLastFrame = nullptr;
}

// Called every frame
void AParagonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ZoomCameraInterp(DeltaTime);
	UpdateLookRates();
	UpdateCrosshairSpread(DeltaTime);
	TraceForItems();
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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AParagonCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AParagonCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AParagonCharacter::SetAimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AParagonCharacter::SetAimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AParagonCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AParagonCharacter::SelectButtonReleased);

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
		SwapWeapon(Weapon);
	}
}

