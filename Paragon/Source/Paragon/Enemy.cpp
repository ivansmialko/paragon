// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"

// Sets default values
AEnemy::AEnemy() :
	CurrentHealth(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(4.f),
	bIsCanHitReact(true),
	HitReactDelayMin(0.5f),
	HitReactDelayMax(3.f),
	HitNumberDestroyTime(1.5f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarDisappearTimer);
	GetWorldTimerManager().SetTimer(HealthBarDisappearTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	HideHealthBar();
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate /*= 1.0f*/)
{
	if (!bIsCanHitReact)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	AnimInstance->Montage_Play(HitMontage, PlayRate);
	AnimInstance->Montage_JumpToSection(Section, HitMontage);

	bIsCanHitReact = false;
	const float HitReactDelay{ FMath::FRandRange(HitReactDelayMin, HitReactDelayMax) };

	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactDelay);
}

void AEnemy::ResetHitReactTimer()
{
	bIsCanHitReact = true;
}

void AEnemy::StoreHitNumberWidget(UUserWidget* HitNumberWidget, FVector WidgetLocation)
{
	HitNumberWidgets.Add(HitNumberWidget, WidgetLocation);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumberWidget);
	GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumberWidgets.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if ((CurrentHealth - DamageAmount) < 0.0f)
	{
		CurrentHealth = 0.f;
		Die();
	}
	else
	{
		CurrentHealth -= DamageAmount;
	}

	return DamageAmount;
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	if (!ImpactSound)
		return;

	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());

	if (!ImpactParticles)
		return;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);

	ShowHealthBar();
	PlayHitMontage(FName("HitReact_Front"));
}

