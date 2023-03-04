// Fill out your copyright notice in the Description page of Project Settings.


#include "ParagonCharacter.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AParagonCharacter::AParagonCharacter():
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; //The camera foollows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

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

void AParagonCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("barrel_socket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		//Get current size of the viewport
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		//Get screen-space location of crosshair
		FVector2D CrosshairLocation(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f - 50.f);

		//Get world position and direction 
		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;
		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation,
			CrosshairWorldPosition, CrosshairWorldDirection);

		if (bScreenToWorld) //Was deprojection successfull?
		{
			FHitResult ScreenTraceHit;
			const FVector LineStart{ CrosshairWorldPosition };
			const FVector LineEnd{ CrosshairWorldPosition + CrosshairWorldDirection * 5'000.f };

			//Set beam end point to linetrace end point
			FVector BeamEndPoint{ LineEnd };

			//Trace outwards from crosshairs world location
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, LineStart, LineEnd, ECollisionChannel::ECC_Visibility);
			if (ScreenTraceHit.bBlockingHit) //was there a trace hit?
			{
				BeamEndPoint = ScreenTraceHit.Location;
			}

			//Perform a second trace, this time from the gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart{ SocketTransform.GetLocation() };
			const FVector WeaponTraceEnd{ BeamEndPoint };
			
			//Object between barrel and beam endpoint
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
			if (WeaponTraceHit.bBlockingHit)
			{
				BeamEndPoint = WeaponTraceHit.Location;
			}

			//Spawn impact particles after updating beam endpoint
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndPoint);
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
				}
			}
		}
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireRecoilMontage)
	{
		AnimInstance->Montage_Play(FireRecoilMontage);
		AnimInstance->Montage_JumpToSection(FName("WeaponFire"));
	}


}

// Called every frame
void AParagonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AParagonCharacter::FireWeapon);

}

