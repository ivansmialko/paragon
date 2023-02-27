// Fill out your copyright notice in the Description page of Project Settings.


#include "ParagonCharacter.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"


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

	//Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);	//Attach camera to the end of boom
	FollowCamera->bUsePawnControlRotation = false; //Camera does not rotate relative to arm
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
}

